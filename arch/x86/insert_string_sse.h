#include "assert.h"

/* Safe to inline this as GCC/clang will use inline asm and Visual Studio will
* use intrinsic without extra params
*/
INLINE Pos insert_string_sse(deflate_state *const s, const Pos str)
{
    Pos ret;
    unsigned *ip, val, h = 0;

    ip = (unsigned *) &s->window[str];
    val = *ip;

    if (s->level >= 6)
        val &= 0xFFFFFF;

    /* Windows clang should use inline asm */
#if defined(_MSC_VER) && !defined(__clang__)
    h = _mm_crc32_u32(h, val);
#elif defined(__i386__) || defined(__amd64__)
    __asm__ __volatile__(
        "crc32 %1,%0\n\t"
        : "+r" (h)
        : "r" (val)
        );
#else
    /* This should never happen */
    assert(0);
#endif

    ret = s->head[h & s->hash_mask];
    s->head[h & s->hash_mask] = str;
    s->prev[str & s->w_mask] = ret;
    return ret;
}
