#define RLE8_XSYMLUT_MIN_RANGE_SHORT (2 + 1) // compression ratio can be slightly improved by reducing this to 2 (because then commonly used symbols accumulate more easily etc.) but it drastically reduces decompression speed.
#define RLE8_XSYMLUT_MIN_RANGE_LONG (2 + 4 + 4 + 1)

#if SYMBOL_COUNT == 3
  #define RLE8_XSYMLUT_COUNT_BITS (7)
  #define RLE8_XSYMLUT_RANGE_BITS (14 - RLE8_XSYMLUT_COUNT_BITS)
  #define RLE8_XSYMLUT_MAX_TINY_COUNT ((1 << RLE8_XSYMLUT_COUNT_BITS) - 1)
  #define RLE8_XSYMLUT_MAX_TINY_RANGE ((1 << RLE8_XSYMLUT_RANGE_BITS) - 1)
#elif SYMBOL_COUNT == 7
  #define RLE8_XSYMLUT_COUNT_BITS (7)
  #define RLE8_XSYMLUT_RANGE_BITS (13 - RLE8_XSYMLUT_COUNT_BITS)
  #define RLE8_XSYMLUT_MAX_TINY_COUNT ((1 << RLE8_XSYMLUT_COUNT_BITS) - 1)
  #define RLE8_XSYMLUT_MAX_TINY_RANGE ((1 << RLE8_XSYMLUT_RANGE_BITS) - 1)
#else
  #fail NOT_IMPLEMENTED
#endif

//////////////////////////////////////////////////////////////////////////

typedef struct
{
  uint8_t symbol;
  uint8_t lastSymbols[SYMBOL_COUNT];
  int64_t count;
  int64_t lastRLE;
  size_t index;
} CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_state_t);

//////////////////////////////////////////////////////////////////////////

static int64_t CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_state_t) *pState);
static int64_t CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_state_t) *pState);

//////////////////////////////////////////////////////////////////////////

inline bool CONCAT3(_rle8_, SYMBOL_COUNT, symlut_process_symbol)(IN const uint8_t *pIn, OUT uint8_t *pOut, const size_t i, IN OUT CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_state_t) *pState)
{
  size_t symbolMatchIndex = 0;

  for (; symbolMatchIndex < SYMBOL_COUNT; symbolMatchIndex++)
    if (pState->symbol == pState->lastSymbols[symbolMatchIndex])
      break;

  const int64_t range = i - pState->lastRLE - pState->count + 2;
  const int64_t storedCount = pState->count - RLE8_XSYMLUT_MIN_RANGE_SHORT + 2;
  const int64_t penalty = (range <= 0xFFFFF ? (range <= RLE8_XSYMLUT_MAX_TINY_RANGE ? 0 : 2) : 4) + (storedCount <= 0xFFFFF ? (storedCount <= (RLE8_XSYMLUT_MAX_TINY_COUNT) ? 0 : 2) : 4) + (int64_t)(symbolMatchIndex == SYMBOL_COUNT);

  if (pState->count >= RLE8_XSYMLUT_MIN_RANGE_LONG || (pState->count >= RLE8_XSYMLUT_MIN_RANGE_SHORT + penalty))
  {
    switch (symbolMatchIndex)
    {
#if SYMBOL_COUNT == 7
    case 7:
    case 6:
      pState->lastSymbols[6] = pState->lastSymbols[5];
#ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
#endif

    case 5:
      pState->lastSymbols[5] = pState->lastSymbols[4];
#ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
#endif

    case 4:
      pState->lastSymbols[4] = pState->lastSymbols[3];
#ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
#endif

    case 3:
      pState->lastSymbols[3] = pState->lastSymbols[2];
#ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
#endif

#elif SYMBOL_COUNT == 3
    case 3:
#endif
    case 2:
      pState->lastSymbols[2] = pState->lastSymbols[1];
#ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
#endif

    case 1:
      pState->lastSymbols[1] = pState->lastSymbols[0];
      pState->lastSymbols[0] = pState->symbol;
      break;

    case 0:
      break;
    }

    uint16_t storedCount7;
    uint16_t range7;

    if (storedCount <= RLE8_XSYMLUT_MAX_TINY_COUNT)
      storedCount7 = (uint8_t)storedCount;
    else if (storedCount <= 0xFFFF)
      storedCount7 = 1;
    else
      storedCount7 = 0;

    if (range <= RLE8_XSYMLUT_MAX_TINY_RANGE)
      range7 = (uint8_t)range;
    else if (range <= 0xFFFF)
      range7 = 1;
    else
      range7 = 0;

    size_t index = pState->index;
#if SYMBOL_COUNT == 3
    const uint16_t value = (uint16_t)(symbolMatchIndex << 14) | (storedCount7 << RLE8_XSYMLUT_RANGE_BITS) | range7;
#elif SYMBOL_COUNT == 7
    const uint16_t value = (uint16_t)(symbolMatchIndex << 13) | (storedCount7 << RLE8_XSYMLUT_RANGE_BITS) | range7;
#endif

    *((uint16_t *)&pOut[index]) = value;
    index += sizeof(uint16_t);

    if (symbolMatchIndex == SYMBOL_COUNT)
    {
      pOut[index] = pState->symbol;
      index++;
    }

    if (storedCount != storedCount7)
    {
      if (storedCount <= 0xFFFF)
      {
        *((uint16_t *)&pOut[index]) = (uint16_t)storedCount;
        index += sizeof(uint16_t);
      }
      else
      {
        *((uint32_t *)&pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }
    }

    if (range != range7)
    {
      if (range <= 0xFFFF)
      {
        *((uint16_t *)&pOut[index]) = (uint16_t)range;
        index += sizeof(uint16_t);
      }
      else
      {
        *((uint32_t *)&pOut[index]) = (uint32_t)range;
        index += sizeof(uint32_t);
      }
    }

    const size_t copySize = i - pState->count - pState->lastRLE;

    memcpy(pOut + index, pIn + pState->lastRLE, copySize);
    index += copySize;

    pState->lastRLE = i;
    pState->index = index;

    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////

uint32_t CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_compress_bounds(inSize))
    return 0;

  ((uint32_t *)pOut)[0] = (uint32_t)inSize;

  CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_state_t) state;
  memset(&state, 0, sizeof(state));
  state.index = sizeof(uint32_t) * 2;
  state.lastSymbols[0] = 0x00;
  state.lastSymbols[1] = 0x7F;
  state.lastSymbols[2] = 0xFF;
#if SYMBOL_COUNT == 7
  state.lastSymbols[3] = 0x01;
  state.lastSymbols[4] = 0x7E;
  state.lastSymbols[5] = 0x80;
  state.lastSymbols[6] = 0xFE;
#endif


  state.symbol = ~(*pIn);

  int64_t i = 0;

  _DetectCPUFeatures();

  if (avx2Supported)
    i = CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_avx2)(pIn, inSize, pOut, &state);
  else
    i = CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_sse2)(pIn, inSize, pOut, &state);

  for (; i < inSize; i++)
  {
    if (pIn[i] == state.symbol)
    {
      state.count++;
    }
    else
    {
      CONCAT3(_rle8_, SYMBOL_COUNT, symlut_process_symbol)(pIn, pOut, i, &state);

      state.symbol = pIn[i];
      state.count = 1;
    }
  }

  // Copy / Encode remaining bytes.
  {
    const int64_t range = i - state.lastRLE - state.count + 2;

    if (CONCAT3(_rle8_, SYMBOL_COUNT, symlut_process_symbol)(pIn, pOut, i, &state))
    {
#if SYMBOL_COUNT == 3
      *((uint16_t *)&pOut[state.index]) = 0b0000000010000001;
#else
      *((uint16_t *)&pOut[state.index]) = 0b0000000001000001;
#endif
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
    }
    else
    {
#if SYMBOL_COUNT == 3
      *((uint16_t *)&pOut[state.index]) = 0b0000000010000000;
#else
      *((uint16_t *)&pOut[state.index]) = 0b0000000001000000;
#endif
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint32_t *)&pOut[state.index]) = (uint32_t)range;
      state.index += sizeof(uint32_t);

      const size_t copySize = i - state.lastRLE;

      memcpy(pOut + state.index, pIn + state.lastRLE, copySize);
      state.index += copySize;
    }
  }

  // Store compressed length.
  ((uint32_t *)pOut)[1] = (uint32_t)state.index;

  return (uint32_t)state.index;
}

//////////////////////////////////////////////////////////////////////////

static int64_t CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_state_t) *pState)
{
  const int64_t endInSize128 = inSize - sizeof(__m128i);
  int64_t i = 0;
  __m128i symbol128 = _mm_set1_epi8(pState->symbol);

  while (i < endInSize128)
  {
    const uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(symbol128, _mm_loadu_si128((const __m128i *) & (pIn[i]))));

    if (0xFFFF == mask)
    {
      pState->count += sizeof(symbol128);
      i += sizeof(symbol128);
    }
    else
    {
      if (mask != 0 || pState->count > 1)
      {
#ifdef _MSC_VER
        unsigned long _zero;
        _BitScanForward64(&_zero, ~mask);
#else
        const uint64_t _zero = __builtin_ctzl(~mask);
#endif

        pState->count += _zero;
        i += _zero;

        CONCAT3(_rle8_, SYMBOL_COUNT, symlut_process_symbol)(pIn, pOut, i, pState);
      }

      while (i < endInSize128)
      {
        const __m128i current = _mm_loadu_si128((const __m128i *)(&pIn[i]));
        const __m128i next = _mm_bsrli_si128(current, 1);
        const int32_t cmp = 0x7FFF & _mm_movemask_epi8(_mm_cmpeq_epi8(current, next));

        if (cmp == 0)
        {
          i += sizeof(symbol128) - 1;
        }
        else
        {
#ifdef _MSC_VER
          unsigned long _zero;
          _BitScanForward64(&_zero, cmp);
#else
          const uint64_t _zero = __builtin_ctzl(cmp);
#endif

          i += _zero;
          break;
        }
      }

      pState->symbol = pIn[i];
      symbol128 = _mm_set1_epi8(pState->symbol);
      pState->count = 1;
      i++;
    }
  }

  return i;
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static int64_t CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8_, SYMBOL_COUNT, symlut_compress_state_t) *pState)
{
  const int64_t endInSize256 = inSize - sizeof(__m256i);
  int64_t i = 0;
  __m256i symbol256 = _mm256_set1_epi8(pState->symbol);

  while (i < endInSize256)
  {
    const uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(symbol256, _mm256_loadu_si256((const __m256i *) & (pIn[i]))));

    if (0xFFFFFFFF == mask)
    {
      pState->count += sizeof(symbol256);
      i += sizeof(symbol256);
    }
    else
    {
      if (mask != 0 || pState->count > 1)
      {
#ifdef _MSC_VER
        unsigned long _zero;
        _BitScanForward64(&_zero, ~mask);
#else
        const uint64_t _zero = __builtin_ctzl(~mask);
#endif

        pState->count += _zero;
        i += _zero;

        CONCAT3(_rle8_, SYMBOL_COUNT, symlut_process_symbol)(pIn, pOut, i, pState);
      }

      while (i < endInSize256)
      {
        const __m256i current = _mm256_loadu_si256((const __m256i *)(&pIn[i]));
        const __m256i next = _mm256_loadu_si256((const __m256i *)(&pIn[i + 1]));
        const int32_t cmp = 0x7FFFFFFF & _mm256_movemask_epi8(_mm256_cmpeq_epi8(current, next));

        if (cmp == 0)
        {
          i += sizeof(symbol256) - 1;
        }
        else
        {
#ifdef _MSC_VER
          unsigned long _zero;
          _BitScanForward64(&_zero, cmp);
#else
          const uint64_t _zero = __builtin_ctzl(cmp);
#endif

          i += _zero;
          break;
        }
      }

      pState->symbol = pIn[i];
      symbol256 = _mm256_set1_epi8(pState->symbol);
      pState->count = 1;
      i++;
    }
  }

  return i;
}

//////////////////////////////////////////////////////////////////////////

static void CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_sse)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = _mm_set1_epi8(0x7F);
  other[1] = _mm_set1_epi8(0xFF);
#if SYMBOL_COUNT == 7
  other[2] = _mm_set1_epi8(0x01);
  other[3] = _mm_set1_epi8(0x7E);
  other[4] = _mm_set1_epi8(0x80);
  other[5] = _mm_set1_epi8(0xFE);
#endif

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    offset = value & RLE8_XSYMLUT_MAX_TINY_RANGE;
    symbolCount = (value >> RLE8_XSYMLUT_RANGE_BITS) & RLE8_XSYMLUT_MAX_TINY_COUNT;

#if SYMBOL_COUNT == 3
    const uint16_t symbolIndex = value >> 14;
#else
    const uint16_t symbolIndex = value >> 13;
#endif

    switch (symbolIndex)
    {
#if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 6:
    {
      const __m128i tmp = other[5];
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 5:
    {
      const __m128i tmp = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 4:
    {
      const __m128i tmp = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 3:
    {
      const __m128i tmp = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }
#else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm_set1_epi8(*pInStart);
      pInStart++;
      break;
    }
#endif

    case 2:
    {
      const __m128i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m128i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    offset -= 2;

    // memcpy.
    MEMCPY_SSE;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_SSE;
  }
}

#ifndef _MSC_VER
__attribute__((target("sse4.1")))
#endif
static void CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_sse41)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = _mm_set1_epi8(0x7F);
  other[1] = _mm_set1_epi8(0xFF);
#if SYMBOL_COUNT == 7
  other[3] = _mm_set1_epi8(0x01);
  other[4] = _mm_set1_epi8(0x7E);
  other[5] = _mm_set1_epi8(0x80);
  other[6] = _mm_set1_epi8(0xFE);
#endif

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    offset = value & RLE8_XSYMLUT_MAX_TINY_RANGE;
    symbolCount = (value >> RLE8_XSYMLUT_RANGE_BITS) & RLE8_XSYMLUT_MAX_TINY_COUNT;

#if SYMBOL_COUNT == 3
    const uint16_t symbolIndex = value >> 14;
#else
    const uint16_t symbolIndex = value >> 13;
#endif

    switch (symbolIndex)
    {
#if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 6:
    {
      const __m128i tmp = other[5];
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 5:
    {
      const __m128i tmp = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 4:
    {
      const __m128i tmp = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 3:
    {
      const __m128i tmp = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }
#else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm_set1_epi8(*pInStart);
      pInStart++;
      break;
    }
#endif

    case 2:
    {
      const __m128i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m128i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    offset -= 2;

    // memcpy.
    MEMCPY_SSE41;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_SSE;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#endif
static void CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_avx)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();
  __m256i other[SYMBOL_COUNT - 1];

  other[0] = _mm256_set1_epi8(0x7F);
  other[1] = _mm256_set1_epi8(0xFF);
#if SYMBOL_COUNT == 7
  other[3] = _mm256_set1_epi8(0x01);
  other[4] = _mm256_set1_epi8(0x7E);
  other[5] = _mm256_set1_epi8(0x80);
  other[6] = _mm256_set1_epi8(0xFE);
#endif

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    offset = value & RLE8_XSYMLUT_MAX_TINY_RANGE;
    symbolCount = (value >> RLE8_XSYMLUT_RANGE_BITS) & RLE8_XSYMLUT_MAX_TINY_COUNT;

#if SYMBOL_COUNT == 3
    const uint16_t symbolIndex = value >> 14;
#else
    const uint16_t symbolIndex = value >> 13;
#endif

    switch (symbolIndex)
    {
#if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm256_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 6:
    {
      const __m256i tmp = other[5];
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 5:
    {
      const __m256i tmp = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 4:
    {
      const __m256i tmp = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 3:
    {
      const __m256i tmp = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }
#else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm256_set1_epi8(*pInStart);
      pInStart++;
      break;
    }
#endif

    case 2:
    {
      const __m256i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m256i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    offset -= 2;

    // memcpy.
    MEMCPY_AVX;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_AVX;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static void CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_avx2)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();
  __m256i other[SYMBOL_COUNT - 1];

  other[0] = _mm256_set1_epi8(0x7F);
  other[1] = _mm256_set1_epi8(0xFF);
#if SYMBOL_COUNT == 7
  other[3] = _mm256_set1_epi8(0x01);
  other[4] = _mm256_set1_epi8(0x7E);
  other[5] = _mm256_set1_epi8(0x80);
  other[6] = _mm256_set1_epi8(0xFE);
#endif

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    offset = value & RLE8_XSYMLUT_MAX_TINY_RANGE;
    symbolCount = (value >> RLE8_XSYMLUT_RANGE_BITS) & RLE8_XSYMLUT_MAX_TINY_COUNT;

#if SYMBOL_COUNT == 3
    const uint16_t symbolIndex = value >> 14;
#else
    const uint16_t symbolIndex = value >> 13;
#endif

    switch (symbolIndex)
    {
#if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm256_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 6:
    {
      const __m256i tmp = other[5];
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 5:
    {
      const __m256i tmp = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 4:
    {
      const __m256i tmp = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 3:
    {
      const __m256i tmp = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }
#else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm256_set1_epi8(*pInStart);
      pInStart++;
      break;
    }
#endif

    case 2:
    {
      const __m256i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m256i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    offset -= 2;

    // memcpy.
    MEMCPY_AVX2;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_AVX2;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx512f")))
#endif
static void CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_avx512f)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m512i symbol = _mm512_setzero_si512();
  __m512i other[SYMBOL_COUNT - 1];

  other[0] = _mm512_set1_epi8(0x7F);
  other[1] = _mm512_set1_epi8(0xFF);
#if SYMBOL_COUNT == 7
  other[3] = _mm512_set1_epi8(0x01);
  other[4] = _mm512_set1_epi8(0x7E);
  other[5] = _mm512_set1_epi8(0x80);
  other[6] = _mm512_set1_epi8(0xFE);
#endif

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    offset = value & RLE8_XSYMLUT_MAX_TINY_RANGE;
    symbolCount = (value >> RLE8_XSYMLUT_RANGE_BITS) & RLE8_XSYMLUT_MAX_TINY_COUNT;

#if SYMBOL_COUNT == 7
    const uint16_t symbolIndex = value >> 13;
#else
    const uint16_t symbolIndex = value >> 14;
#endif

    switch (symbolIndex)
    {
#if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm512_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 6:
    {
      const __m512i tmp = other[5];
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 5:
    {
      const __m512i tmp = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 4:
    {
      const __m512i tmp = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 3:
    {
      const __m512i tmp = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }
#else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm512_set1_epi8(*pInStart);
      pInStart++;
      break;
    }
#endif

    case 2:
    {
      const __m512i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m512i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    if (offset == 0)
      return;

    offset -= 2;

    // memcpy.
    MEMCPY_AVX512;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_AVX512;
  }
}

//////////////////////////////////////////////////////////////////////////

uint32_t CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[1];
  const size_t expectedOutSize = ((uint32_t *)pIn)[0];
  pIn += sizeof(uint32_t) * 2;

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  _DetectCPUFeatures();

  if (avx512FSupported)
    CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_avx512f)(pIn, pOut);
  else if (avx2Supported)
    CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_avx2)(pIn, pOut);
  else if (avxSupported)
    CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_avx)(pIn, pOut);
  else if (sse41Supported)
    CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_sse41)(pIn, pOut);
  else
    CONCAT3(rle8_, SYMBOL_COUNT, symlut_decompress_sse)(pIn, pOut);

  return (uint32_t)expectedOutSize;
}

#undef RLE8_XSYMLUT_MIN_RANGE_SHORT
#undef RLE8_XSYMLUT_MIN_RANGE_LONG

#undef RLE8_XSYMLUT_COUNT_BITS
#undef RLE8_XSYMLUT_RANGE_BITS
#undef RLE8_XSYMLUT_MAX_TINY_COUNT
#undef RLE8_XSYMLUT_MAX_TINY_RANGE
