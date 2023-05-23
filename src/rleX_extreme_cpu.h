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

static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_sse))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();

  typedef CONCAT3(uint, TYPE_SIZE, _t) symbol_t;

  while (true)
  {
#ifndef PACKED
    symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
    pInStart += sizeof(symbol_t);
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
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
      pInStart += sizeof(symbol_t);
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
    symbolCount = (symbolCount + (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
    symbolCount = (symbolCount + RLEX_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
    MEMSET_SSE_MULTI;
  }
}

#ifndef _MSC_VER
__attribute__((target("sse4.1")))
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_sse41))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol;

  typedef CONCAT3(uint, TYPE_SIZE, _t) symbol_t;

  while (true)
  {
#ifndef PACKED
    symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
    pInStart += sizeof(symbol_t);
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
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
      pInStart += sizeof(symbol_t);
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
    MEMCPY_SSE41_MULTI;

    if (!symbolCount)
      return;

#ifndef UNBOUND
    symbolCount = (symbolCount + (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
    symbolCount = (symbolCount + RLEX_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
    MEMSET_SSE_MULTI;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_avx))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();

  typedef CONCAT3(uint, TYPE_SIZE, _t) symbol_t;

  while (true)
  {
#ifndef PACKED
    symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
    pInStart += sizeof(symbol_t);
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
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
      pInStart += sizeof(symbol_t);
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
    symbolCount = (symbolCount + (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
    symbolCount = (symbolCount + RLEX_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
    MEMSET_AVX_MULTI;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_avx2))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();

  typedef CONCAT3(uint, TYPE_SIZE, _t) symbol_t;

  while (true)
  {
#ifndef PACKED
    symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
    pInStart += sizeof(symbol_t);
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
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
      pInStart += sizeof(symbol_t);
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
    MEMCPY_AVX2_MULTI;

    if (!symbolCount)
      return;

#ifndef UNBOUND
    symbolCount = (symbolCount + (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
    symbolCount = (symbolCount + RLEX_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
    MEMSET_AVX_MULTI;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx512f")))
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_avx512f))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m512i symbol = _mm512_setzero_si512();

  typedef CONCAT3(uint, TYPE_SIZE, _t) symbol_t;

  while (true)
  {
#ifndef PACKED
    symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
    pInStart += sizeof(symbol_t);
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
      symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
      pInStart += sizeof(symbol_t);
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
    MEMCPY_AVX512_MULTI;

    if (!symbolCount)
      return;

#ifndef UNBOUND
    symbolCount = (symbolCount + (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
    symbolCount = (symbolCount + RLEX_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
    MEMSET_AVX512_MULTI;
  }
}

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
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _decompress_sse))(pIn, pOut);

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
