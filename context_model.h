#ifndef CONTEXT_MODEL_H
#define CONTEXT_MODEL_H

#include <stdint.h>
#include "range_coder.h"

#define CM_SCALE         4096
#define CM_RESCALE_LIMIT 512

/* ======================================================================
 * BitTreeModel — adaptive binary tree for byte encoding
 * ====================================================================== */

typedef struct {
    int      bits;
    uint16_t probs[256][2];
} BitTreeModel;

void    bittree_init(BitTreeModel *m, int bits);
void    bittree_encode(BitTreeModel *m, ArithEncoder *enc, uint8_t value);
uint8_t bittree_decode(BitTreeModel *m, ArithDecoder *dec);
uint32_t bittree_get_p0(const BitTreeModel *m, int idx);
void     bittree_update_node(BitTreeModel *m, int idx, int bit);

/* ======================================================================
 * InstructionModel — order-3 binary context (8 states)
 * ====================================================================== */

typedef struct {
    uint16_t probs[8][2];   /* order-3: last 3 instruction types */
    int      ctx;
} InstructionModel;

void inst_model_init(InstructionModel *m);
void inst_model_encode(InstructionModel *m, ArithEncoder *enc, int inst);
int  inst_model_decode(InstructionModel *m, ArithDecoder *dec);

/* ======================================================================
 * RepTypeModel — 4-state cascade (rep0/rep1/rep2/new_offset)
 *
 * Encoding tree:
 *   is_new? → 0=rep, 1=new_offset
 *   if rep: is_rep0? → 0=rep0, 1=rep1_or_rep2
 *   if not rep0: is_rep2? → 0=rep1, 1=rep2
 * ====================================================================== */

typedef struct {
    uint16_t is_new_probs[4][2];      /* is new offset? (4-ctx history) */
    uint16_t is_rep0_probs[2][2];     /* is rep0? (2-ctx) */
    uint16_t is_rep2_probs[2][2];     /* is rep2 vs rep1? (2-ctx) */
    int      is_new_ctx;
    int      is_rep0_ctx;
    int      is_rep2_ctx;
} RepTypeModel;

void rep_type_init(RepTypeModel *m);
void rep_type_encode(RepTypeModel *m, ArithEncoder *enc, int type);
int  rep_type_decode(RepTypeModel *m, ArithDecoder *dec);

/* ======================================================================
 * MixedLiteralModel — 5-model fast mixer with sequence context
 *
 * Sub-models:
 *   1. Order-1 full byte (256 ctx)
 *   2. Order-0 per match_len_bucket (4 ctx)
 *   3. Character class (4 ctx)
 *   4. Order-2 hashed (O2_NUM_CONTEXTS ctx)
 *   5. Word model (27 ctx)
 *
 * Sequence-aware: match_len_bucket selects o0 tree context
 * Fast fixed-weight mixing (no sigmoid/stretch/squash overhead)
 * ====================================================================== */

#define O2_NUM_CONTEXTS   4096
#define O3_NUM_CONTEXTS   8192
#define WORD_NUM_CONTEXTS 27

typedef struct {
    BitTreeModel *o1_trees;             /* 256 trees */
    BitTreeModel o0_trees[4];           /* 4 trees (by match_len_bucket) */
    BitTreeModel cc_trees[4];           /* 4 trees */
    BitTreeModel *o2_trees;             /* O2_NUM_CONTEXTS trees */
    BitTreeModel *o3_trees;             /* O3_NUM_CONTEXTS trees (order-3 hashed) */
    BitTreeModel *word_trees;           /* WORD_NUM_CONTEXTS trees */

    uint8_t last_byte;
    uint8_t prev_prev_byte;
    uint8_t prev3_byte;                 /* 3rd previous byte for order-3 */
    uint8_t last_class;
    int     match_len_bucket;           /* 0=lit_run, 1=short, 2=med, 3=long */
} MixedLiteralModel;

void    mixed_lit_init(MixedLiteralModel *m);
void    mixed_lit_encode(MixedLiteralModel *m, ArithEncoder *enc, uint8_t byte_val);
uint8_t mixed_lit_decode(MixedLiteralModel *m, ArithDecoder *dec);
void    mixed_lit_set_match_ctx(MixedLiteralModel *m, int bucket);
void    mixed_lit_free(MixedLiteralModel *m);

/* Match length → bucket mapping */
static inline int match_len_to_bucket(int match_len) {
    if (match_len <= 0) return 0;   /* no match (literal run) */
    if (match_len <= 7) return 1;   /* short match */
    if (match_len <= 31) return 2;  /* medium match */
    return 3;                        /* long match */
}

/* ======================================================================
 * SlotModel — with previous-slot context (8 buckets)
 * ====================================================================== */

#define SLOT_CTX_BUCKETS 8

typedef struct {
    BitTreeModel slot_trees[SLOT_CTX_BUCKETS]; /* selected by prev_slot bucket */
    uint16_t     hi_probs[32][2];
    int          prev_slot_bucket;
} SlotModel;

void     slot_model_init(SlotModel *m);
void     slot_model_encode(SlotModel *m, ArithEncoder *enc, uint32_t value);
uint32_t slot_model_decode(SlotModel *m, ArithDecoder *dec);

/* ======================================================================
 * LiteralModel — Legacy (kept for compatibility)
 * ====================================================================== */

typedef struct {
    BitTreeModel trees[16];
    int          last_msb;
} LiteralModel;

void    lit_model_init(LiteralModel *m);
void    lit_model_encode(LiteralModel *m, ArithEncoder *enc, uint8_t byte_val);
uint8_t lit_model_decode(LiteralModel *m, ArithDecoder *dec);

#endif /* CONTEXT_MODEL_H */
