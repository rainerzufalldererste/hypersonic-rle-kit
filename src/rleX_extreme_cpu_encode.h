#if defined(COMPRESS_IMPL_SSE2)
  #define ENCODE_FUNC_NAME _compress_sse2
#elif defined(COMPRESS_IMPL_AVX2)
  #define ENCODE_FUNC_NAME _compress_avx2
#else
  #define ENCODE_FUNC_NAME _compress_base
#endif

#ifndef _MSC_VER
  #if defined(COMPRESS_IMPL_AVX2)
__attribute__((target("avx2")))
  #endif
#endif
uint32_t CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, ENCODE_FUNC_NAME))(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
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

  typedef CONCAT3(uint, TYPE_SIZE, _t) symbol_t;

  int64_t count = 0;
  symbol_t symbol = ~(*((symbol_t *)(pIn)));

#if defined(COMPRESS_IMPL_SSE2)
  __m128i symbolSimd = CONCAT2(_mm_set1_epi, TYPE_SIZE)(symbol);
  const int64_t inSizeSimd = inSize - sizeof(__m128i) - sizeof(symbol_t);
#elif defined(COMPRESS_IMPL_AVX2)
  __m256i symbolSimd = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(symbol);
  const int64_t inSizeSimd = inSize - sizeof(__m256i) - sizeof(symbol_t);
#endif

#ifdef PACKED
  symbol_t lastSymbol = 0;
#endif

  while (i < inSize)
  {
#if defined(COMPRESS_IMPL_SSE2) || defined(COMPRESS_IMPL_AVX2)
    continue_outer_loop:;

#if defined(COMPRESS_IMPL_SSE2)
    symbolSimd = CONCAT2(_mm_set1_epi, TYPE_SIZE)(symbol);
#elif defined(COMPRESS_IMPL_AVX2)
    symbolSimd = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(symbol);
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
        if (bitMask != (uint32_t)(((size_t)(1) << sizeof(current)) - 1))
        {
#ifdef _MSC_VER
          unsigned long bit;
          _BitScanForward64(&bit, ~bitMask);
#else
          uint64_t bit = __builtin_ctzl(~bitMask);
#endif
#ifndef UNBOUND
          bit &= ~(uint32_t)(((uint32_t)(TYPE_SIZE / 8)) - 1);
#endif

          i += bit;
          count += bit;

          goto continue_second_step;
        }
        else
        {
          // This should be different for 24, 48 bit!
          i += (sizeof(current));
          count += (sizeof(current));
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
      if (i + sizeof(symbol_t) <= inSize)
      {
        const symbol_t next = *(symbol_t *)&pIn[i];

        if (next == symbol)
        {
          count += sizeof(symbol_t);
          i += sizeof(symbol_t);
          continue;
        }
#ifdef UNBOUND
        else
        {
#if TYPE_SIZE == 16
          uint8_t symBytes[sizeof(symbol)];
          memcpy(symBytes, &symbol, sizeof(symbol));

          if (symBytes[0] == pIn[i])
          {
            count++;
            i++;
          }
#elif TYPE_SIZE == 32
          const symbol_t diff = symbol ^ *(symbol_t *)&pIn[i];

#ifdef _MSC_VER
          unsigned long offset;
          _BitScanForward(&offset, diff);
#else
          const uint32_t offset = __builtin_ctz(diff);
#endif

          i += (offset / 8);
          count += (offset / 8);
#elif TYPE_SIZE == 64
          const symbol_t diff = symbol ^ *(symbol_t *)&pIn[i];

#ifdef _MSC_VER
          unsigned long offset;
          _BitScanForward64(&offset, diff);
#else
          const uint32_t offset = __builtin_ctzl(diff);
#endif

          i += (offset / 8);
          count += (offset / 8);
#else // backup
          uint8_t symBytes[sizeof(symbol)];
          memcpy(symBytes, &symbol, sizeof(symbol));

          for (size_t j = 0; j < (sizeof(symbol) - 1); j++) // can't reach the absolute max.
          {
            if (pIn[i] != symBytes[j])
              break;

            count++;
            i++;
          }
#endif
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
        if (range <= RLEX_EXTREME_MAX_COPY_RANGE && count >= RLEX_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
        if (range <= RLEX_EXTREME_MAX_COPY_RANGE && ((count >= RLEX_EXTREME_MULTI_MIN_RANGE_SHORT && symbol == lastSymbol) || (count >= RLEX_EXTREME_MULTI_MIN_RANGE_MEDIUM)))
#endif
        {
#ifndef PACKED
          *(symbol_t *)(&pOut[index]) = symbol;
          index += sizeof(symbol_t);
#endif

#ifndef UNBOUND
          const int64_t storedCount = (count / sizeof(symbol_t)) - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) + 1;
#else
          const int64_t storedCount = count - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT)+1;
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
            index += sizeof(symbol_t);
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
        else if (count >= RLEX_EXTREME_MULTI_MIN_RANGE_LONG)
        {
#ifndef PACKED
          *(symbol_t *)(&pOut[index]) = symbol;
          index += sizeof(symbol_t);
#endif

#ifndef UNBOUND
          const int64_t storedCount = (count / sizeof(symbol_t)) - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) + 1;
#else
          const int64_t storedCount = count - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT)+1;
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
            index += sizeof(symbol_t);
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
#if TYPE_SIZE == 16
        bitMask &= (bitMask >> 1);
#elif TYPE_SIZE == 32
        bitMask &= (bitMask >> 2);
        bitMask &= (bitMask >> 1);
#elif TYPE_SIZE == 64
        bitMask &= (bitMask >> 4);
        bitMask &= (bitMask >> 2);
        bitMask &= (bitMask >> 1);
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
            const uint64_t bit = __builtin_ctzl(bitMask);
#endif
            symbol = *(symbol_t *)(&pIn[i + bit]);

            i += bit + sizeof(symbol_t) * 2;
            count = sizeof(symbol_t) * 2;

            goto continue_outer_loop;
          }
      }
#endif

      symbol = *(symbol_t *)(&pIn[i]);

      if (i + sizeof(symbol_t) <= inSize && ((symbol_t *)(&pIn[i]))[1] == symbol)
      {
        count = sizeof(symbol_t) * 2;
        i += sizeof(symbol_t) * 2;
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
    if (range <= RLEX_EXTREME_MAX_COPY_RANGE && count >= RLEX_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
    if (range <= RLEX_EXTREME_MAX_COPY_RANGE && ((count >= RLEX_EXTREME_MULTI_MIN_RANGE_SHORT && symbol == lastSymbol) || (count >= RLEX_EXTREME_MULTI_MIN_RANGE_MEDIUM)))
#endif
    {
#ifndef PACKED
      *(symbol_t *)(&pOut[index]) = symbol;
      index += sizeof(symbol_t);
#endif

#ifndef UNBOUND
      const int64_t storedCount = (count / sizeof(symbol_t)) - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) + 1;
#else
      const int64_t storedCount = count - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT)+1;
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
        index += sizeof(symbol_t);
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
      index += sizeof(symbol_t);
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
    else if (count >= RLEX_EXTREME_MULTI_MIN_RANGE_LONG)
    {
#ifndef PACKED
      *(symbol_t *)(&pOut[index]) = symbol;
      index += sizeof(symbol_t);
#endif

#ifndef UNBOUND
      const int64_t storedCount = (count / sizeof(symbol_t)) - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) + 1;
#else
      const int64_t storedCount = count - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT)+1;
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
        index += sizeof(symbol_t);
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
      index += sizeof(symbol_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 1;
      index++;
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
      index += sizeof(symbol_t);
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

      const size_t copySize = i - lastRLE;

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
      *((uint32_t *)&pOut[index]) = (uint32_t)((copySize + 1) << 1) | 1;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)copySize + 1;
      index += sizeof(uint32_t);
#endif

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;
    }
  }

  // Store compressed length.
  ((uint32_t *)pOut)[1] = (uint32_t)index;

  return (uint32_t)index;
}

#undef ENCODE_FUNC_NAME
