#pragma once

#include "cstdint"
typedef float float32_t;
typedef double float64_t;

#define pack754_32(f) (static_cast<uint32_t>(pack754((f), 32, 8)))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (static_cast<float32_t>(unpack754((i), 32, 8)))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(float64_t f, uint32_t bits, uint32_t expbits);
float64_t unpack754(uint64_t i, uint32_t bits, uint32_t expbits);