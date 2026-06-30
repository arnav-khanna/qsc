#include "context_model.h"
#include <stdlib.h>
#include <string.h>

/* ====================================================================== */
/* BitTreeModel                                                           */
/* ====================================================================== */

void bittree_init(BitTreeModel *m, int bits) {
    m->bits = bits;
    int n = 1 << bits;
    for (int i = 0; i < n; i++) { m->probs[i][0] = 1; m->probs[i][1] = 1; }
}

void bittree_update_node(BitTreeModel *m, int idx, int bit) {
    m->probs[idx][bit]++;
    if (m->probs[idx][0] + m->probs[idx][1] > CM_RESCALE_LIMIT) {
        m->probs[idx][0] = m->probs[idx][0] > 1 ? m->probs[idx][0] >> 1 : 1;
        m->probs[idx][1] = m->probs[idx][1] > 1 ? m->probs[idx][1] >> 1 : 1;
    }
}

uint32_t bittree_get_p0(const BitTreeModel *m, int idx) {
    uint32_t c0 = m->probs[idx][0], c1 = m->probs[idx][1];
    uint32_t p0 = (c0 * CM_SCALE) / (c0 + c1);
    if (p0 < 1) p0 = 1;
    if (p0 > CM_SCALE - 1) p0 = CM_SCALE - 1;
    return p0;
}

void bittree_encode(BitTreeModel *m, ArithEncoder *enc, uint8_t value) {
    int idx = 1;
    for (int bp = m->bits - 1; bp >= 0; bp--) {
        int bit = (value >> bp) & 1;
        arith_enc_encode(enc, bit, bittree_get_p0(m, idx), CM_SCALE);
        bittree_update_node(m, idx, bit);
        idx = idx * 2 + bit;
    }
}

uint8_t bittree_decode(BitTreeModel *m, ArithDecoder *dec) {
    int idx = 1;
    uint8_t value = 0;
    for (int bp = m->bits - 1; bp >= 0; bp--) {
        uint32_t p0 = bittree_get_p0(m, idx);
        int bit = arith_dec_decode(dec, p0, CM_SCALE);
        bittree_update_node(m, idx, bit);
        value = (value << 1) | (uint8_t)bit;
        idx = idx * 2 + bit;
    }
    return value;
}

/* ====================================================================== */
/* InstructionModel — order-3 (8 states)                                  */
/* ====================================================================== */

void inst_model_init(InstructionModel *m) {
    for (int i = 0; i < 8; i++) { m->probs[i][0] = 1; m->probs[i][1] = 1; }
    m->ctx = 0;
}

static inline void inst_update(InstructionModel *m, int inst) {
    m->probs[m->ctx][inst]++;
    if (m->probs[m->ctx][0] + m->probs[m->ctx][1] > CM_RESCALE_LIMIT) {
        m->probs[m->ctx][0] = m->probs[m->ctx][0] > 1 ? m->probs[m->ctx][0] >> 1 : 1;
        m->probs[m->ctx][1] = m->probs[m->ctx][1] > 1 ? m->probs[m->ctx][1] >> 1 : 1;
    }
    m->ctx = ((m->ctx << 1) | inst) & 7;  /* order-3: mask 0x7 */
}

static inline uint32_t inst_p0(const InstructionModel *m) {
    uint32_t c0 = m->probs[m->ctx][0], t = c0 + m->probs[m->ctx][1];
    uint32_t p0 = (c0 * CM_SCALE) / t;
    if (p0 < 1) p0 = 1; if (p0 > CM_SCALE - 1) p0 = CM_SCALE - 1;
    return p0;
}

void inst_model_encode(InstructionModel *m, ArithEncoder *enc, int inst) {
    arith_enc_encode(enc, inst, inst_p0(m), CM_SCALE);
    inst_update(m, inst);
}

int inst_model_decode(InstructionModel *m, ArithDecoder *dec) {
    int inst = arith_dec_decode(dec, inst_p0(m), CM_SCALE);
    inst_update(m, inst);
    return inst;
}

/* ====================================================================== */
/* Adaptive bit helper (shared by RepTypeModel)                           */
/* ====================================================================== */

static inline void adaptive_bit_enc(uint16_t p[2], ArithEncoder *enc, int bit) {
    uint32_t t = p[0] + p[1], p0 = (p[0] * CM_SCALE) / t;
    if (p0 < 1) p0 = 1; if (p0 > CM_SCALE - 1) p0 = CM_SCALE - 1;
    arith_enc_encode(enc, bit, p0, CM_SCALE);
    p[bit]++;
    if (p[0] + p[1] > CM_RESCALE_LIMIT) {
        p[0] = p[0] > 1 ? p[0] >> 1 : 1; p[1] = p[1] > 1 ? p[1] >> 1 : 1;
    }
}

static inline int adaptive_bit_dec(uint16_t p[2], ArithDecoder *dec) {
    uint32_t t = p[0] + p[1], p0 = (p[0] * CM_SCALE) / t;
    if (p0 < 1) p0 = 1; if (p0 > CM_SCALE - 1) p0 = CM_SCALE - 1;
    int bit = arith_dec_decode(dec, p0, CM_SCALE);
    p[bit]++;
    if (p[0] + p[1] > CM_RESCALE_LIMIT) {
        p[0] = p[0] > 1 ? p[0] >> 1 : 1; p[1] = p[1] > 1 ? p[1] >> 1 : 1;
    }
    return bit;
}

/* ====================================================================== */
/* RepTypeModel — 4-state cascade (rep0/rep1/rep2/new)                    */
/* ====================================================================== */

void rep_type_init(RepTypeModel *m) {
    for (int i = 0; i < 4; i++) { m->is_new_probs[i][0] = 1; m->is_new_probs[i][1] = 1; }
    for (int i = 0; i < 2; i++) { m->is_rep0_probs[i][0] = 1; m->is_rep0_probs[i][1] = 1; }
    for (int i = 0; i < 2; i++) { m->is_rep2_probs[i][0] = 1; m->is_rep2_probs[i][1] = 1; }
    m->is_new_ctx = 0; m->is_rep0_ctx = 0; m->is_rep2_ctx = 0;
}

void rep_type_encode(RepTypeModel *m, ArithEncoder *enc, int type) {
    /* type: 0=rep0, 1=rep1, 2=rep2, 3=new_offset */
    int is_new = (type == 3) ? 1 : 0;
    adaptive_bit_enc(m->is_new_probs[m->is_new_ctx], enc, is_new);
    m->is_new_ctx = ((m->is_new_ctx << 1) | is_new) & 3;

    if (!is_new) {
        int not_rep0 = (type == 0) ? 0 : 1;
        adaptive_bit_enc(m->is_rep0_probs[m->is_rep0_ctx], enc, not_rep0);
        m->is_rep0_ctx = not_rep0 & 1;

        if (not_rep0) {
            int is_rep2 = (type == 2) ? 1 : 0;
            adaptive_bit_enc(m->is_rep2_probs[m->is_rep2_ctx], enc, is_rep2);
            m->is_rep2_ctx = is_rep2 & 1;
        }
    }
}

int rep_type_decode(RepTypeModel *m, ArithDecoder *dec) {
    int is_new = adaptive_bit_dec(m->is_new_probs[m->is_new_ctx], dec);
    m->is_new_ctx = ((m->is_new_ctx << 1) | is_new) & 3;
    if (is_new) return 3;

    int not_rep0 = adaptive_bit_dec(m->is_rep0_probs[m->is_rep0_ctx], dec);
    m->is_rep0_ctx = not_rep0 & 1;
    if (!not_rep0) return 0;

    int is_rep2 = adaptive_bit_dec(m->is_rep2_probs[m->is_rep2_ctx], dec);
    m->is_rep2_ctx = is_rep2 & 1;
    return is_rep2 ? 2 : 1;
}

/* ====================================================================== */
/* Legacy LiteralModel                                                    */
/* ====================================================================== */

void lit_model_init(LiteralModel *m) {
    for (int i = 0; i < 16; i++) bittree_init(&m->trees[i], 8);
    m->last_msb = 0;
}
void lit_model_encode(LiteralModel *m, ArithEncoder *enc, uint8_t v) {
    bittree_encode(&m->trees[m->last_msb], enc, v); m->last_msb = (v >> 4) & 0xF;
}
uint8_t lit_model_decode(LiteralModel *m, ArithDecoder *dec) {
    uint8_t v = bittree_decode(&m->trees[m->last_msb], dec); m->last_msb = (v >> 4) & 0xF;
    return v;
}

/* ====================================================================== */
/* Character class + Word context helpers                                 */
/* ====================================================================== */

static inline uint8_t char_class(uint8_t c) {
    if (c >= 'a' && c <= 'z') return 3;
    if (c >= 'A' && c <= 'Z') return 2;
    if (c >= '0' && c <= '9') return 1;
    return 0;
}

static inline int word_ctx(uint8_t c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    return 26;
}

/* ====================================================================== */
/* MixedLiteralModel — 5-model fast fixed-weight mixer                    */
/*                                                                        */
/* Fast mix (no sigmoid/stretch/squash):                                  */
/*   p = (o1*6 + o2*5 + word*2 + cc*2 + o0*1) >> 4                      */
/* ====================================================================== */

void mixed_lit_init(MixedLiteralModel *m) {
    m->o1_trees = (BitTreeModel *)malloc(256 * sizeof(BitTreeModel));
    for (int i = 0; i < 256; i++) bittree_init(&m->o1_trees[i], 8);
    for (int i = 0; i < 4; i++) bittree_init(&m->o0_trees[i], 8);
    for (int i = 0; i < 4; i++) bittree_init(&m->cc_trees[i], 8);
    m->o2_trees = (BitTreeModel *)malloc(O2_NUM_CONTEXTS * sizeof(BitTreeModel));
    for (int i = 0; i < O2_NUM_CONTEXTS; i++) bittree_init(&m->o2_trees[i], 8);
    m->word_trees = (BitTreeModel *)malloc(WORD_NUM_CONTEXTS * sizeof(BitTreeModel));
    for (int i = 0; i < WORD_NUM_CONTEXTS; i++) bittree_init(&m->word_trees[i], 8);

    m->last_byte = 0; m->prev_prev_byte = 0; m->last_class = 0;
    m->match_len_bucket = 0;
}

static inline int o2_hash(uint8_t b1, uint8_t b0) {
    return ((int)b1 * 257 + (int)b0) & (O2_NUM_CONTEXTS - 1);
}

void mixed_lit_encode(MixedLiteralModel *m, ArithEncoder *enc, uint8_t byte_val) {
    BitTreeModel *o1 = &m->o1_trees[m->last_byte];
    BitTreeModel *o0 = &m->o0_trees[m->match_len_bucket];
    BitTreeModel *cc = &m->cc_trees[m->last_class];
    BitTreeModel *o2 = &m->o2_trees[o2_hash(m->prev_prev_byte, m->last_byte)];
    BitTreeModel *wd = &m->word_trees[word_ctx(m->last_byte)];

    int i1 = 1, i0 = 1, ic = 1, i2 = 1, iw = 1;

    for (int bp = 7; bp >= 0; bp--) {
        int bit = (byte_val >> bp) & 1;
        uint32_t p1 = bittree_get_p0(o1, i1);
        uint32_t p0 = bittree_get_p0(o0, i0);
        uint32_t pc = bittree_get_p0(cc, ic);
        uint32_t p2 = bittree_get_p0(o2, i2);
        uint32_t pw = bittree_get_p0(wd, iw);

        /* Fast fixed-weight mix: o1*6 + o2*5 + word*2 + cc*2 + o0*1 = 16 */
        uint32_t mixed = (p1*6 + p2*5 + pw*2 + pc*2 + p0) >> 4;
        if (mixed < 1) mixed = 1;
        if (mixed > CM_SCALE - 1) mixed = CM_SCALE - 1;

        arith_enc_encode(enc, bit, mixed, CM_SCALE);

        bittree_update_node(o1, i1, bit); i1 = i1 * 2 + bit;
        bittree_update_node(o0, i0, bit); i0 = i0 * 2 + bit;
        bittree_update_node(cc, ic, bit); ic = ic * 2 + bit;
        bittree_update_node(o2, i2, bit); i2 = i2 * 2 + bit;
        bittree_update_node(wd, iw, bit); iw = iw * 2 + bit;
    }
    m->prev_prev_byte = m->last_byte; m->last_byte = byte_val;
    m->last_class = char_class(byte_val);
}

uint8_t mixed_lit_decode(MixedLiteralModel *m, ArithDecoder *dec) {
    BitTreeModel *o1 = &m->o1_trees[m->last_byte];
    BitTreeModel *o0 = &m->o0_trees[m->match_len_bucket];
    BitTreeModel *cc = &m->cc_trees[m->last_class];
    BitTreeModel *o2 = &m->o2_trees[o2_hash(m->prev_prev_byte, m->last_byte)];
    BitTreeModel *wd = &m->word_trees[word_ctx(m->last_byte)];

    int i1 = 1, i0 = 1, ic = 1, i2 = 1, iw = 1;
    uint8_t value = 0;

    for (int bp = 7; bp >= 0; bp--) {
        uint32_t p1 = bittree_get_p0(o1, i1);
        uint32_t p0 = bittree_get_p0(o0, i0);
        uint32_t pc = bittree_get_p0(cc, ic);
        uint32_t p2 = bittree_get_p0(o2, i2);
        uint32_t pw = bittree_get_p0(wd, iw);

        /* Fast fixed-weight mix: same as encoder */
        uint32_t mixed = (p1*6 + p2*5 + pw*2 + pc*2 + p0) >> 4;
        if (mixed < 1) mixed = 1;
        if (mixed > CM_SCALE - 1) mixed = CM_SCALE - 1;

        int bit = arith_dec_decode(dec, mixed, CM_SCALE);

        bittree_update_node(o1, i1, bit); i1 = i1 * 2 + bit;
        bittree_update_node(o0, i0, bit); i0 = i0 * 2 + bit;
        bittree_update_node(cc, ic, bit); ic = ic * 2 + bit;
        bittree_update_node(o2, i2, bit); i2 = i2 * 2 + bit;
        bittree_update_node(wd, iw, bit); iw = iw * 2 + bit;
        value = (value << 1) | (uint8_t)bit;
    }
    m->prev_prev_byte = m->last_byte; m->last_byte = value;
    m->last_class = char_class(value);
    return value;
}

void mixed_lit_set_match_ctx(MixedLiteralModel *m, int bucket) {
    m->match_len_bucket = bucket;
}

void mixed_lit_free(MixedLiteralModel *m) {
    free(m->o1_trees); free(m->o2_trees); free(m->word_trees);
    m->o1_trees = NULL; m->o2_trees = NULL; m->word_trees = NULL;
}

/* ====================================================================== */
/* SlotModel — 8-bucket previous-slot context                             */
/* ====================================================================== */

void slot_model_init(SlotModel *m) {
    for (int i = 0; i < SLOT_CTX_BUCKETS; i++)
        bittree_init(&m->slot_trees[i], 5);
    for (int i = 0; i < 32; i++) { m->hi_probs[i][0] = 1; m->hi_probs[i][1] = 1; }
    m->prev_slot_bucket = 0;
}

static int bit_length_u32(uint32_t v) {
    if (v == 0) return 0;
    int n = 0; while (v) { n++; v >>= 1; } return n;
}

void slot_model_encode(SlotModel *m, ArithEncoder *enc, uint32_t value) {
    if (value < 1) value = 1;
    int slot = bit_length_u32(value) - 1;
    if (slot > 31) slot = 31;

    bittree_encode(&m->slot_trees[m->prev_slot_bucket], enc, (uint8_t)slot);

    if (slot > 0) {
        uint32_t extra = value - (1u << slot);
        if (slot >= 2) {
            int hi_bit = (extra >> (slot - 1)) & 1;
            uint32_t c0 = m->hi_probs[slot][0], t = c0 + m->hi_probs[slot][1];
            uint32_t p0 = (c0 * CM_SCALE) / t;
            if (p0 < 1) p0 = 1; if (p0 > CM_SCALE - 1) p0 = CM_SCALE - 1;
            arith_enc_encode(enc, hi_bit, p0, CM_SCALE);
            m->hi_probs[slot][hi_bit]++;
            if (m->hi_probs[slot][0] + m->hi_probs[slot][1] > CM_RESCALE_LIMIT) {
                m->hi_probs[slot][0] = m->hi_probs[slot][0] > 1 ? m->hi_probs[slot][0] >> 1 : 1;
                m->hi_probs[slot][1] = m->hi_probs[slot][1] > 1 ? m->hi_probs[slot][1] >> 1 : 1;
            }
            for (int bp = slot - 2; bp >= 0; bp--)
                arith_enc_encode(enc, (extra >> bp) & 1, 1, 2);
        } else {
            arith_enc_encode(enc, extra & 1, 1, 2);
        }
    }

    /* Update prev_slot context — 8 buckets */
    int bucket = slot / 4;
    if (bucket >= SLOT_CTX_BUCKETS) bucket = SLOT_CTX_BUCKETS - 1;
    m->prev_slot_bucket = bucket;
}

uint32_t slot_model_decode(SlotModel *m, ArithDecoder *dec) {
    int slot = (int)bittree_decode(&m->slot_trees[m->prev_slot_bucket], dec);
    if (slot == 0) { m->prev_slot_bucket = 0; return 1; }

    uint32_t extra = 0;
    if (slot >= 2) {
        uint32_t c0 = m->hi_probs[slot][0], t = c0 + m->hi_probs[slot][1];
        uint32_t p0 = (c0 * CM_SCALE) / t;
        if (p0 < 1) p0 = 1; if (p0 > CM_SCALE - 1) p0 = CM_SCALE - 1;
        int hi = arith_dec_decode(dec, p0, CM_SCALE);
        m->hi_probs[slot][hi]++;
        if (m->hi_probs[slot][0] + m->hi_probs[slot][1] > CM_RESCALE_LIMIT) {
            m->hi_probs[slot][0] = m->hi_probs[slot][0] > 1 ? m->hi_probs[slot][0] >> 1 : 1;
            m->hi_probs[slot][1] = m->hi_probs[slot][1] > 1 ? m->hi_probs[slot][1] >> 1 : 1;
        }
        extra = (uint32_t)hi;
        for (int bp = slot - 2; bp >= 0; bp--)
            extra = (extra << 1) | (uint32_t)arith_dec_decode(dec, 1, 2);
    } else {
        extra = (uint32_t)arith_dec_decode(dec, 1, 2);
    }

    int bucket = slot / 4;
    if (bucket >= SLOT_CTX_BUCKETS) bucket = SLOT_CTX_BUCKETS - 1;
    m->prev_slot_bucket = bucket;

    return (1u << slot) + extra;
}
