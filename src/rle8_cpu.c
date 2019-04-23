#include "rle8.h"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
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

  size_t index = sizeof(uint32_t); // to make room for the uint32_t length as the first value.

  // Store required information.
  {
    *((uint32_t *)&pOut[index]) = inSize;
    index += sizeof(uint32_t);

    for (size_t i = 0; i < 256 / 8; i++)
    {
      pOut[index] = 0;

      for (size_t j = 0; j < 8; j++)
        pOut[index] |= (((uint8_t)(!!rle[j + i * 8])) << j);

      index++;
    }

    pOut[index] = (uint8_t)remaining;
    index++;

    for (size_t i = 0; i < remaining; i++)
      pOut[index + i] = symbolsByProb[i];

    index += remaining;
  }

  // Compress.
  {
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

      if (rle[b])
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

        pOut[index] = symbolsByProb[count];
        index++;
      }
    }

    for (; i < inSize; i++)
    {
      const uint8_t b = pIn[i];

      pOut[index] = b;
      index++;

      if (rle[b])
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

        pOut[index] = symbolsByProb[count];
        index++;
      }
    }
  }
  
  // Store compressed length.
  ((uint32_t *)pOut)[0] = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t rle8_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || inSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[0];
  const size_t expectedOutSize = ((uint32_t *)pIn)[1];

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  size_t index = 2 * sizeof(uint32_t);
  
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

  const uint8_t *pEnd = pIn + expectedInSize;
  const uint8_t *pPreEnd = pEnd - 256;
  pIn += index;

  uint8_t rleSymbolCount = 0;

  for (size_t i = 0; i < 256; i++)
    rleSymbolCount += rle[i];

#define AVX 1

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

      __declspec(align(SIMD_SIZE)) const simd_t data =
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
        __declspec(align(SIMD_SIZE)) uint8_t dataA[sizeof(simd_t)];
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

      __declspec(align(SIMD_SIZE)) const simd_t data =
#ifndef AVX
        _mm_loadu_si128((const simd_t *)pIn);
      _mm_storeu_si128((simd_t *)pOut, data);
#else
        _mm256_loadu_si256((const simd_t *)pIn);
      _mm256_storeu_si256((simd_t *)pOut, data);
#endif

      __declspec(align(SIMD_SIZE)) uint8_t dataA[sizeof(simd_t)];

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
