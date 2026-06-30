#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define QSC_UNUSED __attribute__((unused))
#else
#define QSC_UNUSED
#endif

/*
 * Generic dynamic array (like a minimal std::vector).
 *
 * Usage:
 *   DYNARRAY_DEFINE(int32_t, IntArray)
 *
 * This generates:
 *   typedef struct { int32_t *data; size_t len, cap; } IntArray;
 *   void IntArray_init(IntArray *a);
 *   void IntArray_push(IntArray *a, int32_t val);
 *   void IntArray_free(IntArray *a);
 *   void IntArray_reserve(IntArray *a, size_t cap);
 *   void IntArray_clear(IntArray *a);
 */

#define DYNARRAY_DEFINE(T, Name)                                              \
    typedef struct {                                                           \
        T      *data;                                                          \
        size_t  len;                                                           \
        size_t  cap;                                                           \
    } Name;                                                                    \
                                                                               \
    static inline QSC_UNUSED void Name##_init(Name *a) {                       \
        a->data = NULL;                                                        \
        a->len  = 0;                                                           \
        a->cap  = 0;                                                           \
    }                                                                          \
                                                                               \
    static inline QSC_UNUSED void Name##_reserve(Name *a, size_t new_cap) {    \
        if (new_cap > a->cap) {                                                \
            a->data = (T *)realloc(a->data, new_cap * sizeof(T));              \
            a->cap  = new_cap;                                                 \
        }                                                                      \
    }                                                                          \
                                                                               \
    static inline QSC_UNUSED void Name##_push(Name *a, T val) {                \
        if (a->len >= a->cap) {                                                \
            size_t nc = a->cap == 0 ? 64 : a->cap * 2;                        \
            Name##_reserve(a, nc);                                             \
        }                                                                      \
        a->data[a->len++] = val;                                               \
    }                                                                          \
                                                                               \
    static inline QSC_UNUSED void Name##_clear(Name *a) {                      \
        a->len = 0;                                                            \
    }                                                                          \
                                                                               \
    static inline QSC_UNUSED void Name##_free(Name *a) {                       \
        free(a->data);                                                         \
        a->data = NULL;                                                        \
        a->len  = 0;                                                           \
        a->cap  = 0;                                                           \
    }

/* Byte buffer (used by encoder output) */
DYNARRAY_DEFINE(uint8_t, ByteBuffer)

/* Int arrays (used for instructions, literals, offsets, lengths) */
DYNARRAY_DEFINE(int32_t, IntArray)

#endif /* DYNARRAY_H */
