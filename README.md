# zlib
Drop in replacement for zlib 1.2.8 with optimizations from various sources.

This fork is based on the official zlib repository:
https://github.com/madler/zlib

With patches from:
- Optimizations from Intel without the new deflate strategies (quick, medium)
  https://github.com/jtkukunas/zlib

- Optimizations from Cloudflare
  https://github.com/cloudflare/zlib

- Others
  https://github.com/Dead2/zlib-ng

Additional changes:
- Support for MSVC14 compiler
