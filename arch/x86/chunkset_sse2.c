/* chunkset_sse2.c -- SSE2 inline functions to copy small data chunks.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "../../zconf.h"
#include "../../zutil.h"

#include <immintrin.h>
#include <stdint.h>

typedef __m128i chunk_t;

#define CHUNK_SIZE 16

#define HAVE_CHUNKMEMSET_1
#define HAVE_CHUNKMEMSET_2
#define HAVE_CHUNKMEMSET_4
#define HAVE_CHUNKMEMSET_8

static INLINE void chunkmemset_2(uint8_t *from, chunk_t *chunk) {
    int16_t tmp;
    zmemcpy_2(&tmp, from);
    *chunk = _mm_set1_epi16(tmp);
}

static INLINE void chunkmemset_4(uint8_t *from, chunk_t *chunk) {
    int32_t tmp;
    zmemcpy_4(&tmp, from);
    *chunk = _mm_set1_epi32(tmp);
}

static INLINE void chunkmemset_8(uint8_t *from, chunk_t *chunk) {
    int64_t tmp;
    zmemcpy_8(&tmp, from);
    *chunk = _mm_set1_epi64x(tmp);
}

static INLINE void loadchunk(uint8_t const *s, chunk_t *chunk) {
    *chunk = _mm_loadu_si128((__m128i *)s);
}

static INLINE void storechunk(uint8_t *out, chunk_t *chunk) {
    _mm_storeu_si128((__m128i *)out, *chunk);
}

#define CHUNKSIZE        chunksize_sse2
#define CHUNKCOPY        chunkcopy_sse2
#define CHUNKCOPY_SAFE   chunkcopy_safe_sse2
#define CHUNKUNROLL      chunkunroll_sse2
#define CHUNKMEMSET      chunkmemset_sse2
#define CHUNKMEMSET_SAFE chunkmemset_safe_sse2

#include "../../chunkset_tpl.h"
