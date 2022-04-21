#pragma once

#include <stdint.h>

extern uint32_t chunksize_c(void);
extern uint8_t* chunkcopy_c(uint8_t* out, uint8_t const* from, unsigned len);
extern uint8_t* chunkcopy_safe_c(uint8_t* out, uint8_t const* from, unsigned len, uint8_t* safe);
extern uint8_t* chunkunroll_c(uint8_t* out, unsigned* dist, unsigned* len);
extern uint8_t* chunkmemset_c(uint8_t* out, unsigned dist, unsigned len);
extern uint8_t* chunkmemset_safe_c(uint8_t* out, unsigned dist, unsigned len, unsigned left);
