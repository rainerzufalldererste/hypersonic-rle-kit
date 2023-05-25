#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #define RLE24_EXTREME_MAX_COPY_RANGE (127)
  #define RLE24_EXTRRME_FULL_COPY_SIZE (4 + 1)
#else
  #define RLE24_EXTREME_MAX_COPY_RANGE (255)
  #define RLE24_EXTRRME_FULL_COPY_SIZE (4)
#endif

#ifndef PACKED
  #define RLE24_EXTREME_MULTI_MIN_RANGE_SHORT ((3 + 1 + 1) + 2)
  #define RLE24_EXTREME_MULTI_MIN_RANGE_LONG ((3 + 1 + 4 + RLE24_EXTRRME_FULL_COPY_SIZE) + 2)
#else
  #define RLE24_EXTREME_MULTI_MIN_RANGE_SHORT ((1 + 1) + 1)
  #define RLE24_EXTREME_MULTI_MIN_RANGE_MEDIUM ((3 + 1 + 1) + 1)
  #define RLE24_EXTREME_MULTI_MIN_RANGE_LONG ((3 + 1 + 4 + RLE24_EXTRRME_FULL_COPY_SIZE) + 1)
#endif

#ifndef UNBOUND
  #ifdef PACKED
    #define CODEC sym_packed
  #else
    #define CODEC sym
  #endif
#else
  #ifdef PACKED
    #define CODEC byte_packed
  #else
    #define CODEC byte
  #endif
#endif

#define COMPRESS_IMPL_SSE2
#define IMPL_SSE2

#include "rle24_extreme_cpu_encode.h"

#undef IMPL_SSE2
#define IMPL_SSSE3

#include "rle24_extreme_cpu_encode.h"

#undef IMPL_SSSE3
#undef COMPRESS_IMPL_SSE2

#define COMPRESS_IMPL_AVX2
#define IMPL_AVX2

#include "rle24_extreme_cpu_encode.h"

#undef IMPL_AVX2
#undef COMPRESS_IMPL_AVX2

#include "rle24_extreme_cpu_encode.h"

uint32_t CONCAT3(rle24_, CODEC, _compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  _DetectCPUFeatures();

  if (avx2Supported)
    return CONCAT3(rle24_, CODEC, _compress_avx2)(pIn, inSize, pOut, outSize);
  else if (ssse3Supported)
    return CONCAT3(rle24_, CODEC, _compress_ssse3)(pIn, inSize, pOut, outSize);
  else if (sse2Supported)
    return CONCAT3(rle24_, CODEC, _compress_sse2)(pIn, inSize, pOut, outSize);
  else
    return CONCAT3(rle24_, CODEC, _compress_base)(pIn, inSize, pOut, outSize);
}

#define IMPL_SSE2
#include "rle24_extreme_cpu_decode.h"
#undef IMPL_SSE2

#define IMPL_SSSE3
#include "rle24_extreme_cpu_decode.h"
#undef IMPL_SSSE3

#define IMPL_AVX2
#include "rle24_extreme_cpu_decode.h"
#undef IMPL_AVX2

uint32_t CONCAT3(rle24_, CODEC, _decompress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[1];
  const size_t expectedOutSize = ((uint32_t *)pIn)[0];

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  size_t index = sizeof(uint32_t) * 2;

  _DetectCPUFeatures();

  pIn += index;

  if (avx2Supported)
    CONCAT3(rle24_, CODEC, _decompress_avx2)(pIn, pOut);
  else if (ssse3Supported)
    CONCAT3(rle24_, CODEC, _decompress_ssse3)(pIn, pOut);
  else
    CONCAT3(rle24_, CODEC, _decompress_sse2)(pIn, pOut);

  return (uint32_t)expectedOutSize;
}

#undef RLE24_EXTREME_MAX_COPY_RANGE
#undef RLE24_EXTRRME_FULL_COPY_SIZE

#undef RLE24_EXTREME_MULTI_MIN_RANGE_SHORT
#undef RLE24_EXTREME_MULTI_MIN_RANGE_LONG

#ifdef RLE24_EXTREME_MULTI_MIN_RANGE_MEDIUM
  #undef RLE24_EXTREME_MULTI_MIN_RANGE_MEDIUM
#endif

#undef CODEC
