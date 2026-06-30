#ifndef RANGE_CODER_H
#define RANGE_CODER_H

#include <stdint.h>
#include <stddef.h>
#include "dynarray.h"

/* ======================================================================
 * Context Model (adaptive binary)
 * ====================================================================== */

typedef struct {
    int      ctx_bits;
    uint32_t counts[256][2];  /* max 8 context bits → 256 states */
    int      context;
    int      mask;
} ContextModel;

void ctx_model_init(ContextModel *m, int ctx_bits);
void ctx_model_get_prob(const ContextModel *m, uint32_t *c0, uint32_t *c1, uint32_t *total);
void ctx_model_update(ContextModel *m, int bit);

/* ======================================================================
 * Arithmetic Encoder
 * ====================================================================== */

typedef struct {
    uint32_t   low;
    uint32_t   high;
    int        pending_bits;
    ByteBuffer out;
    uint8_t    buffer;
    int        count;
} ArithEncoder;

void arith_enc_init(ArithEncoder *e);
void arith_enc_encode(ArithEncoder *e, int bit, uint32_t p0, uint32_t total);
void arith_enc_encode_varint(ArithEncoder *e, uint32_t value);
void arith_enc_flush(ArithEncoder *e);
uint8_t *arith_enc_get_bytes(ArithEncoder *e, size_t *out_len);
void arith_enc_free(ArithEncoder *e);

/* ======================================================================
 * Arithmetic Decoder
 * ====================================================================== */

typedef struct {
    const uint8_t *data;
    size_t         data_len;
    size_t         byte_idx;
    int            bit_idx;
    uint32_t       low;
    uint32_t       high;
    uint32_t       value;
} ArithDecoder;

void     arith_dec_init(ArithDecoder *d, const uint8_t *data, size_t len);
int      arith_dec_decode(ArithDecoder *d, uint32_t p0, uint32_t total);
uint32_t arith_dec_decode_varint(ArithDecoder *d);

#endif /* RANGE_CODER_H */
