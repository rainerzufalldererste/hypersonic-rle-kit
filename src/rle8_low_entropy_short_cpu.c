#include "rle8.h"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define ULTRA_MAX_BLOCK_LENGTH 32

bool rle8_low_entropy_short_get_compress_info(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_low_entropy_compress_info_t *pCompressInfo);
bool rle8_low_entropy_short_get_compress_info_only_max_frequency(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_low_entropy_compress_info_t *pCompressInfo);

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_low_entropy_short_compress_bounds(const uint32_t inSize)
{
  return inSize + (256 / 8) + 1 + 256 + sizeof(uint32_t) * 2;
}

uint32_t rle8_low_entropy_short_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_low_entropy_compress_bounds(inSize))
    return 0;

  rle8_low_entropy_compress_info_t compressInfo;

  if (!rle8_low_entropy_short_get_compress_info(pIn, inSize, &compressInfo))
    return 0;

  size_t index = sizeof(uint32_t); // to make room for the uint32_t length as the first value.

  // Store required information.
  {
    *((uint32_t *)&pOut[index]) = inSize;
    index += sizeof(uint32_t);

    const uint32_t size = rle8_low_entropy_write_compress_info(&compressInfo, &pOut[index], outSize);

    if (size == 0)
      return 0;

    index += size;
  }

  // Compress.
  {
    const uint32_t size = rle8_low_entropy_short_compress_with_info(pIn, inSize, &compressInfo, &pOut[index], outSize - (uint32_t)index);

    if (size == 0)
      return 0;

    index += size;
  }

  // Store compressed length.
  ((uint32_t *)pOut)[0] = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t rle8_low_entropy_short_compress_only_max_frequency(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_low_entropy_compress_bounds(inSize))
    return 0;

  rle8_low_entropy_compress_info_t compressInfo;

  if (!rle8_low_entropy_short_get_compress_info_only_max_frequency(pIn, inSize, &compressInfo))
    return 0;

  size_t index = sizeof(uint32_t); // to make room for the uint32_t length as the first value.

  // Store required information.
  {
    *((uint32_t *)&pOut[index]) = inSize;
    index += sizeof(uint32_t);

    const uint32_t size = rle8_low_entropy_write_compress_info(&compressInfo, &pOut[index], outSize);

    if (size == 0)
      return 0;

    index += size;
  }

  // Compress.
  {
    const uint32_t size = rle8_low_entropy_short_compress_with_info(pIn, inSize, &compressInfo, &pOut[index], outSize - (uint32_t)index);

    if (size == 0)
      return 0;

    index += size;
  }

  // Store compressed length.
  ((uint32_t *)pOut)[0] = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t rle8_low_entropy_short_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[0];
  const size_t expectedOutSize = ((uint32_t *)pIn)[1];

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  size_t index = 2 * sizeof(uint32_t);

  rle8_low_entropy_decompress_info_t decompressInfo;

  index += rle8_low_entropy_read_decompress_info(&pIn[index], inSize, &decompressInfo);

  const uint8_t *pEnd = pIn + expectedInSize;
  pIn += index;

  return rle8_low_entropy_short_decompress_with_info(pIn, pEnd, &decompressInfo, pOut, (uint32_t)expectedOutSize);
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_low_entropy_short_compress_with_info(IN const uint8_t *pIn, const uint32_t inSize, IN const rle8_low_entropy_compress_info_t *pCompressInfo, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pCompressInfo == NULL || pOut == NULL || outSize < inSize)
    return 0;

  size_t index = 0;
  size_t i = 0;
  size_t target_ = (inSize - 256);

  if (target_ > inSize)
    target_ = 0;

  const size_t target = target_;

  for (; i < target; i++)
  {
    const uint8_t b = pIn[i];

    pOut[index] = b;
    index++;

    if (pCompressInfo->rle[b])
    {
      const uint8_t range = ULTRA_MAX_BLOCK_LENGTH;
      uint8_t count = 0;

      int j = 1;

      for (; j < range; j++)
        if (pIn[j + i] == b)
          count++;
        else
          break;

      i += j - 1;

      pOut[index] = pCompressInfo->symbolsByProb[count];
      index++;
    }
  }

  for (; i < inSize; i++)
  {
    const uint8_t b = pIn[i];

    pOut[index] = b;
    index++;

    if (pCompressInfo->rle[b])
    {
      const uint8_t range = (uint8_t)min(inSize - i - 1, ULTRA_MAX_BLOCK_LENGTH);

      uint8_t count = 0;
      size_t j = 1;

      for (; j < range; j++)
        if (pIn[j + i] == b)
          count++;
        else
          break;

      i += j - 1;

      pOut[index] = pCompressInfo->symbolsByProb[count];
      index++;
    }
  }

  return (uint32_t)index;
}

//////////////////////////////////////////////////////////////////////////

const uint8_t * rle8_low_entropy_short_decompress_single_sse(IN const uint8_t *pIn, IN const uint8_t *pPreEnd, OUT uint8_t *pOut, const bool rle[256], const uint8_t symbolToCount[256], OUT uint8_t **ppOut)
{
  typedef __m128i simd_t;
#define SIMD_SIZE 16
  _STATIC_ASSERT(SIMD_SIZE == sizeof(simd_t));

  simd_t interestingSymbol;

  for (size_t i = 0; i < 256; i++)
  {
    if (rle[i])
    {
      interestingSymbol = _mm_set1_epi8((char)i);
      break;
    }
  }

  while (pIn < pPreEnd)
  {
    ALIGN(SIMD_SIZE) const simd_t data = _mm_loadu_si128((const simd_t *)pIn);
    _mm_storeu_si128((simd_t *)pOut, data);

    const int32_t contains = _mm_movemask_epi8(_mm_cmpeq_epi8(data, interestingSymbol));

    if (contains == 0)
    {
      pOut += sizeof(simd_t);
      pIn += sizeof(simd_t);
    }
    else
    {
      ALIGN(SIMD_SIZE) uint8_t dataA[sizeof(simd_t)];
      _mm_store_si128((simd_t *)dataA, data);

#ifdef _MSC_VER
      unsigned long index;
      _BitScanForward(&index, contains);
#else
      const uint32_t index = __builtin_ctz(contains);
#endif

      pIn += index + 1;
      pOut += index + 1;

      const uint8_t count = symbolToCount[*pIn];
      pIn++;

      _mm_storeu_si128((simd_t *)pOut, interestingSymbol);
      _mm_storeu_si128(((simd_t *)pOut) + 1, interestingSymbol);
      pOut += count;
    }
  }

  *ppOut = pOut;
  return pIn;
#undef SIMD_SIZE
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
const uint8_t * rle8_low_entropy_short_decompress_single_avx2(IN const uint8_t *pIn, IN const uint8_t *pPreEnd, OUT uint8_t *pOut, const bool rle[256], const uint8_t symbolToCount[256], OUT uint8_t **ppOut)
{
  typedef __m256i simd_t;
#define SIMD_SIZE 32
  _STATIC_ASSERT(SIMD_SIZE == sizeof(simd_t));

  simd_t interestingSymbol;

  for (size_t i = 0; i < 256; i++)
  {
    if (rle[i])
    {
      interestingSymbol = _mm256_set1_epi8((char)i);
      break;
    }
  }

  while (pIn < pPreEnd)
  {
    ALIGN(SIMD_SIZE) const simd_t data = _mm256_loadu_si256((const simd_t *)pIn);
    _mm256_storeu_si256((simd_t *)pOut, data);

    const int32_t contains = _mm256_movemask_epi8(_mm256_cmpeq_epi8(data, interestingSymbol));

    if (contains == 0)
    {
      pOut += sizeof(simd_t);
      pIn += sizeof(simd_t);
    }
    else
    {
      ALIGN(SIMD_SIZE) uint8_t dataA[sizeof(simd_t)];
      _mm256_store_si256((simd_t *)dataA, data);

#ifdef _MSC_VER
      unsigned long index;
      _BitScanForward(&index, contains);
#else
      const uint32_t index = __builtin_ctz(contains);
#endif

      pIn += index + 1;
      pOut += index + 1;

      const uint8_t count = symbolToCount[*pIn];
      pIn++;

      _mm256_storeu_si256((simd_t *)pOut, interestingSymbol);
      pOut += count;
    }
  }

  *ppOut = pOut;
  return pIn;
#undef SIMD_SIZE
}

const uint8_t * rle8_low_entropy_short_decompress_multi_sse(IN const uint8_t *pIn, IN const uint8_t *pPreEnd, OUT uint8_t *pOut, const bool rle[256], const uint8_t symbolToCount[256], OUT uint8_t **ppOut)
{
  while (pIn < pPreEnd)
  {
    typedef __m128i simd_t;
#define SIMD_SIZE 16

    _STATIC_ASSERT(SIMD_SIZE == sizeof(simd_t));

    ALIGN(SIMD_SIZE) const simd_t data = _mm_loadu_si128((const simd_t *)pIn);
    _mm_storeu_si128((simd_t *)pOut, data);

    ALIGN(SIMD_SIZE) uint8_t dataA[sizeof(simd_t)];
    _mm_store_si128((simd_t *)dataA, data);

    for (size_t i = 0; i < sizeof(simd_t); i++)
    {
      if (rle[dataA[i]])
      {
        pIn += i + 1;
        pOut += i + 1;

        const uint8_t count = symbolToCount[*pIn];
        pIn++;

        if (count)
        {
          const simd_t bb = _mm_set1_epi8((char)dataA[i]);

          _mm_storeu_si128((simd_t *)pOut, bb);
          _mm_storeu_si128(((simd_t *)pOut) + 1, bb);
          pOut += count;
        }

        goto symbol_found;
      }
    }

    pIn += sizeof(simd_t);
    pOut += sizeof(simd_t);

  symbol_found:
    ;
  }

  *ppOut = pOut;
  return pIn;
#undef SIMD_SIZE
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#else
__declspec(noinline)
#endif
const uint8_t * rle8_low_entropy_short_decompress_multi_avx(IN const uint8_t *pIn, IN const uint8_t *pPreEnd, OUT uint8_t *pOut, const bool rle[256], const uint8_t symbolToCount[256], OUT uint8_t **ppOut)
{
  while (pIn < pPreEnd)
  {
    typedef __m256i simd_t;
#define SIMD_SIZE 32

    _STATIC_ASSERT(SIMD_SIZE == sizeof(simd_t));

    ALIGN(SIMD_SIZE) const simd_t data = _mm256_loadu_si256((const simd_t *)pIn);
    _mm256_storeu_si256((simd_t *)pOut, data);

    ALIGN(SIMD_SIZE) uint8_t dataA[sizeof(simd_t)];

    _mm256_store_si256((simd_t *)dataA, data);

    for (size_t i = 0; i < sizeof(simd_t); i++)
    {
      if (rle[dataA[i]])
      {
        pIn += i + 1;
        pOut += i + 1;

        const uint8_t count = symbolToCount[*pIn];
        pIn++;

#if !defined(_DEBUG) && defined(_MSC_VER) && _MSC_VER <= 1900
        const simd_t bb = _mm256_set1_epi16(dataA[i] | (dataA[i] << 8));
#else
        const simd_t bb = _mm256_set1_epi8((char)dataA[i]);
#endif
        _mm256_storeu_si256((simd_t *)pOut, bb);
        pOut += count;

        goto symbol_found;
      }
    }

    pIn += sizeof(simd_t);
    pOut += sizeof(simd_t);

  symbol_found:
    ;
  }

  *ppOut = pOut;
  return pIn;
#undef SIMD_SIZE
}

//////////////////////////////////////////////////////////////////////////

extern bool sseSupported;
extern bool sse2Supported;
extern bool sse3Supported;
extern bool ssse3Supported;
extern bool sse41Supported;
extern bool sse42Supported;
extern bool avxSupported;
extern bool avx2Supported;
extern bool fma3Supported;

void _DetectCPUFeatures();

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_low_entropy_short_decompress_with_info(IN const uint8_t *pIn, IN const uint8_t *pEnd, IN const rle8_low_entropy_decompress_info_t *pDecompressInfo, OUT uint8_t *pOut, const uint32_t expectedOutSize)
{
  bool rle[256];
  uint8_t symbolToCount[256];

  memcpy(rle, pDecompressInfo->rle, sizeof(rle));
  memcpy(symbolToCount, pDecompressInfo->symbolToCount, sizeof(symbolToCount));

  const uint8_t *pPreEnd = pEnd - 256;

  uint8_t rleSymbolCount = 0;

  for (size_t i = 0; i < 256; i++)
    rleSymbolCount += rle[i];

  if (rleSymbolCount == 0 /* || expectedOutSize == (uint32_t)(pEnd - pIn)*/)
  {
    memcpy(pOut, pIn, expectedOutSize);
    return (uint32_t)expectedOutSize;
  }
  else if (rleSymbolCount == 1)
  {
    _DetectCPUFeatures();

    if (avx2Supported)
      pIn = rle8_low_entropy_short_decompress_single_avx2(pIn, pPreEnd, pOut, rle, symbolToCount, &pOut);
    else
      pIn = rle8_low_entropy_short_decompress_single_sse(pIn, pPreEnd, pOut, rle, symbolToCount, &pOut);
  }
  else
  {
    _DetectCPUFeatures();

    if (avxSupported)
      pIn = rle8_low_entropy_short_decompress_multi_avx(pIn, pPreEnd, pOut, rle, symbolToCount, &pOut);
    else
      pIn = rle8_low_entropy_short_decompress_multi_sse(pIn, pPreEnd, pOut, rle, symbolToCount, &pOut);
  }

  while (pIn < pEnd)
  {
    const uint8_t b = *pOut = *pIn;
    pIn++;
    pOut++;

    if (rle[b])
    {
      const uint8_t count = symbolToCount[*pIn];
      pIn++;

      if (count)
      {
        if (count < 16)
        {
          for (size_t i = 0; i < count; i++)
          {
            *pOut = b;
            pOut++;
          }
        }
        else
        {
          size_t countRemaining = count;
          size_t unaligned = ((size_t)pOut & (sizeof(__m128i) - 1));
          const __m128i bb = _mm_set1_epi8((char)b);

          if (unaligned != 0)
          {
            _mm_storeu_si128((__m128i *)pOut, bb);
            uint8_t *pPrevOut = pOut;
            pOut = (uint8_t *)((size_t)pOut & ~(size_t)(sizeof(__m128i) - 1)) + sizeof(__m128i);
            countRemaining -= (pOut - pPrevOut);
          }

          while (countRemaining > (sizeof(__m128i) - 1))
          {
            _mm_store_si128((__m128i *)pOut, bb);
            pOut += sizeof(__m128i);
            countRemaining -= sizeof(__m128i);
          }

          if (countRemaining != 0)
          {
            _mm_storeu_si128((__m128i *)(pOut - (sizeof(__m128i) - countRemaining)), bb);
            pOut += countRemaining;
          }
        }
      }
    }
  }

  return (uint32_t)expectedOutSize;
}

//////////////////////////////////////////////////////////////////////////

bool rle8_low_entropy_short_get_compress_info(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_low_entropy_compress_info_t *pCompressInfo)
{
  if (pIn == NULL || inSize == 0 || pCompressInfo == NULL)
    return false;

  uint8_t symbolsByProb[256];
  bool rle[256];

  uint32_t remaining = 256;

  // Get Probabilities.
  {
    uint32_t prob[256];
    uint32_t pcount[256];
    bool consumed[256];

    memset(prob, 0, sizeof(prob));
    memset(pcount, 0, sizeof(pcount));
    memset(consumed, 0, sizeof(consumed));

    uint8_t lastSymbol = 0;
    uint32_t count = 0;

    if (pIn[0] != lastSymbol)
      pcount[lastSymbol] = (uint32_t)-1;

    for (size_t i = 0; i < inSize; i++)
    {
      if (pIn[i] == lastSymbol)
      {
        count++;
      }
      else
      {
        prob[lastSymbol] += count;
        pcount[lastSymbol] += (count / ULTRA_MAX_BLOCK_LENGTH) + 1;
        count = 1;
        lastSymbol = pIn[i];
      }
    }

    prob[lastSymbol] += count;
    pcount[lastSymbol]++;

    for (size_t i = 0; i < 256; i++)
      if (pcount[i] > 0)
        rle[i] = (prob[i] / pcount[i]) >= 2;
      else
        rle[i] = false;

    for (int64_t i = 255; i >= 0; i--)
    {
      if (pcount[i] == 0)
      {
        consumed[i] = true;
        remaining--;
        symbolsByProb[remaining] = (uint8_t)i;
      }
    }

    for (size_t index = 0; index < remaining; index++)
    {
      uint32_t max = 0;
      size_t maxIndex = 0;

      for (size_t i = 0; i < 256; i++)
      {
        if (!consumed[i] && pcount[i] > max)
        {
          max = pcount[i];
          maxIndex = i;
        }
      }

      symbolsByProb[index] = (uint8_t)maxIndex;
      consumed[maxIndex] = true;
    }
  }

  pCompressInfo->symbolCount = (uint8_t)remaining;
  memcpy(pCompressInfo->symbolsByProb, symbolsByProb, sizeof(symbolsByProb));
  memcpy(pCompressInfo->rle, rle, sizeof(rle));

  return true;
}

bool rle8_low_entropy_short_get_compress_info_only_max_frequency(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_low_entropy_compress_info_t *pCompressInfo)
{
  if (pIn == NULL || inSize == 0 || pCompressInfo == NULL)
    return false;

  uint8_t symbolsByProb[256];
  bool rle[256];
  memset(rle, 0, sizeof(rle));

  uint32_t remaining = 256;

  // Get Probabilities.
  {
    uint32_t prob[256];
    uint32_t pcount[256];
    bool consumed[256];

    memset(prob, 0, sizeof(prob));
    memset(pcount, 0, sizeof(pcount));
    memset(consumed, 0, sizeof(consumed));

    uint8_t lastSymbol = 0;
    uint32_t count = 0;

    if (pIn[0] != lastSymbol)
      pcount[lastSymbol] = (uint32_t)-1;

    for (size_t i = 0; i < inSize; i++)
    {
      if (pIn[i] == lastSymbol)
      {
        count++;
      }
      else
      {
        prob[lastSymbol] += count;
        pcount[lastSymbol] += (count / ULTRA_MAX_BLOCK_LENGTH) + 1;
        count = 1;
        lastSymbol = pIn[i];
      }
    }

    prob[lastSymbol] += count;
    pcount[lastSymbol]++;

    size_t maxBytesSaved = 0;
    size_t maxBytesSavedIndexIndex = 0;

    for (size_t i = 0; i < 256; i++)
    {
      if (pcount[i] > 0 && prob[i] / pcount[i] > 2)
      {
        const size_t saved = prob[i] - (pcount[i] * 2);

        if (saved > maxBytesSaved)
        {
          maxBytesSaved = saved;
          maxBytesSavedIndexIndex = i;
        }
      }
    }

    if (maxBytesSaved > 0)
      rle[maxBytesSavedIndexIndex] = true;

    for (int64_t i = 255; i >= 0; i--)
    {
      if (pcount[i] == 0)
      {
        consumed[i] = true;
        remaining--;
        symbolsByProb[remaining] = (uint8_t)i;
      }
    }

    for (size_t index = 0; index < remaining; index++)
    {
      uint32_t max = 0;
      size_t maxIndex = 0;

      for (size_t i = 0; i < 256; i++)
      {
        if (!consumed[i] && pcount[i] > max)
        {
          max = pcount[i];
          maxIndex = i;
        }
      }

      symbolsByProb[index] = (uint8_t)maxIndex;
      consumed[maxIndex] = true;
    }
  }

  pCompressInfo->symbolCount = (uint8_t)remaining;
  memcpy(pCompressInfo->symbolsByProb, symbolsByProb, sizeof(symbolsByProb));
  memcpy(pCompressInfo->rle, rle, sizeof(rle));

  return true;
}