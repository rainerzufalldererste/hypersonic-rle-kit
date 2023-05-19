#define RLE8_XSYMLUT_MIN_RANGE_SHORT (2 + 1) // compression ratio can be slightly improved by reducing this to 2 (because then commonly used symbols accumulate more easily etc.) but it drastically reduces decompression speed.
#define RLE8_XSYMLUT_MIN_RANGE_LONG (2 + 4 + 4 + (TYPE_SIZE / 8))

#if SYMBOL_COUNT == 3
  #define RLE8_XSYMLUT_COUNT_BITS (7)
  #define RLE8_XSYMLUT_RANGE_BITS (14 - RLE8_XSYMLUT_COUNT_BITS)
  #define RLE8_XSYMLUT_MAX_TINY_COUNT ((1 << RLE8_XSYMLUT_COUNT_BITS) - 1)
  #define RLE8_XSYMLUT_MAX_TINY_RANGE ((1 << RLE8_XSYMLUT_RANGE_BITS) - 1)
#elif SYMBOL_COUNT == 7
  #define RLE8_XSYMLUT_COUNT_BITS (7) // this can be set to 6 for some marginal gains / losses, so may be beneficial for some datasets
  #define RLE8_XSYMLUT_RANGE_BITS (13 - RLE8_XSYMLUT_COUNT_BITS)
  #define RLE8_XSYMLUT_MAX_TINY_COUNT ((1 << RLE8_XSYMLUT_COUNT_BITS) - 1)
  #define RLE8_XSYMLUT_MAX_TINY_RANGE ((1 << RLE8_XSYMLUT_RANGE_BITS) - 1)
#else
  #fail NOT_IMPLEMENTED
#endif

#define RLE8_XSYMLUT_COUNT_VALUE_OFFSET (2)
#define RLE8_XSYMLUT_RANGE_VALUE_OFFSET (2)

#if TYPE_SIZE == 8
  #define CODEC CONCAT2(SYMBOL_COUNT, symlut_)
#else
  #ifdef UNBOUND
    #define CODEC_POSTFIX byte_
  #else
    #define CODEC_POSTFIX sym_
  #endif

  #define CODEC CONCAT3(SYMBOL_COUNT, symlut_, CODEC_POSTFIX)
#endif

#if TYPE_SIZE == 64
  #define _mm_set1_epi64 _mm_set1_epi64x
  #define _mm256_set1_epi64 _mm256_set1_epi64x
#endif

#if TYPE_SIZE == 24
  #define SIMD_TYPE_SIZE 32
  #define uintXX_t uint32_t
  #define SYMBOL_MASK 0xFFFFFF
#elif TYPE_SIZE == 48
  #define SIMD_TYPE_SIZE 64x
  #define uintXX_t uint64_t
  #define SYMBOL_MASK 0xFFFFFFFFFFFF
#else
  #define SIMD_TYPE_SIZE TYPE_SIZE
  #define uintXX_t CONCAT3(uint, TYPE_SIZE, _t)
#endif

#if TYPE_SIZE == 8
  #define VALUE_BROADCAST 0x01
#elif TYPE_SIZE == 16
  #define VALUE_BROADCAST 0x0101
#elif TYPE_SIZE == 24
  #define VALUE_BROADCAST 0x010101
#elif TYPE_SIZE == 32
  #define VALUE_BROADCAST 0x01010101
#elif TYPE_SIZE == 48
  #define VALUE_BROADCAST 0x010101010101ULL
#elif TYPE_SIZE == 64
  #define VALUE_BROADCAST 0x0101010101010101ULL
#endif

#if TYPE_SIZE == 8
  #define COPY_SEGMENT_SSE    MEMCPY_SSE
  #define COPY_SEGMENT_SSE41  MEMCPY_SSE41
  #define COPY_SEGMENT_AVX    MEMCPY_AVX
  #define COPY_SEGMENT_AVX2   MEMCPY_AVX2
  #define COPY_SEGMENT_AVX512 MEMCPY_AVX512
  #define SET_SEGMENT_SSE     MEMSET_SSE
  #define SET_SEGMENT_SSE41   MEMSET_SSE
  #define SET_SEGMENT_AVX     MEMSET_AVX
  #define SET_SEGMENT_AVX2    MEMSET_AVX2
  #define SET_SEGMENT_AVX512  MEMSET_AVX512
#else
  #define COPY_SEGMENT_SSE    MEMCPY_SSE_MULTI
  #define COPY_SEGMENT_SSE41  MEMCPY_SSE41_MULTI
  #define COPY_SEGMENT_AVX    MEMCPY_AVX_MULTI
  #define COPY_SEGMENT_AVX2   MEMCPY_AVX2_MULTI
  #define COPY_SEGMENT_AVX512 MEMCPY_AVX512_MULTI
  #define SET_SEGMENT_SSE     MEMSET_SSE_MULTI
  #define SET_SEGMENT_SSE41   MEMSET_SSE_MULTI
  #define SET_SEGMENT_AVX     MEMSET_AVX_MULTI
  #define SET_SEGMENT_AVX2    MEMSET_AVX_MULTI
  #define SET_SEGMENT_AVX512  MEMSET_AVX512_MULTI
#endif

//////////////////////////////////////////////////////////////////////////

typedef struct
{
  uintXX_t symbol;
  uintXX_t lastSymbols[SYMBOL_COUNT];
  int64_t count;
  int64_t lastRLE;
  size_t index;
} CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_state_t);

//////////////////////////////////////////////////////////////////////////

#if TYPE_SIZE == 8
static int64_t CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_state_t) *pState);
static int64_t CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_state_t) *pState);
#endif

//////////////////////////////////////////////////////////////////////////

inline bool CONCAT3(CONCAT3(_rle, TYPE_SIZE, _), CODEC, process_symbol)(IN const uint8_t *pIn, OUT uint8_t *pOut, const size_t i, IN OUT CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_state_t) *pState)
{
  size_t symbolMatchIndex = 0;

  for (; symbolMatchIndex < SYMBOL_COUNT; symbolMatchIndex++)
    if (pState->symbol == pState->lastSymbols[symbolMatchIndex])
      break;

  const int64_t range = i - pState->lastRLE - pState->count + RLE8_XSYMLUT_RANGE_VALUE_OFFSET;

#if defined(UNBOUND) || TYPE_SIZE == 8
  const int64_t storedCount = pState->count - RLE8_XSYMLUT_MIN_RANGE_SHORT + RLE8_XSYMLUT_COUNT_VALUE_OFFSET;
#else
  const int64_t storedCount = (pState->count / (TYPE_SIZE / 8)) - (RLE8_XSYMLUT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) + RLE8_XSYMLUT_COUNT_VALUE_OFFSET;
#endif

  const int64_t penalty = (range <= 0xFFFFF ? (range <= RLE8_XSYMLUT_MAX_TINY_RANGE ? 0 : 2) : 4) + (storedCount <= 0xFFFFF ? (storedCount <= (RLE8_XSYMLUT_MAX_TINY_COUNT) ? 0 : 2) : 4) + (int64_t)(symbolMatchIndex == SYMBOL_COUNT) /* * (TYPE_SIZE / 8) */;

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
      *((uintXX_t *)&(pOut[index])) = pState->symbol;
      index += (TYPE_SIZE / 8);
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

#if TYPE_SIZE == 8
uint32_t CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  ((uint32_t *)pOut)[0] = (uint32_t)inSize;

  CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_state_t) state;
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
    i = CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_avx2)(pIn, inSize, pOut, &state);
  else
    i = CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_sse2)(pIn, inSize, pOut, &state);

  for (; i < inSize; i++)
  {
    if (pIn[i] == state.symbol)
    {
      state.count++;
    }
    else
    {
      CONCAT3(CONCAT3(_rle, TYPE_SIZE, _), CODEC, process_symbol)(pIn, pOut, i, &state);

      state.symbol = pIn[i];
      state.count = 1;
    }
  }

  // Copy / Encode remaining bytes.
  {
    const int64_t range = i - state.lastRLE - state.count + 2;

    if (CONCAT3(CONCAT3(_rle, TYPE_SIZE, _), CODEC, process_symbol)(pIn, pOut, i, &state))
    {
      *((uint16_t *)&pOut[state.index]) = (1 << RLE8_XSYMLUT_RANGE_BITS) | 1;
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
    }
    else
    {
      *((uint16_t *)&pOut[state.index]) = 1 << RLE8_XSYMLUT_RANGE_BITS;
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
#endif

//////////////////////////////////////////////////////////////////////////

#if TYPE_SIZE == 8
static int64_t CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_state_t) *pState)
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

        CONCAT3(CONCAT3(_rle, TYPE_SIZE, _), CODEC, process_symbol)(pIn, pOut, i, pState);
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
static int64_t CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_state_t) *pState)
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

        CONCAT3(CONCAT3(_rle, TYPE_SIZE, _), CODEC, process_symbol)(pIn, pOut, i, pState);
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
#endif

//////////////////////////////////////////////////////////////////////////

#if TYPE_SIZE > 8
uint32_t CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  ((uint32_t *)pOut)[0] = (uint32_t)inSize;

  CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, compress_state_t) state;
  memset(&state, 0, sizeof(state));
  state.index = sizeof(uint32_t) * 2;

  typedef uintXX_t symbol_t;

  state.lastSymbols[0] = 0x00 * VALUE_BROADCAST;
  state.lastSymbols[1] = 0x7F * VALUE_BROADCAST;
  state.lastSymbols[2] = 0xFF * VALUE_BROADCAST;
#if SYMBOL_COUNT == 7
  state.lastSymbols[3] = 0x01 * VALUE_BROADCAST;
  state.lastSymbols[4] = 0x7E * VALUE_BROADCAST;
  state.lastSymbols[5] = 0x80 * VALUE_BROADCAST;
  state.lastSymbols[6] = 0xFE * VALUE_BROADCAST;
#endif

  state.symbol = ~(*((symbol_t *)(pIn)));

#ifdef SYMBOL_MASK
  state.symbol &= SYMBOL_MASK;
#endif

  size_t i = 0;

  while (i < inSize)
  {
    if (state.count)
    {
      if (i + (TYPE_SIZE / 8) <= inSize)
      {
#ifndef SYMBOL_MASK
        const symbol_t next = *(symbol_t *)&pIn[i];
#else
        const symbol_t next = *(symbol_t *)&pIn[i] & SYMBOL_MASK;
#endif

        if (next == state.symbol)
        {
          state.count += (TYPE_SIZE / 8);
          i += (TYPE_SIZE / 8);
          continue;
        }
#ifdef UNBOUND
        else
        {
#if TYPE_SIZE == 16
          uint8_t symBytes[sizeof(state.symbol)];
          memcpy(symBytes, &state.symbol, sizeof(state.symbol));

          if (symBytes[0] == pIn[i])
          {
            state.count++;
            i++;
          }
#elif TYPE_SIZE == 32 || TYPE_SIZE == 24
          const symbol_t diff = state.symbol ^ *(symbol_t *)&pIn[i];

#ifdef _MSC_VER
          unsigned long offset;
          _BitScanForward(&offset, diff);
#else
          const uint32_t offset = __builtin_ctz(diff);
#endif

          i += (offset / 8);
          state.count += (offset / 8);
#elif TYPE_SIZE == 64 || TYPE_SIZE == 48
          const symbol_t diff = state.symbol ^ *(symbol_t *)&pIn[i];

#ifdef _MSC_VER
          unsigned long offset;
          _BitScanForward64(&offset, diff);
#else
          const uint32_t offset = __builtin_ctzl(diff);
#endif

          i += (offset / 8);
          state.count += (offset / 8);
#else // backup
          uint8_t symBytes[sizeof(state.symbol)];
          memcpy(state.symBytes, &state.symbol, sizeof(state.symbol));

          for (size_t j = 0; j < (sizeof(state.symbol) - 1); j++) // can't reach the absolute max.
          {
            if (pIn[i] != symBytes[j])
              break;

            state.count++;
            i++;
          }
#endif
        }
#endif
      }
    }

    {
      CONCAT3(CONCAT3(_rle, TYPE_SIZE, _), CODEC, process_symbol)(pIn, pOut, i, &state);

#ifndef SYMBOL_MASK
      state.symbol = *(symbol_t *)(&pIn[i]);
#else
      state.symbol = *(symbol_t *)(&pIn[i]) & SYMBOL_MASK;
#endif

#ifndef SYMBOL_MASK
      if (i + (TYPE_SIZE / 8) <= inSize && ((symbol_t *)(&pIn[i]))[1] == state.symbol)
#else
      if (i + (TYPE_SIZE / 8) <= inSize && (*((symbol_t *)(&pIn[i + (TYPE_SIZE / 8)])) & SYMBOL_MASK) == state.symbol)
#endif
      {
        state.count = (TYPE_SIZE / 8) * 2;
        i += (TYPE_SIZE / 8) * 2;
      }
      else
      {
        state.count = 0;
        i++;
      }
    }
  }

  // Copy / Encode remaining bytes.
  {
    const int64_t range = i - state.lastRLE - state.count + 2;

    if (CONCAT3(CONCAT3(_rle, TYPE_SIZE, _), CODEC, process_symbol)(pIn, pOut, i, &state))
    {
      *((uint16_t *)&pOut[state.index]) = (1 << RLE8_XSYMLUT_RANGE_BITS) | 1;
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
    }
    else
    {
      *((uint16_t *)&pOut[state.index]) = 1 << RLE8_XSYMLUT_RANGE_BITS;
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
}
#endif

//////////////////////////////////////////////////////////////////////////

static void CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_sse)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0xFF * VALUE_BROADCAST);
#if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0xFE * VALUE_BROADCAST);
#endif

#if TYPE_SIZE == 24
  const __m128i pattern00 = _mm_set_epi8(0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1);
  const __m128i pattern01 = _mm_set_epi8(-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0);
  const __m128i pattern02 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0);
  const __m128i pattern03 = _mm_set_epi8(0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  const __m128i pattern10 = _mm_set_epi8(0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0);
  const __m128i pattern11 = _mm_set_epi8(0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1);
  const __m128i pattern12 = _mm_set_epi8(-1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0);
  const __m128i pattern13 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0);

  const __m128i pattern20 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0);
  const __m128i pattern21 = _mm_set_epi8(0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0);
  const __m128i pattern22 = _mm_set_epi8(0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1);
  const __m128i pattern23 = _mm_set_epi8(-1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0);

  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
#elif TYPE_SIZE == 48
  const __m128i pattern00 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1);
  const __m128i pattern10 = _mm_set_epi8(0, 0, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0);

  const __m128i pattern02 = _mm_set_epi8(0, 0, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0);
  const __m128i pattern12 = _mm_set_epi8(-1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  const __m128i pattern22 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1);

  const __m128i pattern04 = _mm_set_epi8(-1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  const __m128i pattern14 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1);
  const __m128i pattern24 = _mm_set_epi8(0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0);

  const __m128i pattern16 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0);
  const __m128i pattern26 = _mm_set_epi8(-1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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

#ifdef SYMBOL_MASK
    if (symbolIndex)
    {
#if TYPE_SIZE == 24
      const __m128i shift1 = _mm_or_si128(_mm_srli_si128(symbol, 1), _mm_slli_si128(symbol, 15));

      symbol0 = _mm_or_si128(_mm_and_si128(symbol, pattern00), _mm_and_si128(shift1, pattern01));
      symbol1 = _mm_or_si128(_mm_and_si128(symbol, pattern10), _mm_and_si128(shift1, pattern11));
      symbol2 = _mm_or_si128(_mm_and_si128(symbol, pattern20), _mm_and_si128(shift1, pattern21));

      const __m128i shift2 = _mm_or_si128(_mm_srli_si128(symbol, 2), _mm_slli_si128(symbol, 14));
      const __m128i shift3 = _mm_or_si128(_mm_srli_si128(symbol, 3), _mm_slli_si128(symbol, 13));

      symbol0 = _mm_or_si128(symbol0, _mm_or_si128(_mm_and_si128(shift3, pattern03), _mm_and_si128(shift2, pattern02)));
      symbol1 = _mm_or_si128(symbol1, _mm_or_si128(_mm_and_si128(shift3, pattern13), _mm_and_si128(shift2, pattern12)));
      symbol2 = _mm_or_si128(symbol2, _mm_or_si128(_mm_and_si128(shift3, pattern23), _mm_and_si128(shift2, pattern22)));
#elif TYPE_SIZE == 48
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
#endif
    }
#endif

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

    offset -= RLE8_XSYMLUT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_SSE;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - RLE8_XSYMLUT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

#if TYPE_SIZE == 24 || TYPE_SIZE == 48
    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

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
#else
    // memset.
    SET_SEGMENT_SSE;
#endif
  }
}

#ifdef SYMBOL_MASK
#ifndef _MSC_VER
__attribute__((target("ssse3")))
#endif
static void CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_ssse3)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0xFF * VALUE_BROADCAST);
#if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0xFE * VALUE_BROADCAST);
#endif

#if TYPE_SIZE == 24
  const __m128i shuffle0 = _mm_set_epi8(0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0);
  const __m128i shuffle1 = _mm_set_epi8(1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1);
  const __m128i shuffle2 = _mm_set_epi8(2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2);

  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
#elif TYPE_SIZE == 48
  const __m128i shuffle0 = _mm_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
  const __m128i shuffle1 = _mm_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4);
  const __m128i shuffle2 = _mm_set_epi8(5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2);

  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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

#ifdef SYMBOL_MASK
    if (symbolIndex)
    {
#if TYPE_SIZE == 24 || TYPE_SIZE == 48
      symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm_shuffle_epi8(symbol, shuffle2);
#endif
    }
#endif

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

    offset -= RLE8_XSYMLUT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_SSE;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - RLE8_XSYMLUT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

#if TYPE_SIZE == 24 || TYPE_SIZE == 48
    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

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
#else
    // memset.
    SET_SEGMENT_SSE;
#endif
  }
}
#endif

#ifndef _MSC_VER
__attribute__((target("sse4.1")))
#endif
static void CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_sse41)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0xFF * VALUE_BROADCAST);
#if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0xFE * VALUE_BROADCAST);
#endif

#if TYPE_SIZE == 24
  const __m128i shuffle0 = _mm_set_epi8(0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0);
  const __m128i shuffle1 = _mm_set_epi8(1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1);
  const __m128i shuffle2 = _mm_set_epi8(2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2);

  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
#elif TYPE_SIZE == 48
  const __m128i shuffle0 = _mm_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
  const __m128i shuffle1 = _mm_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4);
  const __m128i shuffle2 = _mm_set_epi8(5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2);

  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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

#ifdef SYMBOL_MASK
    if (symbolIndex)
    {
#if TYPE_SIZE == 24 || TYPE_SIZE == 48
      symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm_shuffle_epi8(symbol, shuffle2);
#endif
    }
#endif

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

    offset -= RLE8_XSYMLUT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_SSE41;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - RLE8_XSYMLUT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

#if TYPE_SIZE == 24 || TYPE_SIZE == 48
    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

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
#else
    // memset.
    SET_SEGMENT_SSE41;
#endif
  }
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#endif
static void CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_avx)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SYMBOL_MASK
  typedef __m256i simd_t;

  __m256i symbol = _mm256_setzero_si256();
  __m256i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0xFF * VALUE_BROADCAST);
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0xFE * VALUE_BROADCAST);
  #endif
#else
  typedef __m128i simd_t;

  __m128i symbol = _mm_setzero_si128();
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0xFF * VALUE_BROADCAST);
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(0xFE * VALUE_BROADCAST);
  #endif
#endif

#if TYPE_SIZE == 24
  const __m128i shuffle0 = _mm_set_epi8(0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0);
  const __m128i shuffle1 = _mm_set_epi8(1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1);
  const __m128i shuffle2 = _mm_set_epi8(2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2);

  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
#elif TYPE_SIZE == 48
  const __m128i shuffle0 = _mm_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
  const __m128i shuffle1 = _mm_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4);
  const __m128i shuffle2 = _mm_set_epi8(5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2);

  __m128i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
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
#ifndef SYMBOL_MASK
      symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#else
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#endif
      pInStart += (TYPE_SIZE / 8);
      break;
    }

    case 6:
    {
      const simd_t tmp = other[5];
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
      const simd_t tmp = other[4];
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
      const simd_t tmp = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 3:
    {
      const simd_t tmp = other[2];
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
#ifndef SYMBOL_MASK
      symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#else
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#endif
      pInStart += (TYPE_SIZE / 8);
      break;
    }
#endif

    case 2:
    {
      const simd_t tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const simd_t tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

#ifdef SYMBOL_MASK
    if (symbolIndex)
    {
#if TYPE_SIZE == 24 || TYPE_SIZE == 48
      symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm_shuffle_epi8(symbol, shuffle2);
#endif
    }
#endif

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

    offset -= RLE8_XSYMLUT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_AVX;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - RLE8_XSYMLUT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

#if TYPE_SIZE == 24 || TYPE_SIZE == 48
    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

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
#else
    // memset.
    SET_SEGMENT_AVX;
#endif
  }
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static void CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_avx2)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();
  __m256i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0xFF * VALUE_BROADCAST);
#if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(0xFE * VALUE_BROADCAST);
#endif

#if TYPE_SIZE == 24
  const __m256i shuffle0 = _mm256_set_epi8(1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0);
  const __m256i shuffle1 = _mm256_set_epi8(0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2);
  const __m256i shuffle2 = _mm256_set_epi8(2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1);

  __m256i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
#elif TYPE_SIZE == 48
  const __m256i shuffle0 = _mm256_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
  const __m256i shuffle1 = _mm256_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2);
  const __m256i shuffle2 = _mm256_set_epi8(5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4);

  __m256i symbol0 = symbol, symbol1 = symbol, symbol2 = symbol;
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
      symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
      symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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

#ifdef SYMBOL_MASK
    if (symbolIndex)
    {
#if TYPE_SIZE == 24 || TYPE_SIZE == 48
      symbol0 = _mm256_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm256_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm256_shuffle_epi8(symbol, shuffle2);
#endif
    }
#endif

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

    offset -= RLE8_XSYMLUT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_AVX2;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - RLE8_XSYMLUT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

#if TYPE_SIZE == 24 || TYPE_SIZE == 48
    // memset.
    {
      uint8_t *pCOut = pOut;
      uint8_t *pCOutEnd = pOut + symbolCount;

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
#else
    // memset.
    SET_SEGMENT_AVX2;
#endif
  }
}

#ifndef SYMBOL_MASK
#ifndef _MSC_VER
__attribute__((target("avx512f")))
#endif
static void CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_avx512f)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m512i symbol = _mm512_setzero_si512();
  __m512i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(0xFF * VALUE_BROADCAST);
#if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(0xFE * VALUE_BROADCAST);
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
      symbol = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
      symbol = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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

    offset -= RLE8_XSYMLUT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_AVX512;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_MIN_RANGE_SHORT - RLE8_XSYMLUT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

    // memset.
    SET_SEGMENT_AVX512;
  }
}
#endif

//////////////////////////////////////////////////////////////////////////

uint32_t CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[1];
  const size_t expectedOutSize = ((uint32_t *)pIn)[0];
  pIn += sizeof(uint32_t) * 2;

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  _DetectCPUFeatures();

#ifndef SYMBOL_MASK
  if (avx512FSupported)
    CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_avx512f)(pIn, pOut);
  else
#endif
    if (avx2Supported)
    CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_avx2)(pIn, pOut);
  else if (avxSupported)
    CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_avx)(pIn, pOut);
  else if (sse41Supported)
    CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_sse41)(pIn, pOut);
#ifdef SYMBOL_MASK
  else if (ssse3Supported)
    CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_ssse3)(pIn, pOut);
#endif
  else
    CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, decompress_sse)(pIn, pOut);

  return (uint32_t)expectedOutSize;
}

#undef RLE8_XSYMLUT_MIN_RANGE_SHORT
#undef RLE8_XSYMLUT_MIN_RANGE_LONG

#undef RLE8_XSYMLUT_COUNT_BITS
#undef RLE8_XSYMLUT_RANGE_BITS
#undef RLE8_XSYMLUT_MAX_TINY_COUNT
#undef RLE8_XSYMLUT_MAX_TINY_RANGE

#if TYPE_SIZE == 64
  #undef _mm_set1_epi64
  #undef _mm256_set1_epi64
#endif
