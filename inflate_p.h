/* inflate_p.h -- Private inline functions and macros shared with more than one deflate method
 *
 */

#ifndef INFLATE_P_H
#define INFLATE_P_H

#include <stdint.h>

/* Load registers with state in inflate() for speed */
#define LOAD() \
    do { \
        put = strm->next_out; \
        left = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } while (0)

/* Restore state from registers in inflate() */
#define RESTORE() \
    do { \
        strm->next_out = put; \
        strm->avail_out = left; \
        strm->next_in = (z_const unsigned char *)next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } while (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    do { \
        hold = 0; \
        bits = 0; \
    } while (0)

/* Ensure that there is at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate()/inflateBack(). */
#define NEEDBITS(n) \
    do { \
        while (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } while (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    (hold & ((1U << (unsigned)(n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } while (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    do { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } while (0)

#endif

/* Set mode=BAD and prepare error message */
#define SET_BAD(errmsg) \
    do { \
        state->mode = BAD; \
        strm->msg = (char *)errmsg; \
    } while (0)

/* Behave like chunkcopy, but avoid writing beyond of legal output. */
static inline uint8_t* chunkcopy_safe(uint8_t *out, uint8_t *from, size_t len, uint8_t *safe) {
    uint32_t safelen = (uint32_t)((safe - out) + 1);
    len = MIN(len, safelen);
    int32_t olap_src = from >= out && from < out + len;
    int32_t olap_dst = out >= from && out < from + len;
    size_t tocopy;

    /* For all cases without overlap, memcpy is ideal */
    if (!(olap_src || olap_dst)) {
        memcpy(out, from, len);
        return out + len;
    }

    /* We are emulating a self-modifying copy loop here. To do this in a way that doesn't produce undefined behavior,
     * we have to get a bit clever. First if the overlap is such that src falls between dst and dst+len, we can do the
     * initial bulk memcpy of the nonoverlapping region. Then, we can leverage the size of this to determine the safest
     * atomic memcpy size we can pick such that we have non-overlapping regions. This effectively becomes a safe look
     * behind or lookahead distance */
    size_t non_olap_size = ((from > out) ? from - out : out - from);

    memcpy(out, from, non_olap_size);
    out += non_olap_size;
    from += non_olap_size;
    len -= non_olap_size;

    /* So this doesn't give use a worst case scenario of function calls in a loop,
     * we want to instead break this down into copy blocks of fixed lengths */
    while (len) {
        tocopy = MIN(non_olap_size, len);
        len -= tocopy;

        while (tocopy >= 32) {
            memcpy(out, from, 32);
            out += 32;
            from += 32;
            tocopy -= 32;
        }

        if (tocopy >= 16) {
            memcpy(out, from, 16);
            out += 16;
            from += 16;
            tocopy -= 16;
        }

        if (tocopy >= 8) {
            zmemcpy_8(out, from);
            out += 8;
            from += 8;
            tocopy -= 8;
        }

        if (tocopy >= 4) {
            zmemcpy_4(out, from);
            out += 4;
            from += 4;
            tocopy -= 4;
        }

        if (tocopy >= 2) {
            zmemcpy_2(out, from);
            out += 2;
            from += 2;
            tocopy -= 2;
        }

        if (tocopy) {
            *out++ = *from++;
        }
    }

    return out;
}
