#pragma once

#include "zutil.h"
#include <stdint.h>

#if defined(UNALIGNED64_OK)
/* UNALIGNED64_OK, 64-bit integer comparison */
INLINE uint32_t compare256_unaligned_64(const uint8_t* src0, const uint8_t* src1) {
    uint32_t len = 0;

    do {
        uint64_t sv, mv, diff;

        zmemcpy_8(&sv, src0);
        zmemcpy_8(&mv, src1);

        diff = sv ^ mv;
        if (diff) {
            uint64_t match_byte = __builtin_ctzll(diff) / 8;
            return len + (uint32_t) match_byte;
        }

        src0 += 8, src1 += 8, len += 8;
    } while (len < 256);

    return 256;
}

#endif
