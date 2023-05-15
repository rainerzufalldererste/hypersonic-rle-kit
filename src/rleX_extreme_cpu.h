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

uint32_t CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, _compress))(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
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

#ifdef PACKED
  symbol_t lastSymbol = 0;
#endif

  while (i < inSize)
  {
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
          const int64_t storedCount = count - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
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
          const int64_t storedCount = count - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
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
      const int64_t storedCount = count - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
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
      const int64_t storedCount = count - (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
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
