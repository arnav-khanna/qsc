#ifndef QSC3_H
#define QSC3_H

#include <stdint.h>
#include <stddef.h>

#define QSC_MAGIC      "QSC3"
#define QSC_VERSION    10        /* v10: ratio-first transform selection with arithmetic payloads */
#define QSC_CHUNK_SIZE 8388608   /* 8 MB */
#define QSC_CARRY_OVER 524288   /* 512 KB */
#define QSC_NUM_THREADS 0       /* 0 = auto-detect */

/* Compress/decompress a single chunk (binary blob) */
uint8_t *qsc_compress_chunk(const uint8_t *chunk, size_t chunk_len,
                            const uint8_t *prev_tail, size_t prev_tail_len,
                            size_t *out_len);

uint8_t *qsc_decompress_chunk(const uint8_t *compressed, size_t comp_len,
                              const uint8_t *prev_tail, size_t prev_tail_len,
                              size_t *out_len);

/* Pack / Unpack archive */
int  qsc_pack(const char *input_path, const char *output_file);
int  qsc_unpack(const char *archive_path, const char *output_folder);

/* Benchmark */
void qsc_benchmark(const char *input_path);

#endif /* QSC3_H */
