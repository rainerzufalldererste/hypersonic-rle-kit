#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #define RLE48_EXTREME_MAX_COPY_RANGE (127)
  #define RLE48_EXTRRME_FULL_COPY_SIZE (4 + 1)
#else
  #define RLE48_EXTREME_MAX_COPY_RANGE (255)
  #define RLE48_EXTRRME_FULL_COPY_SIZE (4)
#endif

#ifndef PACKED
  #define RLE48_EXTREME_MULTI_MIN_RANGE_SHORT ((6 + 1 + 1) + 2)
  #define RLE48_EXTREME_MULTI_MIN_RANGE_LONG ((6 + 1 + 4 + RLE48_EXTRRME_FULL_COPY_SIZE) + 2)
#else
  #define RLE48_EXTREME_MULTI_MIN_RANGE_SHORT ((1 + 1) + 1)
  #define RLE48_EXTREME_MULTI_MIN_RANGE_MEDIUM ((6 + 1 + 1) + 1)
  #define RLE48_EXTREME_MULTI_MIN_RANGE_LONG ((6 + 1 + 4 + RLE48_EXTRRME_FULL_COPY_SIZE) + 1)
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

#include "rle48_extreme_cpu_encode.h"

#undef IMPL_SSE2
#define IMPL_SSSE3

#include "rle48_extreme_cpu_encode.h"

#undef IMPL_SSSE3
#undef COMPRESS_IMPL_SSE2

#define COMPRESS_IMPL_AVX2
#define IMPL_AVX2

#include "rle48_extreme_cpu_encode.h"

#undef IMPL_AVX2
#undef COMPRESS_IMPL_AVX2

#include "rle48_extreme_cpu_encode.h"

uint32_t CONCAT3(rle48_, CODEC, _compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  _DetectCPUFeatures();

  if (avx2Supported)
    return CONCAT3(rle48_, CODEC, _compress_avx2)(pIn, inSize, pOut, outSize);
  else if (ssse3Supported)
    return CONCAT3(rle48_, CODEC, _compress_ssse3)(pIn, inSize, pOut, outSize);
  else if (sse2Supported)
    return CONCAT3(rle48_, CODEC, _compress_sse2)(pIn, inSize, pOut, outSize);
  else
    return CONCAT3(rle48_, CODEC, _compress_base)(pIn, inSize, pOut, outSize);
}

static void CONCAT3(rle48_, CODEC, _decompress_sse2)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();
  
  const __m128i pattern00 = _mm_set_epi8( 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -1);
  const __m128i pattern10 = _mm_set_epi8( 0,  0, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0);
  
  const __m128i pattern02 = _mm_set_epi8( 0,  0,  0,  0, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0);
  const __m128i pattern12 = _mm_set_epi8(-1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0);
  const __m128i pattern22 = _mm_set_epi8( 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1, -1, -1, -1);
  
  const __m128i pattern04 = _mm_set_epi8(-1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0);
  const __m128i pattern14 = _mm_set_epi8( 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1, -1);
  const __m128i pattern24 = _mm_set_epi8( 0,  0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0);

  const __m128i pattern16 = _mm_set_epi8( 0,  0,  0,  0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -1,  0,  0);
  const __m128i pattern26 = _mm_set_epi8(-1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0);
  
  typedef uint64_t symbol_t;
  const size_t symbolSize = 6;

#ifdef PACKED
  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
#endif

  while (true)
  {
#ifndef PACKED
    symbol = _mm_set1_epi64x(*(symbol_t *)pInStart);

    pInStart += symbolSize;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;
    pInStart++;

    const uint8_t sameSymbol = (symbolCount & 0b10000000);
    symbolCount &= 0b01111111;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    if (!sameSymbol)
    {
      symbol = _mm_set1_epi64x(*(symbol_t *)pInStart);
      pInStart += symbolSize;

      const __m128i shift2 = _mm_or_si128(_mm_srli_si128(symbol, 2), _mm_slli_si128(symbol, 14));

      symbol0 = _mm_or_si128(_mm_and_si128(symbol, pattern00), _mm_and_si128(shift2, pattern02));
      const __m128i symbol1a = _mm_or_si128(_mm_and_si128(symbol, pattern10), _mm_and_si128(shift2, pattern12));

      const __m128i shift4 = _mm_or_si128(_mm_srli_si128(symbol, 4), _mm_slli_si128(symbol, 12));

      symbol0 = _mm_or_si128(symbol0, _mm_and_si128(shift4, pattern04));
      symbol2 = _mm_or_si128(_mm_and_si128(shift2, pattern22), _mm_and_si128(shift4, pattern24));

      const __m128i shift6 = _mm_or_si128(_mm_srli_si128(symbol, 6), _mm_slli_si128(symbol, 10));

      const __m128i symbol1b = _mm_or_si128(_mm_and_si128(shift4, pattern14), _mm_and_si128(shift6, pattern16));
      symbol2 = _mm_or_si128(symbol2, _mm_and_si128(shift6, pattern26));
      symbol1 = _mm_or_si128(symbol1a, symbol1b);
    }
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#else
    offset = (size_t)*pInStart;
    pInStart++;

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_SSE_MULTI;

    if (!symbolCount)
      return;

#ifndef UNBOUND
    symbolCount = (symbolCount + (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) - 1) * symbolSize;
#else
    symbolCount = (symbolCount + RLE48_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

#ifndef PACKED
      const __m128i shift2 = _mm_or_si128(_mm_srli_si128(symbol, 2), _mm_slli_si128(symbol, 14));

      __m128i symbol0 = _mm_or_si128(_mm_and_si128(symbol, pattern00), _mm_and_si128(shift2, pattern02));
      const __m128i symbol1a = _mm_or_si128(_mm_and_si128(symbol, pattern10), _mm_and_si128(shift2, pattern12));

      const __m128i shift4 = _mm_or_si128(_mm_srli_si128(symbol, 4), _mm_slli_si128(symbol, 12));

      symbol0 = _mm_or_si128(symbol0, _mm_and_si128(shift4, pattern04));
      __m128i symbol2 = _mm_or_si128(_mm_and_si128(shift2, pattern22), _mm_and_si128(shift4, pattern24));

      const __m128i shift6 = _mm_or_si128(_mm_srli_si128(symbol, 6), _mm_slli_si128(symbol, 10));

      const __m128i symbol1b = _mm_or_si128(_mm_and_si128(shift4, pattern14), _mm_and_si128(shift6, pattern16));
      symbol2 = _mm_or_si128(symbol2, _mm_and_si128(shift6, pattern26));
      const __m128i symbol1 = _mm_or_si128(symbol1a, symbol1b);
#endif

      while (pCOut < pCOutEnd)
      {
        _mm_storeu_si128((__m128i *)pCOut, symbol0);
        pCOut += sizeof(symbol0);
        _mm_storeu_si128((__m128i *)pCOut, symbol1);
        pCOut += sizeof(symbol1);
        _mm_storeu_si128((__m128i *)pCOut, symbol2);
        pCOut += sizeof(symbol2);
      }

      pOut = pCOutEnd;
    }
  }
}

#ifndef _MSC_VER
__attribute__((target("ssse3")))
#endif
static void CONCAT3(rle48_, CODEC, _decompress_ssse3)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();

  const __m128i shuffle0 = _mm_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
  const __m128i shuffle1 = _mm_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4);
  const __m128i shuffle2 = _mm_set_epi8(5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2);

  typedef uint64_t symbol_t;
  const size_t symbolSize = 6;

#ifdef PACKED
  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
#endif

  while (true)
  {
#ifndef PACKED
    symbol = _mm_set1_epi64x(*(symbol_t *)pInStart);

    pInStart += symbolSize;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;
    pInStart++;

    const uint8_t sameSymbol = (symbolCount & 0b10000000);
    symbolCount &= 0b01111111;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    if (!sameSymbol)
    {
      symbol = _mm_set1_epi64x(*(symbol_t *)pInStart);
      pInStart += symbolSize;

      symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm_shuffle_epi8(symbol, shuffle2);
    }
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#else
    offset = (size_t)*pInStart;
    pInStart++;

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_SSE_MULTI;

    if (!symbolCount)
      return;

#ifndef UNBOUND
    symbolCount = (symbolCount + (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) - 1) * symbolSize;
#else
    symbolCount = (symbolCount + RLE48_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

#ifndef PACKED
      const __m128i symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      const __m128i symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      const __m128i symbol2 = _mm_shuffle_epi8(symbol, shuffle2);
#endif

      while (pCOut < pCOutEnd)
      {
        _mm_storeu_si128((__m128i *)pCOut, symbol0);
        pCOut += sizeof(symbol0);
        _mm_storeu_si128((__m128i *)pCOut, symbol1);
        pCOut += sizeof(symbol1);
        _mm_storeu_si128((__m128i *)pCOut, symbol2);
        pCOut += sizeof(symbol2);
      }

      pOut = pCOutEnd;
    }
  }
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static void CONCAT3(rle48_, CODEC, _decompress_avx2)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();

  const __m256i shuffle0 = _mm256_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
  const __m256i shuffle1 = _mm256_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2);
  const __m256i shuffle2 = _mm256_set_epi8(5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4);

  typedef uint64_t symbol_t;
  const size_t symbolSize = 6;

#ifdef PACKED
  __m256i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
#endif

  while (true)
  {
#ifndef PACKED
    symbol = _mm256_set1_epi64x(*(symbol_t *)pInStart);

    pInStart += symbolSize;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;
    pInStart++;

    const uint8_t sameSymbol = (symbolCount & 0b10000000);
    symbolCount &= 0b01111111;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    if (!sameSymbol)
    {
      symbol = _mm256_set1_epi64x(*(symbol_t *)pInStart);
      pInStart += symbolSize;

      symbol0 = _mm256_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm256_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm256_shuffle_epi8(symbol, shuffle2);
    }
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#else
    offset = (size_t)*pInStart;
    pInStart++;

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_AVX_MULTI;

    if (!symbolCount)
      return;

#ifndef UNBOUND
    symbolCount = (symbolCount + (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) - 1) * symbolSize;
#else
    symbolCount = (symbolCount + RLE48_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

#ifndef PACKED
      const __m256i symbol0 = _mm256_shuffle_epi8(symbol, shuffle0);
      const __m256i symbol1 = _mm256_shuffle_epi8(symbol, shuffle1);
      const __m256i symbol2 = _mm256_shuffle_epi8(symbol, shuffle2);
#endif

      while (pCOut < pCOutEnd)
      {
        _mm256_storeu_si256((__m256i *)pCOut, symbol0);
        pCOut += sizeof(symbol0);
        _mm256_storeu_si256((__m256i *)pCOut, symbol1);
        pCOut += sizeof(symbol1);
        _mm256_storeu_si256((__m256i *)pCOut, symbol2);
        pCOut += sizeof(symbol2);
      }

      pOut = pCOutEnd;
    }
  }
}

uint32_t CONCAT3(rle48_, CODEC, _decompress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
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
    CONCAT3(rle48_, CODEC, _decompress_avx2)(pIn, pOut);
  else if (ssse3Supported)
    CONCAT3(rle48_, CODEC, _decompress_ssse3)(pIn, pOut);
  else
    CONCAT3(rle48_, CODEC, _decompress_sse2)(pIn, pOut);

  return (uint32_t)expectedOutSize;
}

#undef RLE48_EXTREME_MAX_COPY_RANGE
#undef RLE48_EXTRRME_FULL_COPY_SIZE

#undef RLE48_EXTREME_MULTI_MIN_RANGE_SHORT
#undef RLE48_EXTREME_MULTI_MIN_RANGE_LONG

#ifdef RLE48_EXTREME_MULTI_MIN_RANGE_MEDIUM
  #undef RLE48_EXTREME_MULTI_MIN_RANGE_MEDIUM
#endif

#undef CODEC
