#include "rle8.h"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define AVX 1
// #define AVX2 1

#ifdef _MSC_VER
//#define DO_NOT_OPTIMIZE_DECODER
#define ALIGN(a) __declspec(align(a))
#else
#define ALIGN(a) __attribute__((aligned(a)))
#define _STATIC_ASSERT(expr) typedef char __static_assert_t[(expr) != 0]
#endif

#define min(a, b) ((a < b) ? (a) : (b))

uint32_t rle8_compress_bounds(const uint32_t inSize)
{
  return inSize + (256 / 8) + 1 + 256 + sizeof(uint32_t) * 2;
}

uint32_t rle8_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_compress_bounds(inSize))
    return 0;

  rle8_compress_info_t compressInfo;

  if (!rle8_get_compress_info(pIn, inSize, &compressInfo))
    return 0;

  size_t index = sizeof(uint32_t); // to make room for the uint32_t length as the first value.

  // Store required information.
  {
    *((uint32_t *)&pOut[index]) = inSize;
    index += sizeof(uint32_t);

    const uint32_t size = rle8_write_compress_info(&compressInfo, &pOut[index], outSize);

    if (size == 0)
      return 0;

    index += size;
  }

  // Compress.
  {
    const uint32_t size = rle8_compress_with_info(pIn, inSize, &compressInfo, &pOut[index], outSize - (uint32_t)index);

    if (size == 0)
      return 0;

    index += size;
  }
  
  // Store compressed length.
  ((uint32_t *)pOut)[0] = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t rle8_decompressed_size(IN const uint8_t *pIn, const uint32_t inSize)
{
  if (pIn == NULL || inSize < sizeof(uint32_t) * 2)
    return 0;

  return ((uint32_t *)pIn)[1];
}

uint32_t rle8_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[0];
  const size_t expectedOutSize = ((uint32_t *)pIn)[1];

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  size_t index = 2 * sizeof(uint32_t);
  
  rle8_decompress_info_t decompressInfo;

  index += rle8_read_decompress_info(&pIn[index], inSize, &decompressInfo);

  const uint8_t *pEnd = pIn + expectedInSize;
  pIn += index;

  return rle8_decompress_with_info(pIn, pEnd, &decompressInfo, pOut, (uint32_t)expectedOutSize);
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle8m_compress_bounds(const uint32_t subSections, const uint32_t inSize)
{
  return inSize + (256 / 8) + 1 + 256 + sizeof(uint32_t) * (2 + subSections - 1 + 1);
}

uint32_t rle8m_compress(const uint32_t subSections, IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8m_compress_bounds(subSections, inSize) || subSections == 0)
    return 0;

  rle8_compress_info_t compressInfo;

  if (!rle8_get_compress_info(pIn, inSize, &compressInfo))
    return 0;

  size_t index = sizeof(uint32_t); // Expected Input Size for the Decoder.

  *((uint32_t *)(&pOut[index])) = inSize;
  index += sizeof(uint32_t);

  *((uint32_t *)(&pOut[index])) = subSections;
  index += sizeof(uint32_t);

  const size_t subSectionIndex = index;
  index += sizeof(uint32_t) * (subSections - 1);

  // Write Info.
  {
    const uint32_t size = rle8_write_compress_info(&compressInfo, &pOut[index], outSize);

    if (size == 0)
      return 0;

    index += size;
  }

  const uint32_t subSectionSize = inSize / subSections;

  for (uint32_t i = 0; i < subSections - 1; i++)
  {
    const uint32_t size = rle8_compress_with_info(pIn + subSectionSize * i, subSectionSize, &compressInfo, pOut + index, (uint32_t)(outSize - index));

    if (size == 0)
      return 0;

    index += size;

    ((uint32_t *)(&pOut[subSectionIndex]))[i] = index;
  }

  const size_t remainingSize = inSize - subSectionSize * (subSections - 1);

  // Compress last block.
  {
    const uint32_t size = rle8_compress_with_info(pIn + subSectionSize * (subSections - 1), (uint32_t)remainingSize, &compressInfo, pOut + index, (uint32_t)(outSize - index));

    if (size == 0)
      return 0;

    index += size;
  }

  ((uint32_t *)pOut)[0] = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t rle8m_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[0];
  const size_t expectedOutSize = ((uint32_t *)pIn)[1];

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  size_t index = 2 * sizeof(uint32_t);

  const uint32_t subSections = *((uint32_t *)(&pIn[index]));
  index += sizeof(uint32_t);

  if (subSections == 0)
    return 0;

  const size_t subSectionIndex = index;
  index += (subSections - 1) * sizeof(uint32_t);

  rle8_decompress_info_t decompressInfo;

  index += rle8_read_decompress_info(&pIn[index], inSize, &decompressInfo);

  const uint32_t subSectionSize = (uint32_t)(expectedOutSize / subSections);

  const uint8_t *pCIn = pIn + index;

  for (size_t i = 0; i < subSections - 1; i++)
  {
    const uint32_t sectionCompressedOffset = ((uint32_t *)(&pIn[subSectionIndex]))[i];
    const uint8_t *pEnd = pIn + sectionCompressedOffset;

    const uint32_t size = rle8_decompress_with_info(pCIn, pEnd, &decompressInfo, pOut, subSectionSize);

    if (size != subSectionSize)
      return 0;

    pCIn = pEnd;
    pOut += subSectionSize;
  }

  const size_t remainingSize = expectedOutSize - subSectionSize * (subSections - 1);

  // Decode last section.
  {
    const uint8_t *pEnd = pIn + inSize;

    const uint32_t size = rle8_decompress_with_info(pCIn, pEnd, &decompressInfo, pOut, (uint32_t)remainingSize);

    if (size != remainingSize)
      return 0;
  }

  return (uint32_t)expectedOutSize;
}

//////////////////////////////////////////////////////////////////////////

bool rle8_get_compress_info(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_compress_info_t *pCompressInfo)
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
        pcount[lastSymbol]++;
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

uint32_t rle8_write_compress_info(IN rle8_compress_info_t *pCompressInfo, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pCompressInfo == NULL || pOut == NULL || outSize < 256 / 8 + 256 + 1)
    return 0;

  uint32_t index = 0;

  for (size_t i = 0; i < 256 / 8; i++)
  {
    pOut[index] = 0;

    for (size_t j = 0; j < 8; j++)
      pOut[index] |= (((uint8_t)(!!pCompressInfo->rle[j + i * 8])) << j);

    index++;
  }

  pOut[index] = pCompressInfo->symbolCount;
  index++;

  for (size_t i = 0; i < pCompressInfo->symbolCount; i++)
    pOut[index + i] = pCompressInfo->symbolsByProb[i];

  index += pCompressInfo->symbolCount;

  return index;
}

uint32_t rle8_compress_with_info(IN const uint8_t *pIn, const uint32_t inSize, IN const rle8_compress_info_t *pCompressInfo, OUT uint8_t *pOut, const uint32_t outSize)
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
      const uint8_t range = 255;
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
      const uint8_t range = (uint8_t)min(inSize - i - 1, 255);

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

uint32_t rle8_read_decompress_info(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_decompress_info_t *pDecompressInfo)
{
  if (pIn == NULL || pDecompressInfo == NULL || inSize == 0)
    return 0;

  size_t index = 0;

  bool rle[256];

  for (size_t i = 0; i < 256 / 8; i++)
  {
    rle[i * 8 + 0] = !!(pIn[index] & 0b1);
    rle[i * 8 + 1] = !!(pIn[index] & 0b10);
    rle[i * 8 + 2] = !!(pIn[index] & 0b100);
    rle[i * 8 + 3] = !!(pIn[index] & 0b1000);
    rle[i * 8 + 4] = !!(pIn[index] & 0b10000);
    rle[i * 8 + 5] = !!(pIn[index] & 0b100000);
    rle[i * 8 + 6] = !!(pIn[index] & 0b1000000);
    rle[i * 8 + 7] = !!(pIn[index] & 0b10000000);

    index++;
  }

  uint8_t symbolToCount[256];

  {
    uint8_t symbolsByProb[256];

    const uint8_t symbolsWithProb = pIn[index];
    index++;

    for (uint8_t i = 0; i < symbolsWithProb; i++)
    {
      symbolsByProb[i] = pIn[index];
      index++;
      symbolToCount[symbolsByProb[i]] = i;
    }

    uint8_t nextSymbol = symbolsWithProb;

    for (size_t i = 0; i < 256; i++)
    {
      for (size_t j = 0; j < symbolsWithProb; j++)
        if (symbolsByProb[j] == i)
          goto next_symbol;

      symbolToCount[i] = nextSymbol;
      nextSymbol++;

    next_symbol:
      ;
    }
  }

  memcpy(pDecompressInfo->rle, rle, sizeof(rle));
  memcpy(pDecompressInfo->symbolToCount, symbolToCount, sizeof(symbolToCount));

  return (uint32_t)index;
}

#ifdef DO_NOT_OPTIMIZE_DECODER
#pragma optimize("", off)
#endif
#ifndef _MSC_VER
#if AVX2
__attribute__((target("avx2")))
#elif AVX
__attribute__((target("avx")))
#endif
#endif
uint32_t rle8_decompress_with_info(IN const uint8_t *pIn, IN const uint8_t *pEnd, IN const rle8_decompress_info_t *pDecompressInfo, OUT uint8_t *pOut, const uint32_t expectedOutSize)
{
  bool rle[256];
  uint8_t symbolToCount[256];

  memcpy(rle, pDecompressInfo->rle, sizeof(rle));
  memcpy(symbolToCount, pDecompressInfo->symbolToCount, sizeof(symbolToCount));

  const uint8_t *pPreEnd = pEnd - 256;

  uint8_t rleSymbolCount = 0;

  for (size_t i = 0; i < 256; i++)
    rleSymbolCount += rle[i];

  if (rleSymbolCount == 0 || expectedOutSize == (uint32_t)(pEnd - pIn))
  {
    memcpy(pOut, pIn, expectedOutSize);
    return (uint32_t)expectedOutSize;
  }
  else if (rleSymbolCount == 1)
  {
#ifndef AVX2
    typedef __m128i simd_t;
#define SIMD_SIZE 16
#else
    typedef __m256i simd_t;
#define SIMD_SIZE 32
#endif

    _STATIC_ASSERT(SIMD_SIZE == sizeof(simd_t));

    simd_t interestingSymbol;

    for (size_t i = 0; i < 256; i++)
    {
      if (rle[i])
      {
#ifndef AVX2
        interestingSymbol = _mm_set1_epi8((char)i);
#else
        interestingSymbol = _mm256_set1_epi8((char)i);
#endif
        break;
      }
    }

    while (pIn < pPreEnd)
    {
      ALIGN(SIMD_SIZE) const simd_t data =
#ifndef AVX2
        _mm_loadu_si128((const simd_t *)pIn);
      _mm_storeu_si128((simd_t *)pOut, data);
#else
        _mm256_loadu_si256((const simd_t *)pIn);
      _mm256_storeu_si256((simd_t *)pOut, data);
#endif

      const int32_t contains =
#ifndef AVX2
        _mm_movemask_epi8(_mm_cmpeq_epi8(data, interestingSymbol));
#else
        _mm256_movemask_epi8(_mm256_cmpeq_epi8(data, interestingSymbol));
#endif

      if (contains == 0)
      {
        pOut += sizeof(simd_t);
        pIn += sizeof(simd_t);
      }
      else
      {
        ALIGN(SIMD_SIZE) uint8_t dataA[sizeof(simd_t)];
#ifndef AVX2
        _mm_store_si128((simd_t *)dataA, data);
#else
        _mm256_store_si256((simd_t *)dataA, data);
#endif

        for (size_t i = 0; i < sizeof(__m128i); i++)
        {
          if (rle[dataA[i]])
          {
            pIn += i + 1;
            pOut += i + 1;

            const uint8_t count = symbolToCount[*pIn];
            pIn++;

            if (count)
            {
#ifndef AVX2
              if (count <= sizeof(__m128i))
              {
                _mm_storeu_si128((__m128i *)pOut, interestingSymbol);
                pOut += count;
              }
              else
              {
                size_t unaligned = ((size_t)pOut & (sizeof(__m128i) - 1));
                const uint8_t *pCOut = pOut;

                if (unaligned != 0)
                {
                  _mm_storeu_si128((__m128i *)pCOut, interestingSymbol);
                  pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m128i) - 1)) + sizeof(__m128i);
                }

                pOut += count;

                while (pCOut < pOut)
                {
                  _mm_store_si128((__m128i *)pCOut, interestingSymbol);
                  pCOut += sizeof(__m128i);
                }
              }
#else
              if (count <= sizeof(__m256i))
              {
                _mm256_storeu_si256((__m256i *)pOut, interestingSymbol);
                pOut += count;
              }
              else
              {
                size_t unaligned = ((size_t)pOut & (sizeof(__m256i) - 1));
                const uint8_t *pCOut = pOut;

                if (unaligned != 0)
                {
                  _mm256_storeu_si256((__m256i *)pCOut, interestingSymbol);
                  pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m256i) - 1)) + sizeof(__m256i);
                }

                pOut += count;

                while (pCOut < pOut)
                {
                  _mm256_store_si256((__m256i *)pCOut, interestingSymbol);
                  pCOut += sizeof(__m256i);
                }
              }
#endif
            }

            break;
          }
        }
      }

#undef SIMD_SIZE
    }
  }
  else
  {
    while (pIn < pPreEnd)
    {
#ifndef AVX
      typedef __m128i simd_t;
#define SIMD_SIZE 16
#else
      typedef __m256i simd_t;
#define SIMD_SIZE 32
#endif

      _STATIC_ASSERT(SIMD_SIZE == sizeof(simd_t));

      ALIGN(SIMD_SIZE) const simd_t data =
#ifndef AVX
        _mm_loadu_si128((const simd_t *)pIn);
      _mm_storeu_si128((simd_t *)pOut, data);
#else
        _mm256_loadu_si256((const simd_t *)pIn);
      _mm256_storeu_si256((simd_t *)pOut, data);
#endif

      ALIGN(SIMD_SIZE) uint8_t dataA[sizeof(simd_t)];

#ifndef AVX
      _mm_store_si128((simd_t *)dataA, data);
#else
      _mm256_store_si256((simd_t *)dataA, data);
#endif

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
#ifndef AVX
            const __m128i bb = _mm_set1_epi8((char)dataA[i]);

            if (count <= sizeof(__m128i))
            {
              _mm_storeu_si128((__m128i *)pOut, bb);
              pOut += count;
            }
            else
            {
              size_t unaligned = ((size_t)pOut & (sizeof(__m128i) - 1));
              const uint8_t *pCOut = pOut;

              if (unaligned != 0)
              {
                _mm_storeu_si128((__m128i *)pCOut, bb);
                pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m128i) - 1)) + sizeof(__m128i);
              }

              pOut += count;

              while (pCOut < pOut)
              {
                _mm_store_si128((__m128i *)pCOut, bb);
                pCOut += sizeof(__m128i);
              }
            }
#else
            const __m256i bb = _mm256_set1_epi8((char)dataA[i]);

            if (count <= sizeof(__m256i))
            {
              _mm256_storeu_si256((__m256i *)pOut, bb);
              pOut += count;
            }
            else
            {
              size_t unaligned = ((size_t)pOut & (sizeof(__m256i) - 1));
              const uint8_t *pCOut = pOut;

              if (unaligned != 0)
              {
                _mm256_storeu_si256((__m256i *)pCOut, bb);
                pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m256i) - 1)) + sizeof(__m256i);
              }

              pOut += count;

              while (pCOut < pOut)
              {
                _mm256_store_si256((__m256i *)pCOut, bb);
                pCOut += sizeof(__m256i);
              }
            }
#endif
          }

          goto symbol_found;
        }
      }

      pIn += sizeof(simd_t);
      pOut += sizeof(simd_t);

    symbol_found:
      ;

#undef SIMD_SIZE
    }
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
#ifdef DO_NOT_OPTIMIZE_DECODER
#pragma optimize("", on)
#endif