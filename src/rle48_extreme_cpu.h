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

uint32_t CONCAT3(rle48_, CODEC, _compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
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

#ifdef PACKED
  symbol_t lastSymbol = 0;
#endif

  while (i < inSize)
  {
    if (count)
    {
      if (i + symbolSize <= inSize)
      {
        const symbol_t next = ((*(symbol_t *)&pIn[i]) & symbolMask);

        if (next == symbol)
        {
          count += symbolSize;
          i += symbolSize;
          continue;
        }
#ifdef UNBOUND
        else
        {
          const symbol_t diff = symbol ^ *(symbol_t *)&pIn[i];

#ifdef _MSC_VER
          unsigned long offset;
          _BitScanForward64(&offset, diff);
#else
          const uint32_t offset = __builtin_ctzl(diff);
#endif

          i += (offset / 8);
          count += (offset / 8);
        }
#endif
      }
    }

    {
      {
        const int64_t range = i - lastRLE - count + 1;

#ifndef PACKED
        if (range <= RLE48_EXTREME_MAX_COPY_RANGE && count >= RLE48_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
        if (range <= RLE48_EXTREME_MAX_COPY_RANGE && ((count >= RLE48_EXTREME_MULTI_MIN_RANGE_SHORT && symbol == lastSymbol) || (count >= RLE48_EXTREME_MULTI_MIN_RANGE_MEDIUM)))
#endif
        {
#ifndef PACKED
          *(symbol_t *)(&pOut[index]) = symbol;
          index += symbolSize;
#endif

#ifndef UNBOUND
          const int64_t storedCount = (count / symbolSize) - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;
#else
          const int64_t storedCount = count - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
#endif

#ifndef PACKED
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
#else
          const uint8_t isSameSymbolMask = ((symbol == lastSymbol) << 7);
          lastSymbol = symbol;

          if (storedCount <= 0b01111111)
          {
            pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
            index++;
          }
          else
          {
            pOut[index] = isSameSymbolMask;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          if (!isSameSymbolMask)
          {
            *(symbol_t *)(&pOut[index]) = symbol;
            index += symbolSize;
          }
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
          pOut[index] = (uint8_t)(range << 1);
          index++;
#else
          pOut[index] = (uint8_t)range;
          index++;
#endif

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
        else if (count >= RLE48_EXTREME_MULTI_MIN_RANGE_LONG)
        {
#ifndef PACKED
          *(symbol_t *)(&pOut[index]) = symbol;
          index += symbolSize;
#endif
          
#ifndef UNBOUND
          const int64_t storedCount = (count / symbolSize) - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;
#else
          const int64_t storedCount = count - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
#endif

#ifndef PACKED
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
#else
          const uint8_t isSameSymbolMask = ((symbol == lastSymbol) << 7);
          lastSymbol = symbol;

          if (storedCount <= 0b01111111)
          {
            pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
            index++;
          }
          else
          {
            pOut[index] = isSameSymbolMask;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          if (!isSameSymbolMask)
          {
            *(symbol_t *)(&pOut[index]) = symbol;
            index += symbolSize;
          }
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
          *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | 1;
          index += sizeof(uint32_t);
#else
          pOut[index] = 0;
          index++;
          *((uint32_t *)&pOut[index]) = (uint32_t)range;
          index += sizeof(uint32_t);
#endif

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

#ifndef PACKED
    if (range <= RLE48_EXTREME_MAX_COPY_RANGE && count >= RLE48_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
    if (range <= RLE48_EXTREME_MAX_COPY_RANGE && ((count >= RLE48_EXTREME_MULTI_MIN_RANGE_SHORT && symbol == lastSymbol) || (count >= RLE48_EXTREME_MULTI_MIN_RANGE_MEDIUM)))
#endif
    {
#ifndef PACKED
      *(symbol_t *)(&pOut[index]) = symbol;
      index += symbolSize;
#endif
      
#ifndef UNBOUND
      const int64_t storedCount = (count / symbolSize) - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;
#else
      const int64_t storedCount = count - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
#endif

#ifndef PACKED
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
#else
      const uint8_t isSameSymbolMask = ((symbol == lastSymbol) << 7);
      lastSymbol = symbol;

      if (storedCount <= 0b01111111)
      {
        pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
        index++;
      }
      else
      {
        pOut[index] = isSameSymbolMask;
        index++;
        *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }

      if (!isSameSymbolMask)
      {
        *(symbol_t *)(&pOut[index]) = symbol;
        index += symbolSize;
      }
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
      pOut[index] = (uint8_t)(range << 1);
      index++;
#else
      pOut[index] = (uint8_t)range;
      index++;
#endif

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

#ifndef PACKED
      *(symbol_t *)(&pOut[index]) = 0;
      index += symbolSize;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0b10000000;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
      *((uint32_t *)&pOut[index]) = 1;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#endif

      lastRLE = i;
    }
    else if (count >= RLE48_EXTREME_MULTI_MIN_RANGE_LONG)
    {
#ifndef PACKED
      *(symbol_t *)(&pOut[index]) = symbol;
      index += symbolSize;
#endif

#ifndef UNBOUND
      const int64_t storedCount = (count / symbolSize) - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;
#else
      const int64_t storedCount = count - (RLE48_EXTREME_MULTI_MIN_RANGE_SHORT)+1;
#endif

#ifndef PACKED
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
#else
      const uint8_t isSameSymbolMask = ((symbol == lastSymbol) << 7);
      lastSymbol = symbol;

      if (storedCount <= 0b01111111)
      {
        pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
        index++;
      }
      else
      {
        pOut[index] = isSameSymbolMask;
        index++;
        *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }

      if (!isSameSymbolMask)
      {
        *(symbol_t *)(&pOut[index]) = symbol;
        index += symbolSize;
      }
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
      *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | 1;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);
#endif

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

#ifndef PACKED
      *(symbol_t *)(&pOut[index]) = 0;
      index += symbolSize;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0b10000000;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
      *((uint32_t *)&pOut[index]) = 1;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#endif

      lastRLE = i;
    }
    else
    {
#ifndef PACKED
      *(symbol_t *)(&pOut[index]) = 0;
      index += symbolSize;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0b10000000;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
      *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | 1;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);
#endif

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
