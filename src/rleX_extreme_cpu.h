#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #define RLEX_EXTREME_MAX_COPY_RANGE (127)
  #define RLEX_EXTRRME_FULL_COPY_SIZE (4 + 1)
#else
  #define RLEX_EXTREME_MAX_COPY_RANGE (255)
  #define RLEX_EXTRRME_FULL_COPY_SIZE (4)
#endif

#ifndef PACKED
  #define RLEX_EXTREME_MULTI_MIN_RANGE_SHORT ((sizeof(symbol_t) + 1 + 1) + 2)
  #define RLEX_EXTREME_MULTI_MIN_RANGE_LONG ((sizeof(symbol_t) + 1 + 4 + RLEX_EXTRRME_FULL_COPY_SIZE) + 2)
#else
  #define RLEX_EXTREME_MULTI_MIN_RANGE_SHORT ((1 + 1) + 1)
  #define RLEX_EXTREME_MULTI_MIN_RANGE_MEDIUM ((sizeof(symbol_t) + 1 + 1) + 1)
  #define RLEX_EXTREME_MULTI_MIN_RANGE_LONG ((sizeof(symbol_t) + 1 + 4 + RLEX_EXTRRME_FULL_COPY_SIZE) + 1)
#endif

#if TYPE_SIZE == 64
  #define _mm_set1_epi64 _mm_set1_epi64x
  #define _mm256_set1_epi64 _mm256_set1_epi64x
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
  #include "rleX_extreme_cpu_encode.h"
#undef COMPRESS_IMPL_SSE2

#define COMPRESS_IMPL_AVX2
  #include "rleX_extreme_cpu_encode.h"
#undef COMPRESS_IMPL_AVX2

#include "rleX_extreme_cpu_encode.h"

uint32_t CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _compress))(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  _DetectCPUFeatures();

  if (avx2Supported)
    return CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _compress_avx2))(pIn, inSize, pOut, outSize);
  else if (sse2Supported)
    return CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _compress_sse2))(pIn, inSize, pOut, outSize);
  else
    return CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _compress_base))(pIn, inSize, pOut, outSize);
}

#define IMPL_SSE2
  #include "rleX_extreme_cpu_decode.h"
#undef IMPL_SSE2

#define IMPL_SSE41
  #include "rleX_extreme_cpu_decode.h"
#undef IMPL_SSE41

#define IMPL_AVX
  #include "rleX_extreme_cpu_decode.h"
#undef IMPL_AVX

#define IMPL_AVX2
  #include "rleX_extreme_cpu_decode.h"
#undef IMPL_AVX2

#define IMPL_AVX512F
  #include "rleX_extreme_cpu_decode.h"
#undef IMPL_AVX512F

uint32_t CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress))(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
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

  if (avx512FSupported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_avx512f))(pIn, pOut);
  else if (avx2Supported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_avx2))(pIn, pOut);
  else if (avxSupported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_avx))(pIn, pOut);
  else if (sse41Supported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_sse41))(pIn, pOut);
  else
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_sse2))(pIn, pOut);

  return (uint32_t)expectedOutSize;
}

#undef RLEX_EXTRRME_FULL_COPY_SIZE
#undef RLEX_EXTREME_MAX_COPY_RANGE

#undef RLEX_EXTREME_MULTI_MIN_RANGE_SHORT
#undef RLEX_EXTREME_MULTI_MIN_RANGE_LONG

#ifdef RLEX_EXTREME_MULTI_MIN_RANGE_SAME_SYMBOL_SHORT
  #undef RLEX_EXTREME_MULTI_MIN_RANGE_SAME_SYMBOL_SHORT
#endif

#ifdef RLEX_EXTREME_MULTI_MIN_RANGE_MEDIUM
  #undef RLEX_EXTREME_MULTI_MIN_RANGE_MEDIUM
#endif

#if TYPE_SIZE == 64
  #undef _mm_set1_epi64
  #undef _mm256_set1_epi64
#endif

#undef CODEC
