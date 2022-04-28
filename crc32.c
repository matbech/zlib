/* crc32.c -- compute the CRC-32 of a data stream
 * Copyright (C) 1995-2022 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * Thanks to Rodney Brown <rbrown64@csc.com.au> for his contribution of faster
 * CRC methods: exclusive-oring 32 bits of data at a time, and pre-computing
 * tables for updating the shift register in one step with three exclusive-ors
 * instead of four steps with four exclusive-ors.  This results in about a
 * factor of two increase in speed on a Power PC G4 (PPC7455) using gcc -O3.
 */

/* @(#) $Id$ */

/*
  Note on the use of DYNAMIC_CRC_TABLE: there is no mutex or semaphore
  protection on the static variables used to control the first-use generation
  of the crc tables.  Therefore, if you #define DYNAMIC_CRC_TABLE, you should
  first call get_crc_table() to initialize the tables before allowing more than
  one thread to use crc32().

  DYNAMIC_CRC_TABLE and MAKECRCH can be #defined to write out crc32.h.
 */

#ifdef __MINGW32__
# include <sys/param.h>
#elif _WIN32
# define LITTLE_ENDIAN 1234
# define BIG_ENDIAN 4321
# if defined(_M_IX86) || defined(_M_AMD64) || defined(_M_IA64) || defined(_M_ARM) || defined(_M_ARM64)
#  define BYTE_ORDER LITTLE_ENDIAN
# else
#  error Unknown endianness!
# endif
#elif __APPLE__
# include <machine/endian.h>
#else
# include <endian.h>
#endif

#ifdef MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_CRC_TABLE
#    define DYNAMIC_CRC_TABLE
#  endif /* !DYNAMIC_CRC_TABLE */
#endif /* MAKECRCH */

#include "deflate.h"
#if defined(_M_IX86) || defined(_M_AMD64)
#include "arch\x86\x86.h"
#elif defined(_M_ARM64)
#include "arch\aarch64\aarch64.h"
#endif
#include "zutil.h"      /* for STDC and FAR definitions */

#define local static

/* Definitions for doing the crc four data bytes at a time. */
#if !defined(NOBYFOUR) && defined(Z_U4)
#  define BYFOUR
#endif
#ifdef BYFOUR
#if BYTE_ORDER == LITTLE_ENDIAN
local unsigned long crc32_little OF((unsigned long,
    const unsigned char FAR *, z_size_t));
#elif BYTE_ORDER == BIG_ENDIAN
local unsigned long crc32_big OF((unsigned long,
    const unsigned char FAR *, z_size_t));
#endif
#  define TBLS 8
#else
#  define TBLS 1
#endif /* BYFOUR */

/* CRC polynomial. */
#define POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

#ifdef DYNAMIC_CRC_TABLE

local volatile int crc_table_empty = 1;
local z_crc_t FAR crc_table[TBLS][256];
local z_crc_t FAR x2n_table[32];
local void make_crc_table OF((void));
#ifdef MAKECRCH
   local void write_table OF((FILE *, const z_crc_t FAR *));
#endif /* MAKECRCH */
/*
  Generate tables for a byte-wise 32-bit CRC calculation on the polynomial:
  x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.

  Polynomials over GF(2) are represented in binary, one bit per coefficient,
  with the lowest powers in the most significant bit.  Then adding polynomials
  is just exclusive-or, and multiplying a polynomial by x is a right shift by
  one.  If we call the above polynomial p, and represent a byte as the
  polynomial q, also with the lowest power in the most significant bit (so the
  byte 0xb1 is the polynomial x^7+x^3+x+1), then the CRC is (q*x^32) mod p,
  where a mod b means the remainder after dividing a by b.

  This calculation is done using the shift-register method of multiplying and
  taking the remainder.  The register is initialized to zero, and for each
  incoming bit, x^32 is added mod p to the register if the bit is a one (where
  x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by
  x (which is shifting right by one and adding x^32 mod p if the bit shifted
  out is a one).  We start with the highest power (least significant bit) of
  q and repeat for all eight bits of q.

  The first table is simply the CRC of all possible eight bit values.  This is
  all the information needed to generate CRCs on data a byte at a time for all
  combinations of CRC register values and incoming bytes.  The remaining tables
  allow for word-at-a-time CRC calculation for both big-endian and little-
  endian machines, where a word is four bytes.
*/
local void make_crc_table()
{
    z_crc_t c;
    int n, k;
    z_crc_t poly;                       /* polynomial exclusive-or pattern */
    /* terms of polynomial defining this crc (except x^32): */
    static volatile int first = 1;      /* flag to limit concurrent making */
    static const unsigned char p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

    /* See if another task is already doing this (not thread-safe, but better
       than nothing -- significantly reduces duration of vulnerability in
       case the advice about DYNAMIC_CRC_TABLE is ignored) */
    if (first) {
        first = 0;

        /* make exclusive-or pattern from polynomial (0xedb88320UL) */
        poly = 0;
        for (n = 0; n < (int)(sizeof(p)/sizeof(unsigned char)); n++)
            poly |= (z_crc_t)1 << (31 - p[n]);

        /* generate a crc for every 8-bit value */
        for (n = 0; n < 256; n++) {
            c = (z_crc_t)n;
            for (k = 0; k < 8; k++)
                c = c & 1 ? poly ^ (c >> 1) : c >> 1;
            crc_table[0][n] = c;
        }

#ifdef BYFOUR
        /* generate crc for each value followed by one, two, and three zeros,
           and then the byte reversal of those as well as the first table */
        for (n = 0; n < 256; n++) {
            c = crc_table[0][n];
            crc_table[4][n] = ZSWAP32(c);
            for (k = 1; k < 4; k++) {
                c = crc_table[0][c & 0xff] ^ (c >> 8);
                crc_table[k][n] = c;
                crc_table[k + 4][n] = ZSWAP32(c);
            }
        }
#endif /* BYFOUR */

        crc_table_empty = 0;
    }
    else {      /* not first */
        /* wait for the other guy to finish (not efficient, but rare) */
        while (crc_table_empty)
            ;
    }

#ifdef MAKECRCH
    /* write out CRC tables to crc32.h */
    {
        FILE *out;

        out = fopen("crc32.h", "w");
        if (out == NULL) return;
        fprintf(out, "/* crc32.h -- tables for rapid CRC calculation\n");
        fprintf(out, " * Generated automatically by crc32.c\n */\n\n");
        fprintf(out, "local const z_crc_t FAR ");
        fprintf(out, "crc_table[TBLS][256] =\n{\n  {\n");
        write_table(out, crc_table[0]);
#  ifdef BYFOUR
        fprintf(out, "#ifdef BYFOUR\n");
        for (k = 1; k < 8; k++) {
            fprintf(out, "  },\n  {\n");
            write_table(out, crc_table[k]);
        }
        fprintf(out, "#endif\n");
#  endif /* BYFOUR */
        fprintf(out, "  }\n};\n");
        fclose(out);
    }
#endif /* MAKECRCH */
}

#ifdef MAKECRCH
local void write_table(out, table)
    FILE *out;
    const z_crc_t FAR *table;
{
    int n;

    for (n = 0; n < 256; n++)
        fprintf(out, "%s0x%08lxUL%s", n % 5 ? "" : "    ",
                (unsigned long)(table[n]),
                n == 255 ? "\n" : (n % 5 == 4 ? ",\n" : ", "));
}
#endif /* MAKECRCH */

#else /* !DYNAMIC_CRC_TABLE */
/* ========================================================================
 * Tables of CRC-32s of all single-byte values, made by make_crc_table().
 */
#include "crc32.h"
#endif /* DYNAMIC_CRC_TABLE */

/* ========================================================================
 * Routines used for CRC calculation. Some are also required for the table
 * generation above.
 */

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
local z_crc_t multmodp(a, b)
    z_crc_t a;
    z_crc_t b;
{
    z_crc_t m, p;

    m = (z_crc_t)1 << 31;
    p = 0;
    for (;;) {
        if (a & m) {
            p ^= b;
            if ((a & (m - 1)) == 0)
                break;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    return p;
}

/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized.
 */
local z_crc_t x2nmodp(n, k)
    z_off64_t n;
    unsigned k;
{
    z_crc_t p;

    p = (z_crc_t)1 << 31;           /* x^0 == 1 */
    while (n) {
        if (n & 1)
            p = multmodp(x2n_table[k & 31], p);
        n >>= 1;
        k++;
    }
    return p;
}

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
const z_crc_t FAR * ZEXPORT get_crc_table()
{
#ifdef DYNAMIC_CRC_TABLE
    once(&made, make_crc_table);
#endif /* DYNAMIC_CRC_TABLE */
    return (const z_crc_t FAR *)crc_table;
}

/* ========================================================================= */
#define DO1 crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8)
#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1
#define DO4 DO1; DO1; DO1; DO1

/* ========================================================================= */
unsigned long ZEXPORT crc32_z(crc, buf, len)
    unsigned long crc;
    const unsigned char FAR *buf;
    z_size_t len;
{
#if defined(_M_ARM64)
    if (buf == Z_NULL) {
        return 0UL;
    }
    return crc32_acle(crc, buf, len);
#elif defined(_M_IX86) || defined(_M_AMD64)
    /*
     * zlib convention is to call crc32(0, NULL, 0); before making
     * calls to crc32(). So this is a good, early (and infrequent)
     * place to cache CPU features if needed for those later, more
     * interesting crc32() calls.
     */
#if defined(USE_PCLMUL_CRC)
     /*
      * crc32_sse42_simd_ buffer size constraints: see the use in zlib/crc32.c
      * for computing the crc32 of an arbitrary length buffer.
      */
#define Z_CRC32_SSE42_MINIMUM_LENGTH 64
#define Z_CRC32_SSE42_CHUNKSIZE_MASK 15

     /*
      * Use x86 sse4.2+pclmul SIMD to compute the crc32. Since this
      * routine can be freely used, check CPU features here.
      */
    if (buf == Z_NULL) {
        if (!len) /* Assume user is calling crc32(0, NULL, 0); */
            x86_check_features();
        return 0UL;
    }

    if (x86_cpu_has_pclmul && len >= Z_CRC32_SSE42_MINIMUM_LENGTH) {
        /* crc32 16-byte chunks */
        z_size_t chunk_size = len & ~Z_CRC32_SSE42_CHUNKSIZE_MASK;
        crc = ~crc32_sse42_simd_(buf, chunk_size, ~(uint32_t) crc);
        /* check remaining data */
        len -= chunk_size;
        if (!len)
            return crc;
        /* Fall into the default crc32 for the remaining data. */
        buf += chunk_size;
    }
#else
    if (buf == Z_NULL) {
        return 0UL;
    }
#endif /* CRC32_SIMD_SSE42_PCLMUL */

#ifdef DYNAMIC_CRC_TABLE
    if (crc_table_empty)
        make_crc_table();
#endif /* DYNAMIC_CRC_TABLE */

#ifdef BYFOUR
    if (sizeof(void *) == sizeof(ptrdiff_t)) {
#if BYTE_ORDER == LITTLE_ENDIAN
        return crc32_little(crc, buf, len);
#elif BYTE_ORDER == BIG_ENDIAN
        return crc32_big(crc, buf, len);
#endif
    }
#endif /* BYFOUR */
    crc = crc ^ 0xffffffffUL;
#ifdef UNROLL_LESS
    while (len >= 4) {
        DO4;
        len -= 4;
    }
#else
    while (len >= 8) {
        DO8;
        len -= 8;
    }
#endif 
    if (len) do {
        DO1;
    } while (--len);
    return crc ^ 0xffffffffUL;
#endif
}

/* ========================================================================= */
unsigned long ZEXPORT crc32(crc, buf, len)
    unsigned long crc;
    const unsigned char FAR *buf;
    uInt len;
{
    return crc32_z(crc, buf, len);
}

#ifdef BYFOUR

/* ========================================================================= */
#define DOLIT4 tmp = *buf4++ ^ c; \
        c = crc_table[3][tmp & 0xff] ^ crc_table[2][(tmp >> 8) & 0xff] ^ \
            crc_table[1][(tmp >> 16) & 0xff] ^ crc_table[0][tmp >> 24]
#define DOLIT32 DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4

/* ========================================================================= */
local unsigned long crc32_little(crc, buf, len)
    unsigned long crc;
    const unsigned char FAR *buf;
    z_size_t len;
{
    register z_crc_t c;
    register z_crc_t tmp;
    register const z_crc_t FAR *buf4;

    c = (z_crc_t)crc;
    c = ~c;
    // This has a negative performance impact on modern CPUs (Intel Ivy Bridge etc.)
    // Disabling the alignment loop improves performance by ~10%
#if 0
    while (len && ((ptrdiff_t)buf & 3)) {
        c = crc_table[0][(c ^ *buf++) & 0xff] ^ (c >> 8);
        len--;
    }
#endif
    buf4 = (const z_crc_t FAR *)(const void FAR *)buf;

#ifndef UNROLL_LESS
    while (len >= 32) {
        DOLIT32;
        len -= 32;
    }
#endif

    while (len >= 4) {
        DOLIT4;
        len -= 4;
    }
    buf = (const unsigned char FAR *)buf4;

    if (len) do {
        c = crc_table[0][(c ^ *buf++) & 0xff] ^ (c >> 8);
    } while (--len);
    c = ~c;
    return (unsigned long)c;
}

#if BYTE_ORDER == BIG_ENDIAN
/* ========================================================================= */
#define DOBIG4 c ^= *++buf4; \
        c = crc_table[4][c & 0xff] ^ crc_table[5][(c >> 8) & 0xff] ^ \
            crc_table[6][(c >> 16) & 0xff] ^ crc_table[7][c >> 24]
#define DOBIG32 DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4

/* ========================================================================= */
local unsigned long crc32_big(crc, buf, len)
    unsigned long crc;
    const unsigned char FAR *buf;
    z_size_t len;
{
    register z_crc_t c;
    register const z_crc_t FAR *buf4;

    c = ZSWAP32((z_crc_t)crc);
    c = ~c;
    // See comments in crc32_little
#if 0
    while (len && ((ptrdiff_t)buf & 3)) {
        c = crc_table[4][(c >> 24) ^ *buf++] ^ (c << 8);
        len--;
    }
#endif

    buf4 = (const z_crc_t FAR *)(const void FAR *)buf;
    buf4--;

#ifndef UNROLL_LESS
    while (len >= 32) {
        DOBIG32;
        len -= 32;
    }
#endif

    while (len >= 4) {
        DOBIG4;
        len -= 4;
    }
    buf4++;
    buf = (const unsigned char FAR *)buf4;

    if (len) do {
        c = crc_table[4][(c >> 24) ^ *buf++] ^ (c << 8);
    } while (--len);
    c = ~c;
    return (unsigned long)(ZSWAP32(c));
}
#endif

#endif /* BYFOUR */

/* ========================================================================= */
uLong ZEXPORT crc32_combine64(crc1, crc2, len2)
    uLong crc1;
    uLong crc2;
    z_off64_t len2;
{
#ifdef DYNAMIC_CRC_TABLE
    once(&made, make_crc_table);
#endif /* DYNAMIC_CRC_TABLE */
    return multmodp(x2nmodp(len2, 3), crc1) ^ crc2;
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine(crc1, crc2, len2)
    uLong crc1;
    uLong crc2;
    z_off_t len2;
{
    return crc32_combine64(crc1, crc2, len2);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine_gen64(len2)
    z_off64_t len2;
{
#ifdef DYNAMIC_CRC_TABLE
    once(&made, make_crc_table);
#endif /* DYNAMIC_CRC_TABLE */
    return x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine_gen(len2)
    z_off_t len2;
{
    return crc32_combine_gen64(len2);
}

/* ========================================================================= */
uLong crc32_combine_op(crc1, crc2, op)
    uLong crc1;
    uLong crc2;
    uLong op;
{
    return multmodp(op, crc1) ^ crc2;
}

ZLIB_INTERNAL void crc_reset(deflate_state *const s)
{
#if defined(USE_PCLMUL_CRC)
    if (x86_cpu_has_pclmul) {
        crc_fold_init(s->crc0);
        s->strm->adler = 0;
        return;
    }
#endif
    s->strm->adler = crc32(0L, Z_NULL, 0);
}

ZLIB_INTERNAL void crc_finalize(deflate_state *const s)
{
#if defined(USE_PCLMUL_CRC)
    if (x86_cpu_has_pclmul) {
        s->strm->adler = crc_fold_512to32(s->crc0);
    }
#endif
}

ZLIB_INTERNAL void copy_with_crc(z_streamp strm, Bytef *dst, long size)
{
#if defined(USE_PCLMUL_CRC)
    if (x86_cpu_has_pclmul) {
        crc_fold_copy(strm->state->crc0, dst, strm->next_in, size);
        return;
    }
#endif
    zmemcpy(dst, strm->next_in, size);
    strm->adler = crc32(strm->adler, dst, size);
}
