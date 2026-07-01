#ifndef LZ_ENGINE_H
#define LZ_ENGINE_H

#include <stdint.h>
#include <stddef.h>
#include "dynarray.h"

#define LZ_MIN_MATCH         4
#define LZ_MAX_MATCH         65535
#define LZ_WINDOW_SIZE       8388608  /* 8 MB */
#define LZ_MAX_SEARCH_DEPTH  256

/* Result of LZ compression — with REP0/1/2 match support
 * rep_types: 0=rep0, 1=rep1, 2=rep2, 3=new_offset */
typedef struct {
    IntArray instructions;   /* 0=literal, 1=match */
    IntArray literals;       /* byte values for literals */
    IntArray rep_types;      /* per match: 0=rep0, 1=rep1, 2=rep2, 3=new_offset */
    IntArray new_offsets;    /* only offsets for rep_type==3 */
    IntArray lengths;        /* all match lengths */
} LZResult;

/* Lazy scoring LZ compression with REP0/1/2 + dual-hash matching */
void lz_compress(const uint8_t *data, size_t data_len,
                 const uint8_t *prev_tail, size_t prev_tail_len,
                 LZResult *result);

/* Decompress with REP0/1/2 match support */
void lz_decompress(const int32_t *instructions, size_t num_instr,
                   const int32_t *literals,
                   const int32_t *rep_types,
                   const int32_t *new_offsets,
                   const int32_t *lengths,
                   const uint8_t *prev_tail, size_t prev_tail_len,
                   uint8_t **out, size_t *out_len);

void lz_result_free(LZResult *r);

#endif /* LZ_ENGINE_H */
