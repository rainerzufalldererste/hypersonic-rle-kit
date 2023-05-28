#include "rle.h"
#include "rleX_extreme_common.h"

#define TYPE_SIZE 48
#define SIMD_TYPE_SIZE 64x

#include "rle48_extreme_cpu.h"

#define UNBOUND

#include "rle48_extreme_cpu.h"

#define PACKED

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
  #define PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle48_extreme_cpu.h"

#undef UNBOUND

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #undef PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle48_extreme_cpu.h"
