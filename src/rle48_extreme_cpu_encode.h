#if defined(IMPL_SSE2)
  #define FUNC_NAME _compress_sse2
#elif defined(IMPL_SSSE3)
  #define FUNC_NAME _compress_ssse3
#elif defined(IMPL_AVX2)
  #define FUNC_NAME _compress_avx2
#else
  #define FUNC_NAME _compress_base
#endif

#ifndef _MSC_VER
#if defined(IMPL_SSSE3)
  __attribute__((target("ssse3")))
#elif defined(IMPL_AVX2)
  __attribute__((target("avx2")))
#endif
#endif
uint32_t CONCAT3(rle48_, CODEC, FUNC_NAME)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
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

#if defined(COMPRESS_IMPL_SSE2)
  __m128i symbolSimd = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(symbol);
  const int64_t inSizeSimd = inSize - sizeof(__m128i) - (TYPE_SIZE / 8);
#elif defined(COMPRESS_IMPL_AVX2)
  __m256i symbolSimd = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(symbol);
  const int64_t inSizeSimd = inSize - sizeof(__m256i) - (TYPE_SIZE / 8);
#endif

#if defined(IMPL_SSE2)
  const __m128i pattern00 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1);
  const __m128i pattern02 = _mm_set_epi8(0, 0, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0);
  const __m128i pattern04 = _mm_set_epi8(-1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#elif defined(IMPL_SSSE3)
  const __m128i shuffle0 = _mm_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
#elif defined(IMPL_AVX2)
  const __m256i shuffle0 = _mm256_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
#endif

#ifdef PACKED
  symbol_t lastSymbol = 0;
#endif

  while (i < inSize)
  {
#if defined(COMPRESS_IMPL_SSE2) || defined(COMPRESS_IMPL_AVX2)
    continue_outer_loop:;

#if defined(COMPRESS_IMPL_SSE2)
    symbolSimd = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(symbol);
#elif defined(COMPRESS_IMPL_AVX2)
    symbolSimd = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(symbol);
#endif

#if defined(IMPL_SSE2)
    const __m128i shift2 = _mm_or_si128(_mm_srli_si128(symbolSimd, 2), _mm_slli_si128(symbolSimd, 14));
    const __m128i symbol0 = _mm_or_si128(_mm_and_si128(symbolSimd, pattern00), _mm_and_si128(shift2, pattern02));
    const __m128i shift4 = _mm_or_si128(_mm_srli_si128(symbolSimd, 4), _mm_slli_si128(symbolSimd, 12));

    symbolSimd = _mm_or_si128(symbol0, _mm_and_si128(shift4, pattern04));
#elif defined(IMPL_SSSE3)
    symbolSimd = _mm_shuffle_epi8(symbolSimd, shuffle0);
#elif defined(IMPL_AVX2)
    symbolSimd = _mm256_shuffle_epi8(symbolSimd, shuffle0);
#endif

    while (i < inSizeSimd)
    {
#if defined(COMPRESS_IMPL_SSE2)
      const __m128i current = _mm_loadu_si128((const __m128i *)(pIn + i));
      const __m128i matchMask = _mm_cmpeq_epi8(current, symbolSimd);
      uint32_t bitMask = _mm_movemask_epi8(matchMask);
#elif defined(COMPRESS_IMPL_AVX2)
      const __m256i current = _mm256_loadu_si256((const __m256i *)(pIn + i));
      const __m256i matchMask = _mm256_cmpeq_epi8(current, symbolSimd);
      uint32_t bitMask = _mm256_movemask_epi8(matchMask);
#endif

      if (bitMask & 1)
      {
        if (bitMask != ((uint32_t)(1) << sizeof(current)) - 1)
        {
#ifdef _MSC_VER
          unsigned long bit;
          _BitScanForward64(&bit, ~bitMask);
#else
          uint64_t bit = __builtin_ctzl(~bitMask);
#endif
#ifndef UNBOUND
          bit -= (bit % (TYPE_SIZE / 8));
#endif

          i += bit;
          count += bit;

          goto continue_second_step;
        }
        else
        {
          i += (sizeof(current) - (sizeof(current) % (TYPE_SIZE / 8)));
          count += (sizeof(current) - (sizeof(current) % (TYPE_SIZE / 8)));
        }
      }
      else
      {
        goto continue_second_step;
      }
    }
#endif

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

#if defined(COMPRESS_IMPL_SSE2) || defined(COMPRESS_IMPL_AVX2)
    continue_second_step:;
#endif

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

#if defined(COMPRESS_IMPL_SSE2) || defined(COMPRESS_IMPL_AVX2)
      while (i < inSizeSimd)
      {
#if defined(COMPRESS_IMPL_SSE2)
        const __m128i current = _mm_loadu_si128((const __m128i *)(pIn + i));
        const __m128i other = _mm_loadu_si128((const __m128i *)(pIn + i + (TYPE_SIZE / 8)));
        //const __m128i other = _mm_bsrli_si128(current, TYPE_SIZE / 8);
        const __m128i matchMask = _mm_cmpeq_epi8(current, other);
        uint32_t bitMask = _mm_movemask_epi8(matchMask);
#elif defined(COMPRESS_IMPL_AVX2)
        const __m256i current = _mm256_loadu_si256((const __m256i *)(pIn + i));
        const __m256i other = _mm256_loadu_si256((const __m256i *)(pIn + i + (TYPE_SIZE / 8)));
        const __m256i matchMask = _mm256_cmpeq_epi8(current, other);
        uint32_t bitMask = _mm256_movemask_epi8(matchMask);
#endif

        //bitMask &= (((uint32_t)1 << (sizeof(current) - (TYPE_SIZE / 8))) - 1);

        // Check if all symbols match:
#if TYPE_SIZE == 48
        bitMask &= (bitMask >> 3);
        bitMask &= ((bitMask >> 1) & (bitMask >> 2));
#else
        #fail NOT IMPLEMENTED
#endif

          if (bitMask == 0)
          {
            i += (sizeof(current) - (TYPE_SIZE / 8));
            //i += (sizeof(current));
          }
          else
          {
#ifdef _MSC_VER
            unsigned long bit;
            _BitScanForward64(&bit, bitMask);
#else
            uint64_t bit = __builtin_ctzl(bitMask);
#endif
            symbol = *(symbol_t *)(&pIn[i + bit]) & symbolMask;

            i += bit + symbolSize * 2;
            count = symbolSize * 2;

            goto continue_outer_loop;
          }
      }
#endif

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

#undef FUNC_NAME
