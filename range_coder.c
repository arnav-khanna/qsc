#include "range_coder.h"

/* ======================================================================
 * Context Model
 * ====================================================================== */

void ctx_model_init(ContextModel *m, int ctx_bits) {
    m->ctx_bits = ctx_bits;
    m->context  = 0;
    m->mask     = (1 << ctx_bits) - 1;
    int n = 1 << ctx_bits;
    for (int i = 0; i < n; i++) {
        m->counts[i][0] = 1;
        m->counts[i][1] = 1;
    }
}

void ctx_model_get_prob(const ContextModel *m, uint32_t *c0, uint32_t *c1, uint32_t *total) {
    *c0    = m->counts[m->context][0];
    *c1    = m->counts[m->context][1];
    *total = *c0 + *c1;
}

void ctx_model_update(ContextModel *m, int bit) {
    m->counts[m->context][bit] += 1;
    m->context = ((m->context << 1) | bit) & m->mask;
}

/* ======================================================================
 * Arithmetic Encoder
 * ====================================================================== */

void arith_enc_init(ArithEncoder *e) {
    e->low          = 0;
    e->high         = 0xFFFFFFFF;
    e->pending_bits = 0;
    e->buffer       = 0;
    e->count        = 0;
    ByteBuffer_init(&e->out);
}

static void enc_write_bit(ArithEncoder *e, int bit) {
    e->buffer = (e->buffer << 1) | (bit & 1);
    e->count++;
    if (e->count == 8) {
        ByteBuffer_push(&e->out, e->buffer);
        e->buffer = 0;
        e->count  = 0;
    }
}

void arith_enc_encode(ArithEncoder *e, int bit, uint32_t p0, uint32_t total) {
    uint64_t range = (uint64_t)e->high - (uint64_t)e->low + 1;
    /* split = boundary between 0-interval and 1-interval.
     * 0-interval: [low, split-1], 1-interval: [split, high]
     * Clamp to ensure both intervals are non-empty. */
    uint32_t split = e->low + (uint32_t)((range * p0) / total);
    if (split <= e->low)  split = e->low + 1;
    if (split > e->high)  split = e->high;

    if (bit == 0) {
        e->high = split - 1;
    } else {
        e->low = split;
    }

    for (;;) {
        if ((e->high & 0x80000000) == (e->low & 0x80000000)) {
            int b = (e->high & 0x80000000) ? 1 : 0;
            enc_write_bit(e, b);
            for (int i = 0; i < e->pending_bits; i++) {
                enc_write_bit(e, 1 ^ b);
            }
            e->pending_bits = 0;

            e->low  = (e->low << 1) & 0xFFFFFFFF;
            e->high = ((e->high << 1) | 1) & 0xFFFFFFFF;
        } else if ((e->low & 0x40000000) && !(e->high & 0x40000000)) {
            e->pending_bits++;
            e->low  = (e->low << 1) & 0x7FFFFFFF;
            e->high = ((e->high << 1) | 1) | 0x80000000;
            e->high &= 0xFFFFFFFF;
        } else {
            break;
        }
    }
}

static void enc_encode_raw_byte(ArithEncoder *e, uint8_t byte_val) {
    for (int i = 7; i >= 0; i--) {
        arith_enc_encode(e, (byte_val >> i) & 1, 1, 2);
    }
}

void arith_enc_encode_varint(ArithEncoder *e, uint32_t value) {
    while (value >= 0x80) {
        enc_encode_raw_byte(e, (uint8_t)((value & 0x7F) | 0x80));
        value >>= 7;
    }
    enc_encode_raw_byte(e, (uint8_t)(value & 0x7F));
}

void arith_enc_flush(ArithEncoder *e) {
    e->pending_bits++;
    int b = (e->low & 0x40000000) ? 1 : 0;
    enc_write_bit(e, b);
    for (int i = 0; i < e->pending_bits; i++) {
        enc_write_bit(e, 1 ^ b);
    }

    if (e->count > 0) {
        e->buffer <<= (8 - e->count);
        ByteBuffer_push(&e->out, e->buffer);
    }
}

uint8_t *arith_enc_get_bytes(ArithEncoder *e, size_t *out_len) {
    *out_len = e->out.len;
    return e->out.data;
}

void arith_enc_free(ArithEncoder *e) {
    ByteBuffer_free(&e->out);
}

/* ======================================================================
 * Arithmetic Decoder
 * ====================================================================== */

static int dec_read_bit(ArithDecoder *d) {
    if (d->byte_idx < d->data_len) {
        int b = (d->data[d->byte_idx] >> (7 - d->bit_idx)) & 1;
        d->bit_idx++;
        if (d->bit_idx == 8) {
            d->bit_idx = 0;
            d->byte_idx++;
        }
        return b;
    }
    return 0;
}

void arith_dec_init(ArithDecoder *d, const uint8_t *data, size_t len) {
    d->data     = data;
    d->data_len = len;
    d->byte_idx = 0;
    d->bit_idx  = 0;
    d->low      = 0;
    d->high     = 0xFFFFFFFF;
    d->value    = 0;

    for (int i = 0; i < 32; i++) {
        d->value = (d->value << 1) | (uint32_t)dec_read_bit(d);
    }
}

int arith_dec_decode(ArithDecoder *d, uint32_t p0, uint32_t total) {
    /* Implicit shortcuts */
    if (p0 == 0)     return 1;
    if (p0 == total) return 0;

    uint64_t range = (uint64_t)d->high - (uint64_t)d->low + 1;
    /* Compute split with same clamping as encoder */
    uint32_t split = d->low + (uint32_t)((range * p0) / total);
    if (split <= d->low)  split = d->low + 1;
    if (split > d->high)  split = d->high;

    int bit;
    if (d->value <= split - 1) {
        bit = 0;
        d->high = split - 1;
    } else {
        bit = 1;
        d->low = split;
    }

    for (;;) {
        if ((d->high & 0x80000000) == (d->low & 0x80000000)) {
            d->low   = (d->low << 1) & 0xFFFFFFFF;
            d->high  = ((d->high << 1) | 1) & 0xFFFFFFFF;
            d->value = ((d->value << 1) & 0xFFFFFFFF) | (uint32_t)dec_read_bit(d);
        } else if ((d->low & 0x40000000) && !(d->high & 0x40000000)) {
            d->low   = (d->low << 1) & 0x7FFFFFFF;
            d->high  = ((d->high << 1) | 1) | 0x80000000;
            d->high &= 0xFFFFFFFF;
            d->value = (d->value & 0x80000000) |
                       ((d->value << 1) & 0x7FFFFFFF) |
                       (uint32_t)dec_read_bit(d);
        } else {
            break;
        }
    }

    return bit;
}

uint32_t arith_dec_decode_varint(ArithDecoder *d) {
    uint32_t value = 0;
    int shift = 0;
    for (;;) {
        uint8_t byte_val = 0;
        for (int i = 0; i < 8; i++) {
            byte_val = (byte_val << 1) | (uint8_t)arith_dec_decode(d, 1, 2);
        }
        value |= (uint32_t)(byte_val & 0x7F) << shift;
        if (!(byte_val & 0x80)) break;
        shift += 7;
    }
    return value;
}
