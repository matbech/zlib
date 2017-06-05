# zlib
Drop in replacement for zlib 1.2.11 with optimizations from various sources.

This fork is based on the official zlib repository:
https://github.com/madler/zlib

## 3rd Party Patches
- Optimizations from Intel without the new deflate strategies (quick, medium)  
  deflate: crc32 implementation with PCLMULQDQ optimized folding  
  deflate: fill_window_sse  
  https://github.com/jtkukunas/zlib

- Optimizations from Cloudflare  
  deflate: use crc32 (SIMD) for hash  
  deflate: longest_match optimizations  
  https://github.com/cloudflare/zlib

- Others small changes  
  put_short optimization  
  inflate_fast: use memset  
  https://github.com/Dead2/zlib-ng

## Additional changes
- Support and optimizations for MSVC15 compiler  
  Support for _M_ARM  
  Use __forceinline

- Use tzcnt instead of bsf  
  This improves performance for AMD CPUs

- Implementation optimized for modern CPUs (Intel Nehalem)  
  Removed alignment loop in crc32  
  Less manual unrolling

- Others  
  Optimized insert_string loop

## New features
- General purpose crc32 interface  
  Based on Intel's PCLMULQDQ crc32 implementation which is already used for deflate.  
  New functions:  
  crc32_init  
  crc32_update  
  crc32_final  
  Brings ~200% performance improvement over the original zlib crc32 implementation
