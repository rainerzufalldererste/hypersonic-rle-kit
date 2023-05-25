#include "rle.h"
#include "rleX_extreme_common.h"

#define TYPE_SIZE 24
#define SIMD_TYPE_SIZE 32
#define COMPRESS_IMPL_SSE2
#define IMPL_SSE2

#include "rle24_extreme_cpu.h"

#undef COMPRESS_IMPL_SSE2
#define UNBOUND

#include "rle24_extreme_cpu.h"

#define PACKED

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
  #define PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle24_extreme_cpu.h"

#undef UNBOUND

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #undef PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle24_extreme_cpu.h"
