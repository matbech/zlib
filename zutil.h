/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2022 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ifndef ZUTIL_H
#define ZUTIL_H

#ifdef HAVE_HIDDEN
#  define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#else
#  define ZLIB_INTERNAL
#endif

#include "zlib.h"

#if defined(STDC) && !defined(Z_SOLO)
#  if !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef Z_SOLO
   typedef long ptrdiff_t;  /* guess -- will be caught if guess is wrong */
#endif

#ifndef local
#  define local static
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

typedef unsigned char  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;

extern z_const char * const z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]

#define ERR_RETURN(strm,err) \
  return (strm->msg = ERR_MSG(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */

#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#if defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    if defined(__TURBOC__) || defined(__BORLANDC__)
#      if (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         void _Cdecl farfree( void *block );
         void *_Cdecl farmalloc( unsigned long nbytes );
#      else
#        include <alloc.h>
#      endif
#    else /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#if defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  if __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  else
#    define OS_CODE 8
#  endif
#endif

#if defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  if defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#if defined(MACOS) || defined(TARGET_OS_MAC)
#  define OS_CODE  7
#  ifndef Z_SOLO
#    if defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#      include <unix.h> /* for fdopen */
#    else
#      ifndef fdopen
#        define fdopen(fd,mode) NULL /* No fdopen() */
#      endif
#    endif
#  endif
#endif

#ifdef __acorn
#  define OS_CODE 13
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
#  define OS_CODE  10
#endif

#ifdef _BEOS_
#  define OS_CODE  16
#endif

#ifdef __TOS_OS400__
#  define OS_CODE 18
#endif

#ifdef __APPLE__
#  define OS_CODE 19
#endif

#if defined(_BEOS_) || defined(RISCOS)
#  define fdopen(fd,mode) NULL /* No fdopen() */
#endif

#if (defined(_MSC_VER) && (_MSC_VER > 600)) && !defined __INTERIX
#  if defined(_WIN32_WCE)
#    define fdopen(fd,mode) NULL /* No fdopen() */
#    ifndef _PTRDIFF_T_DEFINED
       typedef int ptrdiff_t;
#      define _PTRDIFF_T_DEFINED
#    endif
#  else
#    define fdopen(fd,type)  _fdopen(fd,type)
#  endif
#endif

#if defined(__BORLANDC__) && !defined(MSDOS)
  #pragma warn -8004
  #pragma warn -8008
  #pragma warn -8066
#endif

/* provide prototypes for these when building zlib without LFS */
#if !defined(_WIN32) && \
    (!defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0)
    ZEXTERN uLong ZEXPORT adler32_combine64 OF((uLong, uLong, z_off_t));
    ZEXTERN uLong ZEXPORT crc32_combine64 OF((uLong, uLong, z_off_t));
    ZEXTERN uLong ZEXPORT crc32_combine_gen64 OF((z_off_t));
#endif

        /* common defaults */

#ifndef OS_CODE
#  define OS_CODE  3     /* assume Unix */
#endif

#ifndef F_OPEN
#  define F_OPEN(name, mode) fopen((name), (mode))
#endif

         /* functions */

#if defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#if defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#if defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  else
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#else
   void ZLIB_INTERNAL zmemcpy OF((Bytef* dest, const Bytef* source, uInt len));
   int ZLIB_INTERNAL zmemcmp OF((const Bytef* s1, const Bytef* s2, uInt len));
   void ZLIB_INTERNAL zmemzero OF((Bytef* dest, uInt len));
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   extern int ZLIB_INTERNAL z_verbose;
   extern void ZLIB_INTERNAL z_error OF((char *m));
#  define Assert(cond,msg) {if(!(cond)) z_error(msg);}
#  define Trace(x) {if (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {if (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {if (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (z_verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc OF((voidpf opaque, unsigned items,
                                    unsigned size));
   void ZLIB_INTERNAL zcfree  OF((voidpf opaque, voidpf ptr));
#endif

#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {if (p) ZFREE(s, p);}

   /* Reverse the bytes in a 32-bit value. Use compiler intrinsics when
   possible to take advantage of hardware implementations. */
#if defined(_WIN32) && (_MSC_VER >= 1300)
#  pragma intrinsic(_byteswap_ulong)
#  define ZSWAP32(q) _byteswap_ulong(q)

#elif defined(__clang__) || (defined(__GNUC__) && \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)))
#  define ZSWAP32(q) __builtin_bswap32(q)

#elif defined(__GNUC__) && (__GNUC__ >= 2) && defined(__linux__)
#  include <byteswap.h>
#  define ZSWAP32(q) bswap_32(q)

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#  include <sys/endian.h>
#  define ZSWAP32(q) bswap32(q)

#elif defined(__INTEL_COMPILER)
#  define ZSWAP32(q) _bswap(q)

#else
#  define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))
#endif /* ZSWAP32 */

/* Only enable likely/unlikely if the compiler is known to support it */
#if (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__INTEL_COMPILER) || defined(__clang__)
#  ifndef likely
#    define likely(x)      __builtin_expect(!!(x), 1)
#  endif
#  ifndef unlikely
#    define unlikely(x)    __builtin_expect(!!(x), 0)
#  endif
#else
#  ifndef likely
#    define likely(x)      x
#  endif
#  ifndef unlikely
#    define unlikely(x)    x
#  endif
#endif /* (un)likely */

#ifdef _MSC_VER
#define zalign(x) __declspec(align(x))
#else
#define zalign(x) __attribute__((aligned((x))))
#endif

#ifdef _MSC_VER
// __forceinline is apparently required (__inline is not sufficient) for VS14 to inline the insert_string_sse function with /O2
#define INLINE __forceinline
#else
#define INLINE inline
#endif

/* Only enable likely/unlikely if the compiler is known to support it */
#if (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__INTEL_COMPILER) || defined(__clang__)
#  define LIKELY_NULL(x)        __builtin_expect((x) != 0, 0)
#  define LIKELY(x)             __builtin_expect(!!(x), 1)
#  define UNLIKELY(x)           __builtin_expect(!!(x), 0)
#  define PREFETCH_L1(addr)     __builtin_prefetch(addr, 0, 3)
#  define PREFETCH_L2(addr)     __builtin_prefetch(addr, 0, 2)
#  define PREFETCH_RW(addr)     __builtin_prefetch(addr, 1, 2)
#elif defined(__WIN__)
#  include <xmmintrin.h>
#  define LIKELY_NULL(x)        x
#  define LIKELY(x)             x
#  define UNLIKELY(x)           x
#  define PREFETCH_L1(addr)     _mm_prefetch((char *) addr, _MM_HINT_T0)
#  define PREFETCH_L2(addr)     _mm_prefetch((char *) addr, _MM_HINT_T1)
#  define PREFETCH_RW(addr)     _mm_prefetch((char *) addr, _MM_HINT_T1)
#else
#  define LIKELY_NULL(x)        x
#  define LIKELY(x)             x
#  define UNLIKELY(x)           x
#  define PREFETCH_L1(addr)     addr
#  define PREFETCH_L2(addr)     addr
#  define PREFETCH_RW(addr)     addr
#endif /* (un)likely */

#ifndef NO_UNALIGNED
#  if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__) || defined(_M_AMD64)
#    define UNALIGNED_OK
#    define UNALIGNED64_OK
#  elif defined(__i386__) || defined(__i486__) || defined(__i586__) || \
        defined(__i686__) || defined(_X86_) || defined(_M_IX86)
#    define UNALIGNED_OK
#  elif defined(__aarch64__) || defined(_M_ARM64)
#    if (defined(__GNUC__) && defined(__ARM_FEATURE_UNALIGNED)) || !defined(__GNUC__)
#      define UNALIGNED_OK
#      define UNALIGNED64_OK
#    endif
#  elif defined(__arm__) || (_M_ARM >= 7)
#    if (defined(__GNUC__) && defined(__ARM_FEATURE_UNALIGNED)) || !defined(__GNUC__)
#      define UNALIGNED_OK
#    endif
#  elif defined(__powerpc64__) || defined(__ppc64__)
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define UNALIGNED_OK
#      define UNALIGNED64_OK
#    endif
#  endif
#endif

/* Force compiler to emit unaligned memory accesses if unaligned access is supported
   on the architecture, otherwise don't assume unaligned access is supported. Older
   compilers don't optimize memcpy and memcmp calls to unaligned access instructions
   when it is supported on the architecture resulting in significant performance impact.
   Newer compilers might optimize memcpy but not all optimize memcmp for all integer types. */
#ifdef UNALIGNED_OK
#  define zmemcpy_2(dest, src)    (*((uint16_t *)(dest)) = *((uint16_t *)(src)))
#  define zmemcmp_2(str1, str2)   (*((uint16_t *)(str1)) != *((uint16_t *)(str2)))
#  define zmemcpy_4(dest, src)    (*((uint32_t *)(dest)) = *((uint32_t *)(src)))
#  define zmemcmp_4(str1, str2)   (*((uint32_t *)(str1)) != *((uint32_t *)(str2)))
#  if defined(UNALIGNED64_OK) && (UINTPTR_MAX == UINT64_MAX)
#    define zmemcpy_8(dest, src)  (*((uint64_t *)(dest)) = *((uint64_t *)(src)))
#    define zmemcmp_8(str1, str2) (*((uint64_t *)(str1)) != *((uint64_t *)(str2)))
#  else
#    define zmemcpy_8(dest, src)  (((uint32_t *)(dest))[0] = ((uint32_t *)(src))[0], \
                                   ((uint32_t *)(dest))[1] = ((uint32_t *)(src))[1])
#    define zmemcmp_8(str1, str2) (((uint32_t *)(str1))[0] != ((uint32_t *)(str2))[0] || \
                                   ((uint32_t *)(str1))[1] != ((uint32_t *)(str2))[1])
#  endif
#else
#  define zmemcpy_2(dest, src)  memcpy(dest, src, 2)
#  define zmemcmp_2(str1, str2) memcmp(str1, str2, 2)
#  define zmemcpy_4(dest, src)  memcpy(dest, src, 4)
#  define zmemcmp_4(str1, str2) memcmp(str1, str2, 4)
#  define zmemcpy_8(dest, src)  memcpy(dest, src, 8)
#  define zmemcmp_8(str1, str2) memcmp(str1, str2, 8)
#endif

 /* Minimum of a and b. */
#define MIN(a, b) ((a) > (b) ? (b) : (a))

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
/* __builtin_ctzl
 *  - For 0, the result is undefined
 *  - On the x86 architecture, it is typically implemented using BSF
 *  - the equivalent intrinsic on MSC is _BitScanForward
 *
 * _tzcnt_u32
 *  - For 0, the result is the size of the operand 
 *  - On processors that do not support TZCNT, the instruction byte encoding is executed as BSF. In this case the result for 0
 *    is undefined.
 *  - Performance:
 *    + AMD: The reciprocal throughput for TZCNT is 2 vs 3 for BSF
 *    + Intel: On modern Intel CPUs (Haswell), the performance of TZCNT is equivalent to BSF
 *    Reference: http://www.agner.org/optimize/instruction_tables.pdf
*/
#if defined(_M_IX86) || defined(_M_AMD64)
#define __builtin_ctzl _tzcnt_u32
#if defined(_M_AMD64)
#define __builtin_ctzll _tzcnt_u64
#endif
#else
static __forceinline unsigned long __builtin_ctzl(unsigned long value)
{
    unsigned long trailing_zero;
    _BitScanForward(&trailing_zero, value);
    return trailing_zero;
}

#if defined(_M_ARM64)
static __forceinline unsigned long __builtin_ctzll(unsigned long long value) 
{
    unsigned long trailing_zero;
    _BitScanForward64(&trailing_zero, value);
    return trailing_zero;
}
#endif

#endif

#endif

#endif /* ZUTIL_H */
