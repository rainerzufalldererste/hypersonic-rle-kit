#ifndef PACKED
  #define RLE48_EXTREME_MULTI_MIN_RANGE_SHORT ((6 + 1 + 1) + 2)
  #define RLE48_EXTREME_MULTI_MIN_RANGE_LONG ((6 + 1 + 4 + 1 + 4) + 2)
#else
  #define RLE48_EXTREME_MULTI_MIN_RANGE_SHORT ((1 + 1) + 1)
  #define RLE48_EXTREME_MULTI_MIN_RANGE_MEDIUM ((6 + 1 + 1) + 1)
  #define RLE48_EXTREME_MULTI_MIN_RANGE_LONG ((6 + 1 + 4 + 4) + 1)
#endif

#ifndef UNBOUND
  #define CODEC extreme
#else
  #ifdef PACKED
    #define CODEC extreme_packed
  #else
    #define CODEC extreme_unbound
  #endif
#endif

uint32_t CONCAT3(rle48_, CODEC, _compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_extreme_compress_bounds(inSize))
    return 0;

  size_t index = 0;

  *(uint32_t *)(&(pOut[index])) = inSize;
  index += sizeof(uint32_t);
  *(uint32_t *)(&(pOut[index])) = 0; // will be compressed size.
  index += sizeof(uint32_t);
  int64_t i = 0;
  int64_t lastRLE = 0;

  typedef uint64_t symbol_t;
  const symbol_t symbolMask = 0x0000FFFFFFFFFFFF;
  const size_t symbolSize = 6;

  int64_t count = 0;
  symbol_t symbol = (~(*((symbol_t *)(pIn)))) & symbolMask;

  while (i < inSize)
  {
    if (count && ((*(symbol_t *)&pIn[i]) & symbolMask) == symbol && i + symbolSize <= inSize)
    {
      count += symbolSize;
      i += symbolSize;
    }
    else
    {
      {
        const int64_t range = i - lastRLE - count + 1;

        if (range <= 255 && count >= RLE48_EXTREME_MULTI_MIN_RANGE_SHORT)
        {
          *(symbol_t *)(&pOut[index]) = symbol;
          index += symbolSize;

          const int64_t storedCount = (count / symbolSize) - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;

          if (storedCount <= 255)
          {
            pOut[index] = (uint8_t)storedCount;
            index++;
          }
          else
          {
            pOut[index] = 0;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          pOut[index] = (uint8_t)range;
          index++;

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
        else if (count >= RLE48_EXTREME_MULTI_MIN_RANGE_LONG)
        {
          *(symbol_t *)(&pOut[index]) = symbol;
          index += symbolSize;

          const int64_t storedCount = (count / symbolSize) - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;

          if (storedCount <= 255)
          {
            pOut[index] = (uint8_t)storedCount;
            index++;
          }
          else
          {
            pOut[index] = 0;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          pOut[index] = 0;
          index++;
          *((uint32_t *)&pOut[index]) = (uint32_t)range;
          index += sizeof(uint32_t);

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
      }

      symbol = (*(symbol_t *)(&pIn[i])) & symbolMask;

      if (i + symbolSize <= inSize && ((*(symbol_t *)((&pIn[i]) + symbolSize)) & symbolMask) == symbol)
      {
        count = symbolSize * 2;
        i += symbolSize * 2;
      }
      else
      {
        count = 0;
        i++;
      }
    }
  }

  {
    const int64_t range = i - lastRLE - count + 1;

    if (range <= 255 && count >= RLE48_EXTREME_MULTI_MIN_RANGE_SHORT)
    {
      *(symbol_t *)(&pOut[index]) = symbol;
      index += symbolSize;

      const int64_t storedCount = (count / symbolSize) - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;

      if (storedCount <= 255)
      {
        pOut[index] = (uint8_t)storedCount;
        index++;
      }
      else
      {
        pOut[index] = 0;
        index++;
        *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }

      pOut[index] = (uint8_t)range;
      index++;

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

      *(symbol_t *)(&pOut[index]) = 0;
      index += symbolSize;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);

      lastRLE = i;
    }
    else if (count >= RLE48_EXTREME_MULTI_MIN_RANGE_LONG)
    {
      *(symbol_t *)(&pOut[index]) = symbol;
      index += symbolSize;

      const int64_t storedCount = (count / symbolSize) - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;

      if (storedCount <= 255)
      {
        pOut[index] = (uint8_t)storedCount;
        index++;
      }
      else
      {
        pOut[index] = 0;
        index++;
        *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }

      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

      *(symbol_t *)(&pOut[index]) = 0;
      index += symbolSize;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);

      lastRLE = i;
    }
    else
    {
      *(symbol_t *)(&pOut[index]) = 0;
      index += symbolSize;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);

      const size_t copySize = i - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;
    }
  }

  // Store compressed length.
  ((uint32_t *)pOut)[1] = (uint32_t)index;

  return (uint32_t)index;
}

static void CONCAT3(rle48_, CODEC, _decompress_sse2)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol;
  
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

  while (true)
  {
    symbol = _mm_set1_epi64x(*(symbol_t *)pInStart);

    pInStart += symbolSize;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

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

    // memcpy.
    MEMCPY_SSE_MULTI;

    if (!symbolCount)
      return;

    symbolCount = (symbolCount + (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) - 1) * symbolSize;

    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

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
  __m128i symbol;

  const __m128i shuffle0 = _mm_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
  const __m128i shuffle1 = _mm_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4);
  const __m128i shuffle2 = _mm_set_epi8(5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2);

  typedef uint64_t symbol_t;
  const size_t symbolSize = 6;

  while (true)
  {
    symbol = _mm_set1_epi64x(*(symbol_t *)pInStart);

    pInStart += symbolSize;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

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

    // memcpy.
    MEMCPY_SSE_MULTI;

    if (!symbolCount)
      return;

    symbolCount = (symbolCount + (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) - 1) * symbolSize;

    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

      const __m128i symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      const __m128i symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      const __m128i symbol2 = _mm_shuffle_epi8(symbol, shuffle2);

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
  __m256i symbol;

  const __m256i shuffle0 = _mm256_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
  const __m256i shuffle1 = _mm256_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2);
  const __m256i shuffle2 = _mm256_set_epi8(5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4);

  typedef uint64_t symbol_t;
  const size_t symbolSize = 6;

  while (true)
  {
    symbol = _mm256_set1_epi64x(*(symbol_t *)pInStart);

    pInStart += symbolSize;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

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

    // memcpy.
    MEMCPY_AVX_MULTI;

    if (!symbolCount)
      return;

    symbolCount = (symbolCount + (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) - 1) * symbolSize;

    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

      const __m256i symbol0 = _mm256_shuffle_epi8(symbol, shuffle0);
      const __m256i symbol1 = _mm256_shuffle_epi8(symbol, shuffle1);
      const __m256i symbol2 = _mm256_shuffle_epi8(symbol, shuffle2);

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

#undef RLE48_EXTREME_MULTI_MIN_RANGE_SHORT
#undef RLE48_EXTREME_MULTI_MIN_RANGE_LONG

#undef CODEC
