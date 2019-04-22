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
  pIn += index;

  while (pIn < pEnd)
  {
    const uint8_t b = *pOut = *pIn;
    pIn++;
    pOut++;

    if (rle[b])
    {
      const uint8_t count = symbolToCount[*pIn];
      pIn++;

      //for (size_t i = 0; i < count; i++)
      //{
      //  *pOut = b;
      //  pOut++;
      //}
      
      switch (count)
      {
      case 0:
        break;

      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      {
        for (size_t i = 0; i < count; i++)
        {
          *pOut = b;
          pOut++;
        }

        break;
      }

      default:
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
