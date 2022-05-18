/* x86.h -- check for x86 CPU features
* Copyright (C) 2013 Intel Corporation Jim Kukunas
* For conditions of distribution and use, see copyright notice in zlib.h
*/

#ifndef X86_H
#define X86_H

#define USE_PCLMUL_CRC

#include "../../zutil.h" // for ZLIB_INTERNAL

#include <stdint.h>

typedef struct internal_state deflate_state;

extern int x86_cpu_has_sse42;
extern int x86_cpu_has_pclmul;
extern int x86_cpu_has_avx2;
extern int x86_cpu_has_avx512;
extern int x86_cpu_has_vpclmulqdq;

void x86_check_features(void);

void ZLIB_INTERNAL slide_hash_sse2(deflate_state* s);
void ZLIB_INTERNAL slide_hash_avx2(deflate_state* s);
uint32_t ZLIB_INTERNAL adler32_ssse3(uint32_t adler, const unsigned char* buf, size_t len);
uint32_t ZLIB_INTERNAL adler32_avx2(uint32_t adler, const unsigned char* buf, size_t len);

/* Functions that are SIMD optimised on x86 */
void ZLIB_INTERNAL crc_fold_init(unsigned* z_const s);
void ZLIB_INTERNAL crc_fold_copy(unsigned* z_const s,
    unsigned char* dst,
    z_const unsigned char* src,
    size_t len);
void ZLIB_INTERNAL crc_fold(unsigned* z_const s,
    z_const unsigned char* src,
    size_t len);
unsigned ZLIB_INTERNAL crc_fold_512to32(unsigned* z_const s);

/*
 * crc32_sse42_simd_(): compute the crc32 of the buffer, where the buffer
 * length must be at least 64, and a multiple of 16.
 */
uint32_t ZLIB_INTERNAL crc32_sse42_simd_(
    const unsigned char* buf,
    z_size_t len,
    uint32_t crc);

/* memory chunking */
extern uint32_t chunksize_sse2(void);
extern uint8_t* chunkcopy_sse2(uint8_t* out, uint8_t const* from, unsigned len);
extern uint8_t* chunkcopy_safe_sse2(uint8_t* out, uint8_t const* from, unsigned len, uint8_t* safe);
extern uint8_t* chunkunroll_sse2(uint8_t* out, unsigned* dist, unsigned* len);
extern uint8_t* chunkmemset_sse2(uint8_t* out, unsigned dist, unsigned len);
extern uint8_t* chunkmemset_safe_sse2(uint8_t* out, unsigned dist, unsigned len, unsigned left);

extern uint32_t chunksize_avx(void);
extern uint8_t* chunkcopy_avx(uint8_t* out, uint8_t const* from, unsigned len);
extern uint8_t* chunkcopy_safe_avx(uint8_t* out, uint8_t const* from, unsigned len, uint8_t* safe);
extern uint8_t* chunkunroll_avx(uint8_t* out, unsigned* dist, unsigned* len);
extern uint8_t* chunkmemset_avx(uint8_t* out, unsigned dist, unsigned len);
extern uint8_t* chunkmemset_safe_avx(uint8_t* out, unsigned dist, unsigned len, unsigned left);

ZLIB_INTERNAL uint32_t compare256_avx2(const uint8_t* src0, const uint8_t* src1);
ZLIB_INTERNAL uint32_t compare256_sse2(const uint8_t* src0, const uint8_t* src1);

#endif  /* X86_H */
