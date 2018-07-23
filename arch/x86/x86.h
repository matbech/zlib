/* x86.h -- check for x86 CPU features
* Copyright (C) 2013 Intel Corporation Jim Kukunas
* For conditions of distribution and use, see copyright notice in zlib.h
*/

#ifndef X86_H
#define X86_H

#include "zutil.h" // for ZLIB_INTERNAL

typedef struct internal_state deflate_state;

extern int x86_cpu_has_sse2;
extern int x86_cpu_has_sse42;
extern int x86_cpu_has_pclmulqdq;

void x86_check_features(void);

void ZLIB_INTERNAL fill_window_sse(deflate_state* s);

/* Functions that are SIMD optimised on x86 */
void ZLIB_INTERNAL crc_fold_init(unsigned* z_const s);
void ZLIB_INTERNAL crc_fold_copy(unsigned* z_const s,
    unsigned char* dst,
    z_const unsigned char* src,
    long len);
void ZLIB_INTERNAL crc_fold(unsigned* z_const s,
    z_const unsigned char* src,
    size_t len);
unsigned ZLIB_INTERNAL crc_fold_512to32(unsigned* z_const s);

#endif  /* X86_H */
