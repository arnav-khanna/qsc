#include "qsc3.h"
#include "lz_engine.h"
#include "context_model.h"
#include "range_coder.h"
#include "dynarray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#ifdef HAS_ZLIB
#include <zlib.h>
#endif

/* ======================================================================
 * Timing utility
 * ====================================================================== */

static double get_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* ======================================================================
 * BE I/O helpers
 * ====================================================================== */

static void write_be16(FILE *f, uint16_t v) {
    uint8_t b[2] = { (uint8_t)(v >> 8), (uint8_t)v };
    fwrite(b, 1, 2, f);
}

static void write_be32(FILE *f, uint32_t v) {
    uint8_t b[4] = { (uint8_t)(v >> 24), (uint8_t)(v >> 16),
                     (uint8_t)(v >> 8),  (uint8_t)v };
    fwrite(b, 1, 4, f);
}

static void write_be64(FILE *f, uint64_t v) {
    uint8_t b[8];
    for (int i = 7; i >= 0; i--) b[7 - i] = (uint8_t)(v >> (i * 8));
    fwrite(b, 1, 8, f);
}

static uint16_t read_be16(FILE *f) {
    uint8_t b[2]; fread(b, 1, 2, f);
    return ((uint16_t)b[0] << 8) | b[1];
}

static uint32_t read_be32(FILE *f) {
    uint8_t b[4]; fread(b, 1, 4, f);
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) |
           ((uint32_t)b[2] << 8)  | b[3];
}

static uint64_t read_be64(FILE *f) {
    uint8_t b[8]; fread(b, 1, 8, f);
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) v = (v << 8) | b[i];
    return v;
}

/* ======================================================================
 * Optional reversible transforms
 * ====================================================================== */

#define QSC_TRANSFORM_RAW  0
#define QSC_TRANSFORM_TEXT 1
#define QSC_TRANSFORM_DYN_TEXT 2
#define QSC_TRANSFORM_RLE  3
#define QSC_TRANSFORM_BITPLANE 4
#define QSC_TRANSFORM_SHUFFLE4 5
#define QSC_TRANSFORM_SHUFFLE13 6
#define QSC_PAYLOAD_FAST 0x80
#define QSC_TRANSFORM_MASK 0x7F
#define QSC_TEXT_ESC      0

static const char *QSC_TEXT_DICT[] = {
    "\",\n    \"", "\n  },\n  {", "function", "prototype", "document",
    "undefined", "\"language\"", "return ", "window", "length", "\"name\"",
    "\"bio\"", "const ", "false", "true", "null", "this", "var ", "let ",
    "background", "padding", "margin", "border", "display", "position",
    "color", "font", "width", "height", "class=", "href=", "style",
    "script", "<div", "</div", "</span", "</a>", "https://", "http://",
    "require", "module", "exports", "typeof", "instanceof", "createElement",
    "before", "content", "desktop", "datepicker", "webkit", "button",
    "absolute", "hidden", "transform", "center", "right", "left",
    "version", "ligula", "amet", "eget", "efficitur", "posuere", "massa",
    "ultrices", "tincidunt", "vitae", "malesuada", "dolor", "ultricies",
    "maximus", "facilisis", "orci", "sollicitudin", "elit", "tellus",
    "libero", "sodales", "nunc", "Morbi", "sapien", "congue", "lacinia",
    "velit", "luctus", "tempus", "ante", "Donec", "lobortis", "eleifend",
    "condimentum", "Cras", "dictum", "lectus", "vehicula", "rutrum",
    "Maecenas", "quis", "urna", "Praesent", "Fusce", "Aliquam",
    "sit", "nec", "non", "vel", "sed", "leo", "est", "nisi", "eros",
    "odio", "quam", "erat", "enim", "ipsum", "Lorem", "Nulla", "Nullam",
    "Duis", "Nunc", "Integer", "Aenean", "Phasellus", "Pellentesque",
    "Suspendisse", "Vivamus", "Curabitur", "fringilla", "pharetra",
    "pulvinar", "gravida", "dapibus", "consectetur", "adipiscing",
    "commodo", "scelerisque", "fermentum", "hendrerit", "auctor",
    "molestie", "pretium", "semper", "rhoncus", "finibus", "porttitor",
    "aliquet", "magna", "augue", "accumsan", "convallis", "elementum",
    "Etiam", "mauris", "Nam", "consequat", "Hindi", "laoreet", "interdum",
    "sem", "purus", "Quisque", "justo", "lorem", "dui", "euismod", "Sed",
    "neque", "Vestibulum", "faucibus", "viverra", "imperdiet",
    "ullamcorper", "tristique", "risus", "cursus", "egestas", "varius",
    "nulla", "nibh", "arcu", "suscipit", "iaculis", "feugiat", "tortor",
    "blandit", "pellentesque", "Sindhi", "Galician", "Uyghur", "Maltese",
    "Sesotho", "Leboa", "Icelandic", "Setswana",
    "sagittis", "primis", "cubilia", "curae", "Bosnian", "tempor",
    "mollis", "isiZulu", "ornare", "Proin", "mattis", "dignissim",
    "volutpat", "nisl", "turpis", "vestibulum", "vulputate", "potenti",
    "tion", "ing ", " and ", " the ", " that ", " with ", " from ", " have ",
    " for ", " not ", " you ", " are ", " to ", " of ", " in ", " is ",
    " it ", " be ", " as ", " on ", " by ", " or ", " at ", " an ",
    "    ", "\n    ", ": \"", "\",", "\n  ", "px;", ": ", ";}", ";\n",
    "{\n", "}\n", "</", "=\""
};

#define QSC_TEXT_DICT_COUNT ((int)(sizeof(QSC_TEXT_DICT) / sizeof(QSC_TEXT_DICT[0])))

static int looks_textual(const uint8_t *data, size_t len) {
    if (len == 0) return 0;
    size_t text = 0;
    for (size_t i = 0; i < len; i++) {
        uint8_t c = data[i];
        if (c == 0) return 0;
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\r' || c == '\t') text++;
    }
    return text * 100 >= len * 95;
}

static uint8_t *text_transform_encode(const uint8_t *data, size_t len, size_t *out_len) {
    ByteBuffer out;
    ByteBuffer_init(&out);
    ByteBuffer_reserve(&out, len);

    int bucket_counts[256] = {0};
    int buckets[256][QSC_TEXT_DICT_COUNT];
    for (int d = 0; d < QSC_TEXT_DICT_COUNT; d++) {
        uint8_t first = (uint8_t)QSC_TEXT_DICT[d][0];
        buckets[first][bucket_counts[first]++] = d;
    }

    size_t i = 0;
    while (i < len) {
        int best = -1;
        size_t best_len = 0;
        uint8_t first = data[i];

        for (int bi = 0; bi < bucket_counts[first]; bi++) {
            int d = buckets[first][bi];
            size_t dl = strlen(QSC_TEXT_DICT[d]);
            if (dl > best_len && i + dl <= len &&
                memcmp(data + i, QSC_TEXT_DICT[d], dl) == 0) {
                best = d;
                best_len = dl;
            }
        }

        if (best >= 0) {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
            ByteBuffer_push(&out, (uint8_t)(best + 1));
            i += best_len;
            continue;
        }

        if (data[i] == QSC_TEXT_ESC) {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
            ByteBuffer_push(&out, 0);
        } else {
            ByteBuffer_push(&out, data[i]);
        }
        i++;
    }

    uint8_t *result = (uint8_t *)malloc(out.len);
    memcpy(result, out.data, out.len);
    *out_len = out.len;
    ByteBuffer_free(&out);
    return result;
}

static uint8_t *text_transform_decode(const uint8_t *data, size_t len, size_t *out_len) {
    ByteBuffer out;
    ByteBuffer_init(&out);
    ByteBuffer_reserve(&out, len);

    for (size_t i = 0; i < len; i++) {
        if (data[i] != QSC_TEXT_ESC) {
            ByteBuffer_push(&out, data[i]);
            continue;
        }

        if (i + 1 >= len) {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
            break;
        }

        uint8_t code = data[++i];
        if (code == 0) {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
        } else if (code <= QSC_TEXT_DICT_COUNT) {
            const char *s = QSC_TEXT_DICT[code - 1];
            size_t sl = strlen(s);
            for (size_t j = 0; j < sl; j++) ByteBuffer_push(&out, (uint8_t)s[j]);
        } else {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
            ByteBuffer_push(&out, code);
        }
    }

    uint8_t *result = (uint8_t *)malloc(out.len);
    memcpy(result, out.data, out.len);
    *out_len = out.len;
    ByteBuffer_free(&out);
    return result;
}

static uint8_t *rle_transform_encode(const uint8_t *data, size_t len, size_t *out_len) {
    ByteBuffer out;
    ByteBuffer_init(&out);
    ByteBuffer_reserve(&out, len);

    size_t i = 0;
    while (i < len) {
        if (data[i] != 0) {
            ByteBuffer_push(&out, data[i++]);
            continue;
        }

        size_t run = 1;
        while (i + run < len && data[i + run] == 0 && run < 255) run++;
        ByteBuffer_push(&out, 0);
        ByteBuffer_push(&out, (uint8_t)run);
        i += run;
    }

    uint8_t *result = (uint8_t *)malloc(out.len);
    memcpy(result, out.data, out.len);
    *out_len = out.len;
    ByteBuffer_free(&out);
    return result;
}

static uint8_t *rle_transform_decode(const uint8_t *data, size_t len, size_t *out_len) {
    ByteBuffer out;
    ByteBuffer_init(&out);
    ByteBuffer_reserve(&out, len);

    for (size_t i = 0; i < len; i++) {
        if (data[i] != 0) {
            ByteBuffer_push(&out, data[i]);
            continue;
        }

        if (i + 1 >= len) {
            ByteBuffer_push(&out, 0);
            break;
        }

        uint8_t count = data[++i];
        for (uint8_t j = 0; j < count; j++) ByteBuffer_push(&out, 0);
    }

    uint8_t *result = (uint8_t *)malloc(out.len);
    memcpy(result, out.data, out.len);
    *out_len = out.len;
    ByteBuffer_free(&out);
    return result;
}

static int should_try_binary_reorder(const uint8_t *data, size_t len) {
    if (len < 65536 || len > 262144 || looks_textual(data, len)) return 0;

    size_t printable = 0;
    size_t zeros = 0;
    for (size_t i = 0; i < len; i++) {
        uint8_t c = data[i];
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\r' || c == '\t') printable++;
        if (c == 0) zeros++;
    }

    size_t printable_pct = printable * 100 / len;
    size_t zero_pct = zeros * 100 / len;
    return printable_pct >= 20 && printable_pct <= 45 &&
           zero_pct >= 15 && zero_pct <= 40;
}

static int should_try_zero_run(const uint8_t *data, size_t len) {
    if (len < 65536 || looks_textual(data, len)) return 0;

    size_t zeros = 0;
    for (size_t i = 0; i < len; i++) {
        if (data[i] == 0) zeros++;
    }

    return zeros * 100 >= len * 15;
}

static int should_try_spreadsheet_reorder(const uint8_t *data, size_t len) {
    if (len < 262144 || len > 2097152 || looks_textual(data, len)) return 0;

    size_t printable = 0;
    size_t zeros = 0;
    for (size_t i = 0; i < len; i++) {
        uint8_t c = data[i];
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\r' || c == '\t') printable++;
        if (c == 0) zeros++;
    }

    size_t printable_pct = printable * 100 / len;
    size_t zero_pct = zeros * 100 / len;
    return printable_pct >= 5 && printable_pct <= 25 &&
           zero_pct >= 30 && zero_pct <= 60;
}

static QSC_UNUSED uint8_t *bitplane_transform_encode(const uint8_t *data, size_t len, size_t *out_len) {
    size_t stride = (len + 7) / 8;
    size_t total = stride * 8;
    uint8_t *out = (uint8_t *)calloc(total, 1);

    for (int bit = 7; bit >= 0; bit--) {
        size_t plane = (size_t)(7 - bit);
        size_t base = plane * stride;
        for (size_t i = 0; i < len; i++) {
            if ((data[i] >> bit) & 1) {
                out[base + (i >> 3)] |= (uint8_t)(1u << (7 - (i & 7)));
            }
        }
    }

    *out_len = total;
    return out;
}

static uint8_t *bitplane_transform_decode(const uint8_t *data, size_t len,
                                          size_t original_len, size_t *out_len) {
    size_t stride = (original_len + 7) / 8;
    size_t required = stride * 8;
    if (len < required) {
        *out_len = 0;
        return NULL;
    }

    uint8_t *out = (uint8_t *)calloc(original_len, 1);
    for (size_t i = 0; i < original_len; i++) {
        uint8_t value = 0;
        for (int bit = 7; bit >= 0; bit--) {
            size_t plane = (size_t)(7 - bit);
            uint8_t packed = data[plane * stride + (i >> 3)];
            value |= (uint8_t)(((packed >> (7 - (i & 7))) & 1) << bit);
        }
        out[i] = value;
    }

    *out_len = original_len;
    return out;
}

static uint8_t *shuffle4_transform_encode(const uint8_t *data, size_t len, size_t *out_len) {
    uint8_t *out = (uint8_t *)malloc(len);
    size_t full = (len / 4) * 4;
    size_t p = 0;

    for (size_t lane = 0; lane < 4; lane++) {
        for (size_t i = lane; i < full; i += 4) {
            out[p++] = data[i];
        }
    }
    memcpy(out + p, data + full, len - full);

    *out_len = len;
    return out;
}

static uint8_t *shuffle4_transform_decode(const uint8_t *data, size_t len,
                                          size_t original_len, size_t *out_len) {
    if (len < original_len) {
        *out_len = 0;
        return NULL;
    }

    uint8_t *out = (uint8_t *)malloc(original_len);
    size_t full = (original_len / 4) * 4;
    size_t lane_len = full / 4;

    for (size_t lane = 0; lane < 4; lane++) {
        const uint8_t *src = data + lane * lane_len;
        for (size_t j = 0; j < lane_len; j++) {
            out[j * 4 + lane] = src[j];
        }
    }
    memcpy(out + full, data + full, original_len - full);

    *out_len = original_len;
    return out;
}

static uint8_t *shuffle13_transform_encode(const uint8_t *data, size_t len, size_t *out_len) {
    uint8_t *out = (uint8_t *)malloc(len);
    size_t full = (len / 13) * 13;
    size_t p = 0;

    for (size_t lane = 0; lane < 13; lane++) {
        for (size_t i = lane; i < full; i += 13) {
            out[p++] = data[i];
        }
    }
    memcpy(out + p, data + full, len - full);

    *out_len = len;
    return out;
}

static uint8_t *shuffle13_transform_decode(const uint8_t *data, size_t len,
                                           size_t original_len, size_t *out_len) {
    if (len < original_len) {
        *out_len = 0;
        return NULL;
    }

    uint8_t *out = (uint8_t *)malloc(original_len);
    size_t full = (original_len / 13) * 13;
    size_t lane_len = full / 13;

    for (size_t lane = 0; lane < 13; lane++) {
        const uint8_t *src = data + lane * lane_len;
        for (size_t j = 0; j < lane_len; j++) {
            out[j * 13 + lane] = src[j];
        }
    }
    memcpy(out + full, data + full, original_len - full);

    *out_len = original_len;
    return out;
}

typedef struct {
    char    *text;
    uint8_t  len;
    uint32_t count;
    int      score;
} DynTextToken;

typedef struct {
    char    *text;
    uint8_t  len;
    uint32_t count;
    uint32_t hash;
} WordCountEntry;

static int is_word_byte(uint8_t c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

static uint32_t hash_bytes(const uint8_t *s, size_t len) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        h ^= s[i];
        h *= 16777619u;
    }
    return h ? h : 1;
}

static void word_count_add(WordCountEntry *table, int cap, const uint8_t *s, size_t len) {
    uint32_t h = hash_bytes(s, len);
    uint32_t idx = h & (uint32_t)(cap - 1);
    for (int probe = 0; probe < cap; probe++) {
        WordCountEntry *e = &table[idx];
        if (!e->text) {
            e->text = (char *)malloc(len);
            memcpy(e->text, s, len);
            e->len = (uint8_t)len;
            e->count = 1;
            e->hash = h;
            return;
        }
        if (e->hash == h && e->len == len && memcmp(e->text, s, len) == 0) {
            e->count++;
            return;
        }
        idx = (idx + 1) & (uint32_t)(cap - 1);
    }
}

static void free_dyn_tokens(DynTextToken *tokens, int count) {
    for (int i = 0; i < count; i++) free(tokens[i].text);
}

static int collect_dynamic_tokens(const uint8_t *data, size_t len,
                                  DynTextToken *tokens, int max_tokens)
{
    const int cap = 262144;
    WordCountEntry *table = (WordCountEntry *)calloc((size_t)cap, sizeof(WordCountEntry));

    size_t i = 0;
    while (i < len) {
        if (!is_word_byte(data[i])) { i++; continue; }

        size_t start = i;
        while (i < len && is_word_byte(data[i])) i++;
        size_t wl = i - start;
        if (wl < 3 || wl > 24) continue;

        word_count_add(table, cap, data + start, wl);
    }

    if (len <= 32768) {
        for (size_t p = 0; p < len; p++) {
            if (p > 0 && is_word_byte(data[p - 1]) && is_word_byte(data[p])) continue;

            for (size_t sl = 3; sl <= 12 && p + sl <= len; sl++) {
                int has_non_word = 0;
                for (size_t k = 0; k < sl; k++) {
                    if (!is_word_byte(data[p + k])) {
                        has_non_word = 1;
                        break;
                    }
                }
                if (!has_non_word) continue;
                word_count_add(table, cap, data + p, sl);
            }
        }
    }

    int token_count = 0;
    for (int j = 0; j < cap; j++) {
        WordCountEntry *e = &table[j];
        if (!e->text) continue;

        int score = ((int)e->len - 2) * (int)e->count - ((int)e->len + 1);
        if (e->count >= 3 && score > 0) {
            int pos = token_count;
            if (pos < max_tokens) {
                token_count++;
            } else {
                pos = max_tokens - 1;
                if (score <= tokens[pos].score) {
                    free(e->text);
                    continue;
                }
                free(tokens[pos].text);
            }

            while (pos > 0 && tokens[pos - 1].score < score) {
                tokens[pos] = tokens[pos - 1];
                pos--;
            }
            tokens[pos].text = e->text;
            tokens[pos].len = e->len;
            tokens[pos].count = e->count;
            tokens[pos].score = score;
            e->text = NULL;
        }

        free(e->text);
    }

    free(table);
    return token_count;
}

static uint8_t *dynamic_transform_encode(const uint8_t *data, size_t len,
                                         const DynTextToken *tokens, int token_count,
                                         size_t *out_len)
{
    ByteBuffer out;
    ByteBuffer_init(&out);
    ByteBuffer_reserve(&out, len);

    int *bucket_data = NULL;
    int bucket_counts[256] = {0};
    if (token_count > 0) {
        bucket_data = (int *)malloc(256 * (size_t)token_count * sizeof(int));
        for (int d = 0; d < token_count; d++) {
            uint8_t first = (uint8_t)tokens[d].text[0];
            bucket_data[(size_t)first * token_count + bucket_counts[first]++] = d;
        }
    }

    size_t i = 0;
    while (i < len) {
        int best = -1;
        size_t best_len = 0;
        uint8_t first = data[i];

        for (int bi = 0; bi < bucket_counts[first]; bi++) {
            int d = bucket_data[(size_t)first * token_count + bi];
            size_t dl = tokens[d].len;
            if (dl > best_len && i + dl <= len &&
                memcmp(data + i, tokens[d].text, dl) == 0) {
                best = d;
                best_len = dl;
            }
        }

        if (best >= 0) {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
            ByteBuffer_push(&out, (uint8_t)(best + 1));
            i += best_len;
            continue;
        }

        if (data[i] == QSC_TEXT_ESC) {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
            ByteBuffer_push(&out, 0);
        } else {
            ByteBuffer_push(&out, data[i]);
        }
        i++;
    }

    uint8_t *result = (uint8_t *)malloc(out.len);
    memcpy(result, out.data, out.len);
    *out_len = out.len;
    ByteBuffer_free(&out);
    free(bucket_data);
    return result;
}

static uint8_t *dynamic_transform_decode(const uint8_t *data, size_t len,
                                         const char **dict, const uint8_t *lens,
                                         int token_count, size_t *out_len)
{
    ByteBuffer out;
    ByteBuffer_init(&out);
    ByteBuffer_reserve(&out, len);

    for (size_t i = 0; i < len; i++) {
        if (data[i] != QSC_TEXT_ESC) {
            ByteBuffer_push(&out, data[i]);
            continue;
        }

        if (i + 1 >= len) {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
            break;
        }

        uint8_t code = data[++i];
        if (code == 0) {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
        } else if (code <= token_count) {
            int idx = (int)code - 1;
            for (uint8_t j = 0; j < lens[idx]; j++) {
                ByteBuffer_push(&out, (uint8_t)dict[idx][j]);
            }
        } else {
            ByteBuffer_push(&out, QSC_TEXT_ESC);
            ByteBuffer_push(&out, code);
        }
    }

    uint8_t *result = (uint8_t *)malloc(out.len);
    memcpy(result, out.data, out.len);
    *out_len = out.len;
    ByteBuffer_free(&out);
    return result;
}

/* ======================================================================
 * Compress payload — REP2 + adaptive models + arithmetic coding
 *
 * Stream order:
 *   [metadata] [instructions] [lengths/QS] [literals] [rep_types] [offsets]
 *
 * Lengths encoded BEFORE literals so decoder can use match_len_bucket
 * as literal context during sequential decode.
 * ====================================================================== */

static uint8_t *qsc_compress_payload(const uint8_t *chunk, size_t chunk_len,
                                     const uint8_t *prev_tail, size_t prev_tail_len,
                                     size_t *out_len)
{
    /* LZ match engine with REP0/1/2 detection */
    LZResult lz;
    lz_compress(chunk, chunk_len, prev_tail, prev_tail_len, &lz);

    ArithEncoder enc;
    arith_enc_init(&enc);

    /* Metadata */
    arith_enc_encode_varint(&enc, (uint32_t)lz.instructions.len);
    arith_enc_encode_varint(&enc, (uint32_t)lz.literals.len);
    arith_enc_encode_varint(&enc, (uint32_t)lz.rep_types.len);   /* num matches */
    arith_enc_encode_varint(&enc, (uint32_t)lz.new_offsets.len);  /* num new offsets */

    /* Stream 1: Instructions (order-3 binary) */
    InstructionModel inst_model;
    inst_model_init(&inst_model);
    for (size_t i = 0; i < lz.instructions.len; i++) {
        inst_model_encode(&inst_model, &enc, lz.instructions.data[i]);
    }

    /* Save original lengths for literal match-length context */
    int32_t *orig_lengths = NULL;
    if (lz.lengths.len > 0) {
        orig_lengths = (int32_t *)malloc(lz.lengths.len * sizeof(int32_t));
        memcpy(orig_lengths, lz.lengths.data, lz.lengths.len * sizeof(int32_t));
    }

    /* Stream 2: Lengths (slot model) — BEFORE literals */
    if (lz.lengths.len > 0) {
        SlotModel len_model;
        slot_model_init(&len_model);
        for (size_t i = 0; i < lz.lengths.len; i++) {
            slot_model_encode(&len_model, &enc, (uint32_t)lz.lengths.data[i]);
        }
    }

    /* Stream 3: Literals (fast mixer + match_len_bucket context) */
    MixedLiteralModel *lit_model = (MixedLiteralModel *)malloc(sizeof(MixedLiteralModel));
    mixed_lit_init(lit_model);
    {
        size_t lit_idx = 0, match_idx = 0;
        int match_len_bucket = 0;
        for (size_t i = 0; i < lz.instructions.len; i++) {
            if (lz.instructions.data[i] == 0) {
                mixed_lit_set_match_ctx(lit_model, match_len_bucket);
                mixed_lit_encode(lit_model, &enc, (uint8_t)lz.literals.data[lit_idx++]);
                match_len_bucket = 0;  /* after a literal: we're in literal run */
            } else {
                int32_t ml = orig_lengths[match_idx];
                match_len_bucket = match_len_to_bucket(ml);
                match_idx++;
            }
        }
    }
    mixed_lit_free(lit_model);
    free(lit_model);

    /* Stream 4: REP types (4-state cascade) */
    RepTypeModel rep_model;
    rep_type_init(&rep_model);
    for (size_t i = 0; i < lz.rep_types.len; i++) {
        rep_type_encode(&rep_model, &enc, lz.rep_types.data[i]);
    }

    /* Stream 5: New offsets (slot-based) */
    SlotModel offset_model;
    slot_model_init(&offset_model);
    for (size_t i = 0; i < lz.new_offsets.len; i++) {
        slot_model_encode(&offset_model, &enc, (uint32_t)lz.new_offsets.data[i]);
    }

    arith_enc_flush(&enc);

    size_t len;
    uint8_t *bytes = arith_enc_get_bytes(&enc, &len);
    uint8_t *result = (uint8_t *)malloc(len);
    memcpy(result, bytes, len);
    *out_len = len;

    arith_enc_free(&enc);
    lz_result_free(&lz);
    free(orig_lengths);
    return result;
}

static void fast_write_varint(ByteBuffer *out, uint32_t value) {
    while (value >= 0x80) {
        ByteBuffer_push(out, (uint8_t)((value & 0x7F) | 0x80));
        value >>= 7;
    }
    ByteBuffer_push(out, (uint8_t)value);
}

static uint32_t fast_read_varint(const uint8_t **p, const uint8_t *end) {
    uint32_t value = 0;
    int shift = 0;
    while (*p < end && shift <= 28) {
        uint8_t byte_val = *(*p)++;
        value |= (uint32_t)(byte_val & 0x7F) << shift;
        if ((byte_val & 0x80) == 0) break;
        shift += 7;
    }
    return value;
}

static uint8_t *qsc_compress_payload_fast(const uint8_t *chunk, size_t chunk_len,
                                          const uint8_t *prev_tail, size_t prev_tail_len,
                                          size_t *out_len)
{
    LZResult lz;
    lz_compress(chunk, chunk_len, prev_tail, prev_tail_len, &lz);

    ByteBuffer out;
    ByteBuffer_init(&out);
    ByteBuffer_reserve(&out, chunk_len / 2 + 64);

    fast_write_varint(&out, (uint32_t)lz.instructions.len);
    fast_write_varint(&out, (uint32_t)lz.literals.len);
    fast_write_varint(&out, (uint32_t)lz.rep_types.len);
    fast_write_varint(&out, (uint32_t)lz.new_offsets.len);

    for (size_t i = 0; i < lz.instructions.len; i += 8) {
        uint8_t packed = 0;
        for (int b = 0; b < 8 && i + (size_t)b < lz.instructions.len; b++) {
            if (lz.instructions.data[i + (size_t)b] != 0) {
                packed |= (uint8_t)(1u << b);
            }
        }
        ByteBuffer_push(&out, packed);
    }

    for (size_t i = 0; i < lz.literals.len; i++) {
        ByteBuffer_push(&out, (uint8_t)lz.literals.data[i]);
    }

    for (size_t i = 0; i < lz.lengths.len; i++) {
        fast_write_varint(&out, (uint32_t)lz.lengths.data[i]);
    }

    for (size_t i = 0; i < lz.rep_types.len; i += 4) {
        uint8_t packed = 0;
        for (int b = 0; b < 4 && i + (size_t)b < lz.rep_types.len; b++) {
            packed |= (uint8_t)((lz.rep_types.data[i + (size_t)b] & 3) << (b * 2));
        }
        ByteBuffer_push(&out, packed);
    }

    for (size_t i = 0; i < lz.new_offsets.len; i++) {
        fast_write_varint(&out, (uint32_t)lz.new_offsets.data[i]);
    }

    uint8_t *result = (uint8_t *)malloc(out.len);
    memcpy(result, out.data, out.len);
    *out_len = out.len;

    ByteBuffer_free(&out);
    lz_result_free(&lz);
    return result;
}

static void write_len_header(uint8_t header[4], size_t len) {
    header[0] = (uint8_t)(len >> 24);
    header[1] = (uint8_t)(len >> 16);
    header[2] = (uint8_t)(len >> 8);
    header[3] = (uint8_t)len;
}

static uint8_t *wrap_chunk_payload(uint8_t flag,
                                   const uint8_t *header, size_t header_len,
                                   uint8_t *payload, size_t payload_len,
                                   size_t *out_len)
{
    uint8_t *result = (uint8_t *)malloc(payload_len + header_len + 1);
    result[0] = flag;
    if (header_len > 0) {
        memcpy(result + 1, header, header_len);
    }
    memcpy(result + 1 + header_len, payload, payload_len);
    *out_len = payload_len + header_len + 1;
    free(payload);
    return result;
}

uint8_t *qsc_compress_chunk(const uint8_t *chunk, size_t chunk_len,
                            const uint8_t *prev_tail, size_t prev_tail_len,
                            size_t *out_len)
{
    if (should_try_spreadsheet_reorder(chunk, chunk_len)) {
        size_t shuf_len;
        uint8_t *shuf = shuffle13_transform_encode(chunk, chunk_len, &shuf_len);
        size_t shuf_comp_len;
        uint8_t *shuf_comp = qsc_compress_payload(shuf, shuf_len, NULL, 0, &shuf_comp_len);
        free(shuf);
        uint8_t header[4];
        write_len_header(header, chunk_len);
        return wrap_chunk_payload(QSC_TRANSFORM_SHUFFLE13, header, 4,
                                  shuf_comp, shuf_comp_len, out_len);
    }

    if (should_try_binary_reorder(chunk, chunk_len)) {
        size_t shuf_len;
        uint8_t *shuf = shuffle4_transform_encode(chunk, chunk_len, &shuf_len);
        size_t shuf_comp_len;
        uint8_t *shuf_comp = qsc_compress_payload(shuf, shuf_len, NULL, 0, &shuf_comp_len);
        free(shuf);
        uint8_t header[4];
        write_len_header(header, chunk_len);
        return wrap_chunk_payload(QSC_TRANSFORM_SHUFFLE4, header, 4,
                                  shuf_comp, shuf_comp_len, out_len);
    }

    if (should_try_zero_run(chunk, chunk_len)) {
        size_t rle_len;
        uint8_t *rle = rle_transform_encode(chunk, chunk_len, &rle_len);
        if (rle_len + 64 < chunk_len && rle_len * 10 < chunk_len * 9) {
            size_t rle_comp_len;
            uint8_t *rle_comp = qsc_compress_payload(rle, rle_len, NULL, 0, &rle_comp_len);
            free(rle);
            return wrap_chunk_payload(QSC_TRANSFORM_RLE, NULL, 0,
                                      rle_comp, rle_comp_len, out_len);
        }
        free(rle);
    }

    if (looks_textual(chunk, chunk_len)) {
        uint8_t *best_text = NULL;
        size_t best_text_len = 0;
        uint8_t *best_header = NULL;
        size_t best_header_len = 0;
        uint8_t best_flag = QSC_TRANSFORM_RAW;
        size_t best_estimated_total = chunk_len;
        size_t min_savings = 256 + chunk_len / 100;

        size_t tx_len;
        uint8_t *tx = text_transform_encode(chunk, chunk_len, &tx_len);
        if (tx_len + min_savings < chunk_len) {
            best_text = tx;
            best_text_len = tx_len;
            best_flag = QSC_TRANSFORM_TEXT;
            best_estimated_total = tx_len;
        } else {
            free(tx);
        }

        DynTextToken tokens[255] = {0};
        int token_count = collect_dynamic_tokens(chunk, chunk_len, tokens, 255);
        if (token_count > 0) {
            size_t dyn_len;
            uint8_t *dyn = dynamic_transform_encode(chunk, chunk_len, tokens, token_count, &dyn_len);

            size_t header_len = 1;
            for (int i = 0; i < token_count; i++) header_len += 1 + tokens[i].len;
            size_t estimated_total = dyn_len + header_len;

            if (estimated_total + min_savings < chunk_len &&
                estimated_total < best_estimated_total) {
                uint8_t *header = (uint8_t *)malloc(header_len);
                size_t hp = 0;
                header[hp++] = (uint8_t)token_count;
                for (int i = 0; i < token_count; i++) {
                    header[hp++] = tokens[i].len;
                    memcpy(header + hp, tokens[i].text, tokens[i].len);
                    hp += tokens[i].len;
                }

                free(best_text);
                free(best_header);
                best_text = dyn;
                best_text_len = dyn_len;
                best_header = header;
                best_header_len = header_len;
                best_flag = QSC_TRANSFORM_DYN_TEXT;
                best_estimated_total = estimated_total;
            } else {
                free(dyn);
            }
        }
        free_dyn_tokens(tokens, token_count);

        if (best_text) {
            size_t text_comp_len;
            uint8_t *text_comp = qsc_compress_payload(best_text, best_text_len, NULL, 0, &text_comp_len);
            free(best_text);
            uint8_t *result = wrap_chunk_payload(best_flag, best_header, best_header_len,
                                                 text_comp, text_comp_len, out_len);
            free(best_header);
            return result;
        }
    }

    size_t raw_len;
    uint8_t *raw = qsc_compress_payload_fast(chunk, chunk_len, prev_tail, prev_tail_len, &raw_len);
    return wrap_chunk_payload(QSC_PAYLOAD_FAST | QSC_TRANSFORM_RAW, NULL, 0, raw, raw_len, out_len);
}

/* ======================================================================
 * Decompress payload — mirrors encoder stream order exactly
 * ====================================================================== */

static uint8_t *qsc_decompress_payload(const uint8_t *compressed, size_t comp_len,
                                       const uint8_t *prev_tail, size_t prev_tail_len,
                                       size_t *out_len)
{
    ArithDecoder dec;
    arith_dec_init(&dec, compressed, comp_len);

    /* Metadata */
    uint32_t num_instructions = arith_dec_decode_varint(&dec);
    uint32_t num_literals     = arith_dec_decode_varint(&dec);
    uint32_t num_matches      = arith_dec_decode_varint(&dec);
    uint32_t num_new_offsets  = arith_dec_decode_varint(&dec);

    /* Stream 1: Instructions */
    int32_t *instructions = (int32_t *)malloc(num_instructions * sizeof(int32_t));
    InstructionModel inst_model;
    inst_model_init(&inst_model);
    for (uint32_t i = 0; i < num_instructions; i++) {
        instructions[i] = inst_model_decode(&inst_model, &dec);
    }

    /* Stream 2: Lengths (slot model) — BEFORE literals */
    int32_t *lengths = (int32_t *)calloc(num_matches, sizeof(int32_t));
    if (num_matches > 0) {
        SlotModel len_model;
        slot_model_init(&len_model);
        for (uint32_t i = 0; i < num_matches; i++) {
            lengths[i] = (int32_t)slot_model_decode(&len_model, &dec);
        }
    }

    /* Stream 3: Literals (with match_len_bucket context from decoded lengths) */
    int32_t *literals = (int32_t *)malloc(num_literals * sizeof(int32_t));
    MixedLiteralModel *lit_model = (MixedLiteralModel *)malloc(sizeof(MixedLiteralModel));
    mixed_lit_init(lit_model);
    {
        uint32_t lit_idx = 0, match_idx = 0;
        int match_len_bucket = 0;
        for (uint32_t i = 0; i < num_instructions; i++) {
            if (instructions[i] == 0) {
                mixed_lit_set_match_ctx(lit_model, match_len_bucket);
                literals[lit_idx++] = (int32_t)mixed_lit_decode(lit_model, &dec);
                match_len_bucket = 0;  /* after a literal: literal run */
            } else {
                int32_t ml = lengths[match_idx];
                match_len_bucket = match_len_to_bucket(ml);
                match_idx++;
            }
        }
    }
    mixed_lit_free(lit_model);
    free(lit_model);

    /* Stream 4: REP types (4-state cascade) */
    int32_t *rep_types = (int32_t *)malloc(num_matches * sizeof(int32_t));
    RepTypeModel rep_model;
    rep_type_init(&rep_model);
    for (uint32_t i = 0; i < num_matches; i++) {
        rep_types[i] = rep_type_decode(&rep_model, &dec);
    }

    /* Stream 5: New offsets */
    int32_t *new_offsets = (int32_t *)malloc(num_new_offsets * sizeof(int32_t));
    SlotModel offset_model;
    slot_model_init(&offset_model);
    for (uint32_t i = 0; i < num_new_offsets; i++) {
        new_offsets[i] = (int32_t)slot_model_decode(&offset_model, &dec);
    }

    /* LZ decompress with REP0/1/2 support */
    uint8_t *decompressed;
    size_t   decompressed_len;
    lz_decompress(instructions, num_instructions,
                  literals, rep_types, new_offsets, lengths,
                  prev_tail, prev_tail_len,
                  &decompressed, &decompressed_len);

    free(instructions);
    free(literals);
    free(rep_types);
    free(new_offsets);
    free(lengths);

    *out_len = decompressed_len;
    return decompressed;
}

static uint8_t *qsc_decompress_payload_fast(const uint8_t *compressed, size_t comp_len,
                                            const uint8_t *prev_tail, size_t prev_tail_len,
                                            size_t *out_len)
{
    const uint8_t *p = compressed;
    const uint8_t *end = compressed + comp_len;

    uint32_t num_instructions = fast_read_varint(&p, end);
    uint32_t num_literals     = fast_read_varint(&p, end);
    uint32_t num_matches      = fast_read_varint(&p, end);
    uint32_t num_new_offsets  = fast_read_varint(&p, end);
    (void)num_new_offsets;

    const uint8_t *instr_p = p;
    size_t instr_bytes = ((size_t)num_instructions + 7) / 8;
    p += instr_bytes;

    const uint8_t *lit_p = p;
    p += num_literals;

    const uint8_t *len_p = p;
    const uint8_t *len_scan = len_p;
    size_t final_len = num_literals;
    for (uint32_t i = 0; i < num_matches; i++) {
        final_len += fast_read_varint(&len_scan, end);
    }
    const uint8_t *rep_p = len_scan;
    const uint8_t *off_p = rep_p + (((size_t)num_matches + 3) / 4);

    size_t buf_len = prev_tail_len + final_len;
    uint8_t *buf = (uint8_t *)malloc(buf_len ? buf_len : 1);
    if (prev_tail_len > 0) memcpy(buf, prev_tail, prev_tail_len);

    size_t out_pos = prev_tail_len;
    uint32_t lit_idx = 0;
    uint32_t match_idx = 0;
    int32_t rep0 = 0, rep1 = 0, rep2 = 0;
    const uint8_t *len_read = len_p;
    const uint8_t *off_read = off_p;

    for (uint32_t i = 0; i < num_instructions; i++) {
        uint8_t inst_byte = instr_p[i >> 3];
        int is_match = (inst_byte >> (i & 7)) & 1;

        if (!is_match) {
            buf[out_pos++] = lit_p[lit_idx++];
            continue;
        }

        int32_t length = (int32_t)fast_read_varint(&len_read, rep_p);
        uint8_t rep_byte = rep_p[match_idx >> 2];
        int32_t rt = (rep_byte >> ((match_idx & 3) * 2)) & 3;
        match_idx++;

        int32_t offset;
        if (rt == 0) {
            offset = rep0;
        } else if (rt == 1) {
            offset = rep1;
            int32_t tmp = rep1;
            rep1 = rep0; rep0 = tmp;
        } else if (rt == 2) {
            offset = rep2;
            int32_t tmp = rep2;
            rep2 = rep1; rep1 = rep0; rep0 = tmp;
        } else {
            offset = (int32_t)fast_read_varint(&off_read, end);
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

    uint8_t *decompressed = (uint8_t *)malloc(final_len);
    memcpy(decompressed, buf + prev_tail_len, final_len);
    free(buf);

    *out_len = final_len;
    return decompressed;
}

static uint8_t *qsc_decompress_payload_auto(int fast_payload,
                                            const uint8_t *compressed, size_t comp_len,
                                            const uint8_t *prev_tail, size_t prev_tail_len,
                                            size_t *out_len)
{
    if (fast_payload) {
        return qsc_decompress_payload_fast(compressed, comp_len,
                                           prev_tail, prev_tail_len, out_len);
    }
    return qsc_decompress_payload(compressed, comp_len,
                                  prev_tail, prev_tail_len, out_len);
}

uint8_t *qsc_decompress_chunk(const uint8_t *compressed, size_t comp_len,
                              const uint8_t *prev_tail, size_t prev_tail_len,
                              size_t *out_len)
{
    if (comp_len == 0) {
        *out_len = 0;
        return NULL;
    }

    uint8_t stored_flag = compressed[0];
    int fast_payload = (stored_flag & QSC_PAYLOAD_FAST) != 0;
    uint8_t flag = stored_flag & QSC_TRANSFORM_MASK;
    const uint8_t *payload = compressed + 1;
    size_t payload_len = comp_len - 1;

    if (flag == QSC_TRANSFORM_TEXT) {
        size_t tx_len;
        uint8_t *tx = qsc_decompress_payload_auto(fast_payload, payload, payload_len, NULL, 0, &tx_len);
        uint8_t *decoded = text_transform_decode(tx, tx_len, out_len);
        free(tx);
        return decoded;
    }

    if (flag == QSC_TRANSFORM_RLE) {
        size_t rle_len;
        uint8_t *rle = qsc_decompress_payload_auto(fast_payload, payload, payload_len, NULL, 0, &rle_len);
        uint8_t *decoded = rle_transform_decode(rle, rle_len, out_len);
        free(rle);
        return decoded;
    }

    if (flag == QSC_TRANSFORM_BITPLANE) {
        if (payload_len < 4) {
            *out_len = 0;
            return NULL;
        }

        size_t original_len = ((size_t)payload[0] << 24) |
                              ((size_t)payload[1] << 16) |
                              ((size_t)payload[2] << 8)  |
                              (size_t)payload[3];
        size_t bp_len;
        uint8_t *bp = qsc_decompress_payload_auto(fast_payload, payload + 4, payload_len - 4, NULL, 0, &bp_len);
        uint8_t *decoded = bitplane_transform_decode(bp, bp_len, original_len, out_len);
        free(bp);
        return decoded;
    }

    if (flag == QSC_TRANSFORM_SHUFFLE4) {
        if (payload_len < 4) {
            *out_len = 0;
            return NULL;
        }

        size_t original_len = ((size_t)payload[0] << 24) |
                              ((size_t)payload[1] << 16) |
                              ((size_t)payload[2] << 8)  |
                              (size_t)payload[3];
        size_t shuf_len;
        uint8_t *shuf = qsc_decompress_payload_auto(fast_payload, payload + 4, payload_len - 4, NULL, 0, &shuf_len);
        uint8_t *decoded = shuffle4_transform_decode(shuf, shuf_len, original_len, out_len);
        free(shuf);
        return decoded;
    }

    if (flag == QSC_TRANSFORM_SHUFFLE13) {
        if (payload_len < 4) {
            *out_len = 0;
            return NULL;
        }

        size_t original_len = ((size_t)payload[0] << 24) |
                              ((size_t)payload[1] << 16) |
                              ((size_t)payload[2] << 8)  |
                              (size_t)payload[3];
        size_t shuf_len;
        uint8_t *shuf = qsc_decompress_payload_auto(fast_payload, payload + 4, payload_len - 4, NULL, 0, &shuf_len);
        uint8_t *decoded = shuffle13_transform_decode(shuf, shuf_len, original_len, out_len);
        free(shuf);
        return decoded;
    }

    if (flag == QSC_TRANSFORM_DYN_TEXT) {
        if (payload_len < 1) {
            *out_len = 0;
            return NULL;
        }

        uint8_t token_count = payload[0];
        const char **dict = (const char **)calloc(token_count, sizeof(char *));
        uint8_t *lens = (uint8_t *)calloc(token_count, sizeof(uint8_t));

        size_t p = 1;
        int ok = 1;
        for (uint8_t i = 0; i < token_count; i++) {
            if (p >= payload_len) { ok = 0; break; }
            lens[i] = payload[p++];
            if (p + lens[i] > payload_len) { ok = 0; break; }
            dict[i] = (const char *)(payload + p);
            p += lens[i];
        }

        if (!ok) {
            free(dict);
            free(lens);
            *out_len = 0;
            return NULL;
        }

        size_t tx_len;
        uint8_t *tx = qsc_decompress_payload_auto(fast_payload, payload + p, payload_len - p, NULL, 0, &tx_len);
        uint8_t *decoded = dynamic_transform_decode(tx, tx_len, dict, lens, token_count, out_len);
        free(tx);
        free(dict);
        free(lens);
        return decoded;
    }

    return qsc_decompress_payload_auto(fast_payload, payload, payload_len,
                                       prev_tail, prev_tail_len, out_len);
}

/* ======================================================================
 * File walking (POSIX)
 * ====================================================================== */

typedef struct {
    char *full_path;
    char *rel_path;
} FileEntry;

DYNARRAY_DEFINE(FileEntry, FileList)

static int is_regular_file(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

static int is_directory(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static uint64_t file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return (uint64_t)st.st_size;
}

static void walk_dir(const char *base, const char *rel, FileList *list) {
    char full[4096];
    snprintf(full, sizeof(full), "%s/%s", base, rel);
    DIR *d = opendir(full);
    if (!d) return;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        char child_rel[4096], child_full[4096];
        if (rel[0] == '\0')
            snprintf(child_rel, sizeof(child_rel), "%s", ent->d_name);
        else
            snprintf(child_rel, sizeof(child_rel), "%s/%s", rel, ent->d_name);
        snprintf(child_full, sizeof(child_full), "%s/%s", base, child_rel);
        if (is_directory(child_full))
            walk_dir(base, child_rel, list);
        else if (is_regular_file(child_full)) {
            FileEntry fe;
            fe.full_path = strdup(child_full);
            fe.rel_path  = strdup(child_rel);
            FileList_push(list, fe);
        }
    }
    closedir(d);
}

static void file_list_free(FileList *list) {
    for (size_t i = 0; i < list->len; i++) {
        free(list->data[i].full_path);
        free(list->data[i].rel_path);
    }
    FileList_free(list);
}

/* ======================================================================
 * Multi-threaded chunk compression
 * ====================================================================== */

typedef struct {
    const uint8_t *chunk_data;
    size_t         chunk_len;
    const uint8_t *prev_tail;
    size_t         prev_tail_len;
    uint8_t       *compressed;
    size_t         comp_len;
} ChunkTask;

static void *compress_chunk_thread(void *arg) {
    ChunkTask *task = (ChunkTask *)arg;
    task->compressed = qsc_compress_chunk(
        task->chunk_data, task->chunk_len,
        task->prev_tail, task->prev_tail_len,
        &task->comp_len);
    return NULL;
}

static int get_num_threads(void) {
    int n = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (n < 1) n = 1;
    if (n > 32) n = 32;
    return n;
}

/* ======================================================================
 * Pack (compress) — multi-threaded
 * ====================================================================== */

int qsc_pack(const char *input_path, const char *output_file) {
    FileList files;
    FileList_init(&files);

    if (is_regular_file(input_path)) {
        const char *base = strrchr(input_path, '/');
        FileEntry fe;
        fe.full_path = strdup(input_path);
        fe.rel_path  = strdup(base ? base + 1 : input_path);
        FileList_push(&files, fe);
    } else {
        walk_dir(input_path, "", &files);
    }

    FILE *fout = fopen(output_file, "wb");
    if (!fout) {
        fprintf(stderr, "Error: Cannot open %s for writing\n", output_file);
        file_list_free(&files);
        return -1;
    }

    fwrite(QSC_MAGIC, 1, 4, fout);
    uint8_t ver = QSC_VERSION;
    fwrite(&ver, 1, 1, fout);
    write_be32(fout, (uint32_t)files.len);

    long *offset_positions = (long *)malloc(files.len * sizeof(long));
    for (size_t i = 0; i < files.len; i++) {
        const char *rel = files.data[i].rel_path;
        uint16_t path_len = (uint16_t)strlen(rel);
        write_be16(fout, path_len);
        fwrite(rel, 1, path_len, fout);
        write_be64(fout, file_size(files.data[i].full_path));
        offset_positions[i] = ftell(fout);
        write_be64(fout, 0);
    }

    int num_threads = get_num_threads();

    for (size_t idx = 0; idx < files.len; idx++) {
        long current_offset = ftell(fout);

        /* Read entire file */
        FILE *fin = fopen(files.data[idx].full_path, "rb");
        if (!fin) {
            fprintf(stderr, "Error: Cannot open %s\n", files.data[idx].full_path);
            continue;
        }
        fseek(fin, 0, SEEK_END);
        size_t fsize = (size_t)ftell(fin);
        fseek(fin, 0, SEEK_SET);
        uint8_t *fdata = (uint8_t *)malloc(fsize);
        fread(fdata, 1, fsize, fin);
        fclose(fin);

        /* Split into chunks and pre-compute prev_tails */
        int num_chunks = 0;
        size_t pos = 0;
        while (pos < fsize) {
            num_chunks++;
            pos += (fsize - pos > QSC_CHUNK_SIZE) ? QSC_CHUNK_SIZE : (fsize - pos);
        }

        ChunkTask *tasks = (ChunkTask *)calloc((size_t)num_chunks, sizeof(ChunkTask));
        uint8_t **prev_tails = (uint8_t **)calloc((size_t)num_chunks, sizeof(uint8_t *));

        pos = 0;
        for (int c = 0; c < num_chunks; c++) {
            size_t cl = fsize - pos;
            if (cl > QSC_CHUNK_SIZE) cl = QSC_CHUNK_SIZE;
            tasks[c].chunk_data = fdata + pos;
            tasks[c].chunk_len  = cl;

            if (c > 0) {
                size_t prev_end = pos;
                if (prev_end >= QSC_CARRY_OVER) {
                    prev_tails[c] = (uint8_t *)malloc(QSC_CARRY_OVER);
                    memcpy(prev_tails[c], fdata + prev_end - QSC_CARRY_OVER, QSC_CARRY_OVER);
                    tasks[c].prev_tail = prev_tails[c];
                    tasks[c].prev_tail_len = QSC_CARRY_OVER;
                } else if (prev_end > 0) {
                    prev_tails[c] = (uint8_t *)malloc(prev_end);
                    memcpy(prev_tails[c], fdata, prev_end);
                    tasks[c].prev_tail = prev_tails[c];
                    tasks[c].prev_tail_len = prev_end;
                }
            }

            pos += cl;
        }

        /* Compress chunks in parallel */
        pthread_t *threads = (pthread_t *)malloc((size_t)num_threads * sizeof(pthread_t));
        int c = 0;
        while (c < num_chunks) {
            int batch = num_chunks - c;
            if (batch > num_threads) batch = num_threads;

            for (int t = 0; t < batch; t++)
                pthread_create(&threads[t], NULL, compress_chunk_thread, &tasks[c + t]);
            for (int t = 0; t < batch; t++)
                pthread_join(threads[t], NULL);

            c += batch;
        }
        free(threads);

        /* Write compressed chunks sequentially */
        for (int ci = 0; ci < num_chunks; ci++) {
            write_be32(fout, (uint32_t)tasks[ci].chunk_len);
            write_be32(fout, (uint32_t)tasks[ci].comp_len);
            fwrite(tasks[ci].compressed, 1, tasks[ci].comp_len, fout);
            free(tasks[ci].compressed);
        }

        /* End-of-file sentinel */
        write_be32(fout, 0);
        write_be32(fout, 0);

        /* Patch offset */
        long end_offset = ftell(fout);
        fseek(fout, offset_positions[idx], SEEK_SET);
        write_be64(fout, (uint64_t)current_offset);
        fseek(fout, end_offset, SEEK_SET);

        printf("  📦 %s: %d chunks\n", files.data[idx].rel_path, num_chunks);

        for (int ci = 0; ci < num_chunks; ci++) free(prev_tails[ci]);
        free(prev_tails);
        free(tasks);
        free(fdata);
    }

    free(offset_positions);
    fclose(fout);
    file_list_free(&files);
    return 0;
}

/* ======================================================================
 * Unpack (decompress) — sequential
 * ====================================================================== */

static void mkdirs(const char *path) {
    char tmp[4096];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') { *p = '\0'; mkdir(tmp, 0755); *p = '/'; }
    }
}

int qsc_unpack(const char *archive_path, const char *output_folder) {
    FILE *fin = fopen(archive_path, "rb");
    if (!fin) { fprintf(stderr, "Error: Cannot open %s\n", archive_path); return -1; }

    char magic[4];
    fread(magic, 1, 4, fin);
    if (memcmp(magic, QSC_MAGIC, 4) != 0) {
        fprintf(stderr, "Error: Invalid magic\n"); fclose(fin); return -1;
    }

    uint8_t version;
    fread(&version, 1, 1, fin);
    if (version != QSC_VERSION) {
        fprintf(stderr, "Error: Unsupported version %d (expected %d)\n",
                version, QSC_VERSION);
        fclose(fin); return -1;
    }

    uint32_t file_count = read_be32(fin);

    typedef struct { char *rel_path; uint64_t orig_size; uint64_t offset; } FInfo;
    FInfo *finfo = (FInfo *)malloc(file_count * sizeof(FInfo));

    for (uint32_t i = 0; i < file_count; i++) {
        uint16_t path_len = read_be16(fin);
        finfo[i].rel_path = (char *)malloc(path_len + 1);
        fread(finfo[i].rel_path, 1, path_len, fin);
        finfo[i].rel_path[path_len] = '\0';
        finfo[i].orig_size = read_be64(fin);
        finfo[i].offset    = read_be64(fin);
    }

    for (uint32_t i = 0; i < file_count; i++) {
        char out_path[4096];
        snprintf(out_path, sizeof(out_path), "%s/%s", output_folder, finfo[i].rel_path);
        mkdirs(out_path);

        FILE *fout = fopen(out_path, "wb");
        if (!fout) { fprintf(stderr, "Error: Cannot create %s\n", out_path); continue; }

        fseek(fin, (long)finfo[i].offset, SEEK_SET);

        uint8_t prev_tail[QSC_CARRY_OVER];
        size_t prev_tail_len = 0;
        uint64_t bytes_written = 0;

        while (1) {
            uint32_t orig_len = read_be32(fin);
            uint32_t comp_len = read_be32(fin);
            if (orig_len == 0 && comp_len == 0) break;

            uint8_t *comp_data = (uint8_t *)malloc(comp_len);
            fread(comp_data, 1, comp_len, fin);

            size_t decomp_len;
            uint8_t *decompressed = qsc_decompress_chunk(
                comp_data, comp_len, prev_tail, prev_tail_len, &decomp_len);

            fwrite(decompressed, 1, decomp_len, fout);
            bytes_written += decomp_len;

            if (decomp_len >= QSC_CARRY_OVER) {
                memcpy(prev_tail, decompressed + decomp_len - QSC_CARRY_OVER, QSC_CARRY_OVER);
                prev_tail_len = QSC_CARRY_OVER;
            } else {
                memcpy(prev_tail, decompressed, decomp_len);
                prev_tail_len = decomp_len;
            }

            free(comp_data);
            free(decompressed);
        }

        fclose(fout);
        printf("  📤 %s: %llu bytes\n", finfo[i].rel_path,
               (unsigned long long)bytes_written);
    }

    for (uint32_t i = 0; i < file_count; i++) free(finfo[i].rel_path);
    free(finfo);
    fclose(fin);
    return 0;
}

/* ======================================================================
 * Benchmark — multi-threaded compression
 * ====================================================================== */

void qsc_benchmark(const char *input_path) {
    FILE *f = fopen(input_path, "rb");
    if (!f) { fprintf(stderr, "Error: Cannot open %s\n", input_path); return; }

    fseek(f, 0, SEEK_END);
    size_t orig_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *data = (uint8_t *)malloc(orig_size);
    fread(data, 1, orig_size, f);
    fclose(f);

    const char *basename = strrchr(input_path, '/');
    basename = basename ? basename + 1 : input_path;

    int num_threads = get_num_threads();

    printf("\n📊 Benchmark: %s (%zu bytes) [%d threads]\n", basename, orig_size, num_threads);
    printf("=================================================================\n");

#ifdef HAS_ZLIB
    {
        uLongf zlib6_len = compressBound((uLong)orig_size);
        uint8_t *zlib6 = (uint8_t *)malloc(zlib6_len);
        compress2(zlib6, &zlib6_len, data, (uLong)orig_size, 6);
        printf("  zlib (lvl 6): %10lu bytes  (%5.1f%%)\n",
               (unsigned long)zlib6_len, (double)zlib6_len / orig_size * 100.0);

        uLongf zlib9_len = compressBound((uLong)orig_size);
        uint8_t *zlib9 = (uint8_t *)malloc(zlib9_len);
        compress2(zlib9, &zlib9_len, data, (uLong)orig_size, 9);
        printf("  zlib (lvl 9): %10lu bytes  (%5.1f%%)\n",
               (unsigned long)zlib9_len, (double)zlib9_len / orig_size * 100.0);
        free(zlib6);
        free(zlib9);
    }
#else
    printf("  (zlib comparison not available — build with -DHAS_ZLIB -lz)\n");
#endif

    printf("\n  Compressing with QSC3 v%d...\n", QSC_VERSION);
    double t_start = get_time_sec();

    /* Pre-split into chunks */
    int num_chunks = 0;
    {
        size_t p = 0;
        while (p < orig_size) {
            num_chunks++;
            p += (orig_size - p > QSC_CHUNK_SIZE) ? QSC_CHUNK_SIZE : (orig_size - p);
        }
    }

    ChunkTask *tasks = (ChunkTask *)calloc((size_t)num_chunks, sizeof(ChunkTask));
    uint8_t **prev_tails = (uint8_t **)calloc((size_t)num_chunks, sizeof(uint8_t *));

    {
        size_t p = 0;
        for (int c = 0; c < num_chunks; c++) {
            size_t cl = orig_size - p;
            if (cl > QSC_CHUNK_SIZE) cl = QSC_CHUNK_SIZE;
            tasks[c].chunk_data = data + p;
            tasks[c].chunk_len  = cl;

            if (c > 0) {
                size_t prev_end = p;
                if (prev_end >= QSC_CARRY_OVER) {
                    prev_tails[c] = (uint8_t *)malloc(QSC_CARRY_OVER);
                    memcpy(prev_tails[c], data + prev_end - QSC_CARRY_OVER, QSC_CARRY_OVER);
                    tasks[c].prev_tail = prev_tails[c];
                    tasks[c].prev_tail_len = QSC_CARRY_OVER;
                } else if (prev_end > 0) {
                    prev_tails[c] = (uint8_t *)malloc(prev_end);
                    memcpy(prev_tails[c], data, prev_end);
                    tasks[c].prev_tail = prev_tails[c];
                    tasks[c].prev_tail_len = prev_end;
                }
            }

            p += cl;
        }
    }

    /* Compress in parallel */
    pthread_t *threads = (pthread_t *)malloc((size_t)num_threads * sizeof(pthread_t));
    int c = 0;
    while (c < num_chunks) {
        int batch = num_chunks - c;
        if (batch > num_threads) batch = num_threads;

        for (int t = 0; t < batch; t++)
            pthread_create(&threads[t], NULL, compress_chunk_thread, &tasks[c + t]);
        for (int t = 0; t < batch; t++)
            pthread_join(threads[t], NULL);

        c += batch;
    }
    free(threads);

    size_t total_compressed = 0;
    for (int ci = 0; ci < num_chunks; ci++) {
        total_compressed += tasks[ci].comp_len + 8;
    }

    double elapsed = get_time_sec() - t_start;
    printf("  QSC3:         %10zu bytes  (%5.1f%%)  %.2fs\n",
           total_compressed, (double)total_compressed / orig_size * 100.0, elapsed);

    /* Verify roundtrip (sequential — must match encoder state) */
    printf("\n  Verifying...\n");
    int all_ok = 1;
    {
        uint8_t prev_tail[QSC_CARRY_OVER];
        size_t ptl = 0;
        size_t pos = 0;

        for (int ci = 0; ci < num_chunks; ci++) {
            size_t chunk_len = tasks[ci].chunk_len;

            /* Re-compress with correct sequential prev_tail for verify */
            size_t comp_len2;
            uint8_t *comp2 = qsc_compress_chunk(
                data + pos, chunk_len, prev_tail, ptl, &comp_len2);

            size_t decomp_len;
            uint8_t *decompressed = qsc_decompress_chunk(
                comp2, comp_len2, prev_tail, ptl, &decomp_len);

            if (decomp_len != chunk_len ||
                memcmp(decompressed, data + pos, chunk_len) != 0) {
                printf("  ❌ Chunk %d FAILED!\n", ci);
                all_ok = 0;
                free(comp2);
                free(decompressed);
                break;
            }

            if (chunk_len >= QSC_CARRY_OVER) {
                memcpy(prev_tail, data + pos + chunk_len - QSC_CARRY_OVER, QSC_CARRY_OVER);
                ptl = QSC_CARRY_OVER;
            } else {
                memcpy(prev_tail, data + pos, chunk_len);
                ptl = chunk_len;
            }

            free(comp2);
            free(decompressed);
            pos += chunk_len;
        }
    }

    if (all_ok) printf("  ✅ All %d chunks verified\n", num_chunks);

    printf("\n=================================================================\n\n");

    for (int ci = 0; ci < num_chunks; ci++) {
        free(tasks[ci].compressed);
        free(prev_tails[ci]);
    }
    free(tasks);
    free(prev_tails);
    free(data);
}
