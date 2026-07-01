#include "lz_engine.h"
#include <stdlib.h>
#include <string.h>

/* ======================================================================
 * Primary Hash Table — 131K buckets, chained, 4-byte prefix
 * ====================================================================== */

#define HT_BUCKETS  131072
#define HT2_BUCKETS 65536    /* secondary: direct-mapped, 8-byte prefix */

typedef struct HTNode {
    int32_t        pos;
    struct HTNode *next;
} HTNode;

typedef struct {
    HTNode  **buckets;
    HTNode  *pool;
    int32_t  pool_cap;
    int32_t  pool_used;
} HashTable;

/* Secondary hash table — direct-mapped, for long matches */
typedef struct {
    int32_t positions[HT2_BUCKETS];
} HashTable2;

static void ht_init(HashTable *ht, int32_t cap) {
    ht->buckets = (HTNode **)calloc(HT_BUCKETS, sizeof(HTNode *));
    ht->pool = (HTNode *)malloc((size_t)cap * sizeof(HTNode));
    ht->pool_cap = cap;
    ht->pool_used = 0;
}

static void ht_free(HashTable *ht) { free(ht->pool); free(ht->buckets); }

static inline void ht_insert(HashTable *ht, uint32_t hash, int32_t pos) {
    if (ht->pool_used >= ht->pool_cap) {
        int32_t nc = ht->pool_cap + (ht->pool_cap >> 1);
        ht->pool = (HTNode *)realloc(ht->pool, (size_t)nc * sizeof(HTNode));
        ht->pool_cap = nc;
    }
    uint32_t idx = hash & (HT_BUCKETS - 1);
    HTNode *n = &ht->pool[ht->pool_used++];
    n->pos  = pos;
    n->next = ht->buckets[idx];
    ht->buckets[idx] = n;
}

static void ht2_init(HashTable2 *ht) {
    memset(ht->positions, -1, sizeof(ht->positions));
}

static inline void ht2_insert(HashTable2 *ht, uint32_t hash, int32_t pos) {
    ht->positions[hash & (HT2_BUCKETS - 1)] = pos;
}

static inline int32_t ht2_lookup(const HashTable2 *ht, uint32_t hash) {
    return ht->positions[hash & (HT2_BUCKETS - 1)];
}

/* ======================================================================
 * Hash functions — 4-byte primary + 8-byte secondary
 * ====================================================================== */

static inline uint32_t compute_hash4(const uint8_t *d, int32_t s) {
    return (uint32_t)d[s] * 16777619u ^ (uint32_t)d[s+1] * 65599u ^
           (uint32_t)d[s+2] * 257u ^ (uint32_t)d[s+3];
}

static inline uint32_t compute_hash8(const uint8_t *d, int32_t s) {
    uint32_t h = 2166136261u;
    h ^= d[s];   h *= 16777619u;
    h ^= d[s+1]; h *= 16777619u;
    h ^= d[s+2]; h *= 16777619u;
    h ^= d[s+3]; h *= 16777619u;
    h ^= d[s+4]; h *= 16777619u;
    h ^= d[s+5]; h *= 16777619u;
    h ^= d[s+6]; h *= 16777619u;
    h ^= d[s+7]; h *= 16777619u;
    return h;
}

/* ======================================================================
 * Match scoring — entropy-aligned (log2-based costs)
 * ====================================================================== */

static inline int32_t log2_fast(int32_t v) {
    int32_t r = 0;
    while (v > 1) { r++; v >>= 1; }
    return r;
}

/* Slot cost estimate for offset encoding (bits) */
static inline int32_t slot_cost_bits(int32_t off) {
    int32_t slot = log2_fast(off > 0 ? off : 1);
    return 5 + slot;  /* ~5 bits for slot selection + slot extra bits */
}

/* REP0: near-free (1.5 bits effective) */
#define SCORE_REP0(len) ((len) * 256 - 36)
/* REP1: very cheap (2.5 bits) */
#define SCORE_REP1(len) ((len) * 256 - 64)
/* REP2: cheap (3 bits) */
#define SCORE_REP2(len) ((len) * 256 - 80)
/* New offset: slot_cost * 24 (each offset bit costs ~24 score units) */
#define SCORE_NEW(len, off) ((len) * 256 - slot_cost_bits(off) * 24)
/* Lazy threshold */
#define LAZY_THRESHOLD 384

/* ======================================================================
 * Find best match — primary hash table (4-byte prefix, chained)
 * ====================================================================== */

typedef struct { int32_t length; int32_t offset; } Match;

static Match find_hash_match(const uint8_t *window, int32_t pos, int32_t n,
                             HashTable *ht) {
    Match best = {0, 0};
    if (pos + LZ_MIN_MATCH > n) return best;

    uint32_t h = compute_hash4(window, pos);
    uint32_t bidx = h & (HT_BUCKETS - 1);
    HTNode *node = ht->buckets[bidx];
    int depth = 0;

    while (node && depth < LZ_MAX_SEARCH_DEPTH) {
        int32_t p = node->pos;
        int32_t dist = pos - p;
        if (dist > LZ_WINDOW_SIZE || dist <= 0) {
            if (dist > LZ_WINDOW_SIZE) break;
            node = node->next;
            continue;
        }

        if (window[p] == window[pos] &&
            window[p+1] == window[pos+1] &&
            window[p+2] == window[pos+2] &&
            window[p+3] == window[pos+3]) {
            int32_t l = LZ_MIN_MATCH;
            int32_t max_l = n - pos;
            if (max_l > LZ_MAX_MATCH) max_l = LZ_MAX_MATCH;
            while (l < max_l && window[p + l] == window[pos + l]) l++;

            if (l > best.length) {
                best.length = l;
                best.offset = dist;
                if (l >= 64) break; /* Great match — stop */
            }
        }

        node = node->next;
        depth++;
    }
    return best;
}

/* ======================================================================
 * Find long match — secondary hash table (8-byte prefix, direct-mapped)
 * ====================================================================== */

static Match find_long_match(const uint8_t *window, int32_t pos, int32_t n,
                             HashTable2 *ht2) {
    Match best = {0, 0};
    if (pos + 8 > n) return best;

    uint32_t h = compute_hash8(window, pos);
    int32_t p = ht2_lookup(ht2, h);
    if (p < 0) return best;

    int32_t dist = pos - p;
    if (dist <= 0 || dist > LZ_WINDOW_SIZE) return best;

    /* Verify 8-byte prefix */
    for (int i = 0; i < 8; i++)
        if (window[p+i] != window[pos+i]) return best;

    int32_t l = 8;
    int32_t max_l = n - pos;
    if (max_l > LZ_MAX_MATCH) max_l = LZ_MAX_MATCH;
    while (l < max_l && window[p + l] == window[pos + l]) l++;

    best.length = l;
    best.offset = dist;
    return best;
}

/* ======================================================================
 * Try REP match at given offset
 * ====================================================================== */

static int32_t try_rep_match(const uint8_t *window, int32_t pos, int32_t n,
                             int32_t rep_off) {
    if (rep_off <= 0 || pos < rep_off) return 0;
    int32_t ref = pos - rep_off;
    int32_t l = 0;
    int32_t max_l = n - pos;
    if (max_l > LZ_MAX_MATCH) max_l = LZ_MAX_MATCH;
    while (l < max_l && window[ref + l] == window[pos + l]) l++;
    return l;
}

/* ======================================================================
 * LZ Compress — Lazy Scoring with REP0/1/2 + Dual Hash
 * ====================================================================== */

void lz_compress(const uint8_t *data, size_t data_len,
                 const uint8_t *prev_tail, size_t prev_tail_len,
                 LZResult *result)
{
    size_t total_len = prev_tail_len + data_len;
    uint8_t *window = (uint8_t *)malloc(total_len);
    if (prev_tail_len > 0) memcpy(window, prev_tail, prev_tail_len);
    memcpy(window + prev_tail_len, data, data_len);

    int32_t offset_base = (int32_t)prev_tail_len;
    int32_t n = (int32_t)total_len;
    int32_t data_n = (int32_t)data_len;

    IntArray_init(&result->instructions);
    IntArray_init(&result->literals);
    IntArray_init(&result->rep_types);
    IntArray_init(&result->new_offsets);
    IntArray_init(&result->lengths);

    if (data_len == 0) { free(window); return; }

    /* Build hash tables */
    HashTable ht;
    ht_init(&ht, (int32_t)(total_len + 4096));
    HashTable2 *ht2 = (HashTable2 *)malloc(sizeof(HashTable2));
    ht2_init(ht2);

    /* Insert prev_tail positions */
    if ((int32_t)prev_tail_len >= LZ_MIN_MATCH) {
        for (int32_t i = 0; i <= (int32_t)prev_tail_len - LZ_MIN_MATCH; i++) {
            ht_insert(&ht, compute_hash4(window, i), i);
            if (i + 8 <= (int32_t)prev_tail_len)
                ht2_insert(ht2, compute_hash8(window, i), i);
        }
    }

    int32_t rep0 = 0, rep1 = 0, rep2 = 0;
    int32_t pos = 0;

    while (pos < data_n) {
        int32_t wpos = offset_base + pos;

        /* Step 1: Try REP matches (free — no hash lookup) */
        int32_t rep0_len = try_rep_match(window, wpos, n, rep0);
        int32_t rep1_len = try_rep_match(window, wpos, n, rep1);
        int32_t rep2_len = try_rep_match(window, wpos, n, rep2);

        /* Step 2: Try hash matches (primary + secondary) */
        Match hm = find_hash_match(window, wpos, n, &ht);
        Match lm = find_long_match(window, wpos, n, ht2);

        /* Step 3: Pick best option by score */
        int32_t best_len = 0, best_off = 0, best_rep = 3;
        int32_t best_score = -1;

        if (rep0_len >= LZ_MIN_MATCH) {
            int32_t s = SCORE_REP0(rep0_len);
            if (s > best_score) {
                best_score = s; best_len = rep0_len;
                best_off = rep0; best_rep = 0;
            }
        }
        if (rep1_len >= LZ_MIN_MATCH) {
            int32_t s = SCORE_REP1(rep1_len);
            if (s > best_score) {
                best_score = s; best_len = rep1_len;
                best_off = rep1; best_rep = 1;
            }
        }
        if (rep2_len >= LZ_MIN_MATCH) {
            int32_t s = SCORE_REP2(rep2_len);
            if (s > best_score) {
                best_score = s; best_len = rep2_len;
                best_off = rep2; best_rep = 2;
            }
        }
        if (hm.length >= LZ_MIN_MATCH) {
            int32_t s = SCORE_NEW(hm.length, hm.offset);
            if (s > best_score) {
                best_score = s; best_len = hm.length;
                best_off = hm.offset; best_rep = 3;
            }
        }
        if (lm.length >= LZ_MIN_MATCH) {
            int32_t s = SCORE_NEW(lm.length, lm.offset);
            if (s > best_score) {
                best_score = s; best_len = lm.length;
                best_off = lm.offset; best_rep = 3;
            }
        }

        if (best_len < LZ_MIN_MATCH) {
            /* No match — emit literal */
            IntArray_push(&result->instructions, 0);
            IntArray_push(&result->literals, (int32_t)window[wpos]);
            if (wpos + LZ_MIN_MATCH <= n)
                ht_insert(&ht, compute_hash4(window, wpos), wpos);
            if (wpos + 8 <= n)
                ht2_insert(ht2, compute_hash8(window, wpos), wpos);
            pos++;
            continue;
        }

        /* Step 4: Lazy evaluation — check pos+1 (skip for long matches) */
        if (best_len < 64 && pos + 1 < data_n) {
            int32_t wpos2 = wpos + 1;
            Match hm2 = find_hash_match(window, wpos2, n, &ht);
            Match lm2 = find_long_match(window, wpos2, n, ht2);
            int32_t rep0_len2 = try_rep_match(window, wpos2, n, rep0);

            int32_t lazy_score = -1;
            if (hm2.length >= LZ_MIN_MATCH) {
                int32_t s = SCORE_NEW(hm2.length, hm2.offset);
                if (s > lazy_score) lazy_score = s;
            }
            if (lm2.length >= LZ_MIN_MATCH) {
                int32_t s = SCORE_NEW(lm2.length, lm2.offset);
                if (s > lazy_score) lazy_score = s;
            }
            if (rep0_len2 >= LZ_MIN_MATCH) {
                int32_t s = SCORE_REP0(rep0_len2);
                if (s > lazy_score) lazy_score = s;
            }

            if (lazy_score > best_score + LAZY_THRESHOLD) {
                IntArray_push(&result->instructions, 0);
                IntArray_push(&result->literals, (int32_t)window[wpos]);
                ht_insert(&ht, compute_hash4(window, wpos), wpos);
                if (wpos + 8 <= n)
                    ht2_insert(ht2, compute_hash8(window, wpos), wpos);
                pos++;
                continue;
            }
        }

        /* Step 5: Emit match */
        IntArray_push(&result->instructions, 1);
        IntArray_push(&result->lengths, best_len);

        if (best_rep == 0) {
            IntArray_push(&result->rep_types, 0);
            /* rep0 stays — no state change */
        } else if (best_rep == 1) {
            IntArray_push(&result->rep_types, 1);
            /* promote rep1 → rep0 */
            int32_t tmp = rep1;
            rep1 = rep0; rep0 = tmp;
        } else if (best_rep == 2) {
            IntArray_push(&result->rep_types, 2);
            /* promote rep2 → rep0 */
            int32_t tmp = rep2;
            rep2 = rep1; rep1 = rep0; rep0 = tmp;
        } else {
            IntArray_push(&result->rep_types, 3);
            IntArray_push(&result->new_offsets, best_off);
            rep2 = rep1; rep1 = rep0; rep0 = best_off;
        }

        /* Step 6: Insert intermediate positions into hash tables */
        int32_t insert_limit = best_len < 1024 ? best_len : 1024;
        for (int32_t k = 0; k < insert_limit && wpos + k + LZ_MIN_MATCH <= n; k++) {
            ht_insert(&ht, compute_hash4(window, wpos + k), wpos + k);
            if (wpos + k + 8 <= n)
                ht2_insert(ht2, compute_hash8(window, wpos + k), wpos + k);
        }

        pos += best_len;
    }

    ht_free(&ht);
    free(ht2);
    free(window);
}

/* ======================================================================
 * LZ Decompress — with REP0/1/2 matches
 * ====================================================================== */

void lz_decompress(const int32_t *instructions, size_t num_instr,
                   const int32_t *literals,
                   const int32_t *rep_types,
                   const int32_t *new_offsets,
                   const int32_t *lengths,
                   const uint8_t *prev_tail, size_t prev_tail_len,
                   uint8_t **out, size_t *out_len)
{
    size_t final_len = 0;
    size_t match_count = 0;
    for (size_t j = 0; j < num_instr; j++) {
        if (instructions[j] == 0) {
            final_len++;
        } else {
            final_len += (size_t)lengths[match_count++];
        }
    }

    size_t buf_len = prev_tail_len + final_len;
    uint8_t *buf = (uint8_t *)malloc(buf_len ? buf_len : 1);
    if (prev_tail_len > 0) memcpy(buf, prev_tail, prev_tail_len);
    size_t out_pos = prev_tail_len;

    size_t lit_idx = 0, match_idx = 0, new_off_idx = 0;
    int32_t rep0 = 0, rep1 = 0, rep2 = 0;

    for (size_t j = 0; j < num_instr; j++) {
        if (instructions[j] == 0) {
            buf[out_pos++] = (uint8_t)literals[lit_idx++];
        } else {
            int32_t rt = rep_types[match_idx];
            int32_t length = lengths[match_idx];
            match_idx++;

            int32_t offset;
            if (rt == 0) {
                offset = rep0;
                /* rep0 stays */
            } else if (rt == 1) {
                offset = rep1;
                int32_t tmp = rep1;
                rep1 = rep0; rep0 = tmp;
            } else if (rt == 2) {
                offset = rep2;
                int32_t tmp = rep2;
                rep2 = rep1; rep1 = rep0; rep0 = tmp;
            } else {
                offset = new_offsets[new_off_idx++];
                rep2 = rep1; rep1 = rep0; rep0 = offset;
            }

            size_t start = out_pos - (size_t)offset;
            if ((size_t)offset >= (size_t)length) {
                memcpy(buf + out_pos, buf + start, (size_t)length);
            } else {
                for (int32_t k = 0; k < length; k++) {
                    buf[out_pos + (size_t)k] = buf[start + (size_t)k];
                }
            }
            out_pos += (size_t)length;
        }
    }

    *out_len = final_len;
    *out = (uint8_t *)malloc(*out_len);
    memcpy(*out, buf + prev_tail_len, *out_len);
    free(buf);
}

void lz_result_free(LZResult *r) {
    IntArray_free(&r->instructions);
    IntArray_free(&r->literals);
    IntArray_free(&r->rep_types);
    IntArray_free(&r->new_offsets);
    IntArray_free(&r->lengths);
}
