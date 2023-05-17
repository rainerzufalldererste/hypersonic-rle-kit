#if SYMBOL_COUNT != 0 || defined(SINGLE)
  #define RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT (1 + 1)
  #ifndef SINGLE
    #define RLE8_XSYMLUT_SHORT_MIN_RANGE_LONG (3 + 4 + 4 + (TYPE_SIZE / 8))
  #else
    #define RLE8_XSYMLUT_SHORT_MIN_RANGE_LONG (3 + 4 + 4)
  #endif
#else
  #define RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT (1 + (TYPE_SIZE / 8) + 1)
  #define RLE8_XSYMLUT_SHORT_MIN_RANGE_LONG (3 + 4 + 4 + (TYPE_SIZE / 8) + 1)
#endif

#if SYMBOL_COUNT == 3
  #define RLE8_XSYMLUT_SHORT_LUT_BITS (2)
  #define RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED (3)
#elif SYMBOL_COUNT == 1
  #define RLE8_XSYMLUT_SHORT_LUT_BITS (1)
  #define RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED (3)
#elif SYMBOL_COUNT == 0
  #define RLE8_XSYMLUT_SHORT_LUT_BITS (0)
  #define RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED (4)
#else
  #define RLE8_XSYMLUT_SHORT_LUT_BITS (3)
  #define RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED (2)
#endif

#define RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED (8 - RLE8_XSYMLUT_SHORT_LUT_BITS - RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED)
#define RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE ((1 << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED) - 1)
#define RLE8_XSYMLUT_SHORT_MAX_PACKED_COUNT ((1 << RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED) - 2)
#define RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID ((1 << RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED) - 1)
#define RLE8_XSYMLUT_SHORT_COUNT_BITS (9)

#if SYMBOL_COUNT != 7
  #define RLE8_XSYMLUT_SHORT_RANGE_BITS (24 - RLE8_XSYMLUT_SHORT_LUT_BITS - RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED - RLE8_XSYMLUT_SHORT_COUNT_BITS)
#else
  #define RLE8_XSYMLUT_SHORT_RANGE_BITS (24 - RLE8_XSYMLUT_SHORT_LUT_BITS - RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED - RLE8_XSYMLUT_SHORT_COUNT_BITS)
#endif

#define RLE8_XSYMLUT_SHORT_MAX_TINY_COUNT ((1 << RLE8_XSYMLUT_SHORT_COUNT_BITS) - 1)
#define RLE8_XSYMLUT_SHORT_MAX_TINY_RANGE ((1 << RLE8_XSYMLUT_SHORT_RANGE_BITS) - 1)
#define RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET (2)
#define RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET (2)

#if TYPE_SIZE == 8
  #if SYMBOL_COUNT > 0
    #define CODEC CONCAT2(SYMBOL_COUNT, symlut_short_)
  #else
    #ifdef SINGLE
      #define CODEC single_short_
    #else  
      #define CODEC multi_short_
    #endif
  #endif
#else
  #ifdef UNBOUND
    #define CODEC_POSTFIX byte_
  #else
    #define CODEC_POSTFIX sym_
  #endif

  #if SYMBOL_COUNT > 0
    #define CODEC CONCAT3(SYMBOL_COUNT, symlut_, CONCAT2(CODEC_POSTFIX, short_))
  #else
    #define CODEC CONCAT2(CODEC_POSTFIX, short_)
  #endif
#endif

#if TYPE_SIZE == 64
  #define _mm_set1_epi64 _mm_set1_epi64x
  #define _mm256_set1_epi64 _mm256_set1_epi64x
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
  CONCAT3(uint, TYPE_SIZE, _t) symbol;
#if SYMBOL_COUNT == 1
  CONCAT3(uint, TYPE_SIZE, _t) lastSymbol;
#elif SYMBOL_COUNT > 0
  CONCAT3(uint, TYPE_SIZE, _t) lastSymbols[SYMBOL_COUNT];
#endif
  int64_t count;
  int64_t lastRLE;
  size_t index;
} CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t));

//////////////////////////////////////////////////////////////////////////

#if !defined(SINGLE) && TYPE_SIZE == 8
static int64_t CONCAT3(rle8_, CODEC, compress_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState);
static int64_t CONCAT3(rle8_, CODEC, compress_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState);
#else
static int64_t CONCAT3(rle8_, CODEC, compress_single_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState);
static int64_t CONCAT3(rle8_, CODEC, compress_single_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState);
#endif

//////////////////////////////////////////////////////////////////////////

inline bool CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(IN const uint8_t *pIn, OUT uint8_t *pOut, const size_t i, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState)
{
#if SYMBOL_COUNT > 1
  size_t symbolMatchIndex = 0;

  for (; symbolMatchIndex < SYMBOL_COUNT; symbolMatchIndex++)
    if (pState->symbol == pState->lastSymbols[symbolMatchIndex])
      break;
#elif SYMBOL_COUNT == 1
  const size_t symbolMatchIndex = (size_t)(pState->symbol != pState->lastSymbol);
#endif

  const int64_t range = i - pState->lastRLE - pState->count + RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;

#if defined(UNBOUND) || TYPE_SIZE == 8
  const int64_t storedCount = pState->count - RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
#else
  const int64_t storedCount = (pState->count / (TYPE_SIZE / 8)) - (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
#endif

  bool isSingleValuePack = false;
  bool is19BitRangeCount = false;

  const uint64_t count3 = storedCount - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
  const uint64_t range3 = range - RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;

  isSingleValuePack = range3 <= RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE && count3 <= RLE8_XSYMLUT_SHORT_MAX_PACKED_COUNT;
  is19BitRangeCount = storedCount <= RLE8_XSYMLUT_SHORT_MAX_TINY_COUNT && range <= RLE8_XSYMLUT_SHORT_MAX_TINY_RANGE;

#if SYMBOL_COUNT != 0
  int64_t penalty = (size_t)(symbolMatchIndex == SYMBOL_COUNT);
#else
  int64_t penalty = 0;
#endif

  if (!isSingleValuePack)
  {
    penalty += 2;

    if (!is19BitRangeCount)
      penalty += (range <= 0xFFFFF ? (range <= RLE8_XSYMLUT_SHORT_MAX_TINY_RANGE ? 0 : 2) : 4) + (storedCount <= 0xFFFFF ? (storedCount <= (RLE8_XSYMLUT_SHORT_MAX_TINY_COUNT) ? 0 : 2) : 4);
  }

  if (pState->count >= RLE8_XSYMLUT_SHORT_MIN_RANGE_LONG || (pState->count >= RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT + penalty))
  {
#if SYMBOL_COUNT > 0
    switch (symbolMatchIndex)
    {
  #if SYMBOL_COUNT != 1
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

    #else
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
  #else // 1 sym lut.
    case 1:
      pState->lastSymbol = pState->symbol;
      break;
  #endif

    case 0:
      break;
    }
#endif

    if (isSingleValuePack)
    {
#if SYMBOL_COUNT != 0
      const uint8_t valuePack8 = (uint8_t)(symbolMatchIndex << (RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED + RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED)) | ((uint8_t)count3 << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED) | (uint8_t)(range3);
#else
      const uint8_t valuePack8 = (uint8_t)((uint8_t)count3 << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED) | (uint8_t)(range3);
#endif

      pOut[pState->index] = valuePack8;
      pState->index++;
    }
    else
    {
      uint16_t storedCountX;
      uint16_t rangeX;

      if (storedCount <= RLE8_XSYMLUT_SHORT_MAX_TINY_COUNT)
        storedCountX = (uint16_t)storedCount;
      else if (storedCount <= 0xFFFF)
        storedCountX = 1;
      else
        storedCountX = 0;

      if (range <= RLE8_XSYMLUT_SHORT_MAX_TINY_RANGE)
        rangeX = (uint16_t)range;
      else if (range <= 0xFFFF)
        rangeX = 1;
      else
        rangeX = 0;

#if SYMBOL_COUNT != 0
  #if RLE8_XSYMLUT_SHORT_RANGE_BITS != 8
      const uint8_t valuePack1 = (uint8_t)(symbolMatchIndex << (RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED + RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED)) | ((uint8_t)(RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED)) | (uint8_t)((storedCountX << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) >> 8);
      const uint8_t valuePack2 = (uint8_t)((storedCountX << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) | (rangeX >> 8));
  #else
      const uint8_t valuePack1 = (uint8_t)(symbolMatchIndex << (RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED + RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED)) | ((uint8_t)(RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED)) | (uint8_t)(storedCountX >> 8);
      const uint8_t valuePack2 = (uint8_t)(storedCountX);
  #endif
#else
  #if RLE8_XSYMLUT_SHORT_RANGE_BITS != 8
      const uint8_t valuePack1 = ((uint8_t)(RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED)) | (uint8_t)((storedCountX << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) >> 8);
      const uint8_t valuePack2 = (uint8_t)((storedCountX << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) | (rangeX >> 8));
  #else
      const uint8_t valuePack1 = ((uint8_t)(RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED)) | (uint8_t)(storedCountX >> 8);
      const uint8_t valuePack2 = (uint8_t)(storedCountX);
  #endif
#endif

      pOut[pState->index] = valuePack1;
      pState->index++;
      pOut[pState->index] = valuePack2;
      pState->index++;
      pOut[pState->index] = (uint8_t)rangeX;
      pState->index++;

      if (storedCount != storedCountX)
      {
        if (storedCount <= 0xFFFF)
        {
          *((uint16_t *)&pOut[pState->index]) = (uint16_t)storedCount;
          pState->index += sizeof(uint16_t);
        }
        else
        {
          *((uint32_t *)&pOut[pState->index]) = (uint32_t)storedCount;
          pState->index += sizeof(uint32_t);
        }
      }

      if (range != rangeX)
      {
        if (range <= 0xFFFF)
        {
          *((uint16_t *)&pOut[pState->index]) = (uint16_t)range;
          pState->index += sizeof(uint16_t);
        }
        else
        {
          *((uint32_t *)&pOut[pState->index]) = (uint32_t)range;
          pState->index += sizeof(uint32_t);
        }
      }
    }

#ifndef SINGLE
  #if SYMBOL_COUNT != 0
    if (symbolMatchIndex == SYMBOL_COUNT)
  #endif
    {
      *((CONCAT3(uint, TYPE_SIZE, _t) *)&(pOut[pState->index])) = pState->symbol;
      pState->index += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
    }
#endif

    const size_t copySize = i - pState->count - pState->lastRLE;

    memcpy(pOut + pState->index, pIn + pState->lastRLE, copySize);
    pState->index += copySize;

    pState->lastRLE = i;
    pState->index = pState->index;

    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////

#if TYPE_SIZE == 8
uint32_t CONCAT3(rle8_, CODEC, compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  ((uint32_t *)pOut)[0] = (uint32_t)inSize;

  CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) state;
  memset(&state, 0, sizeof(state));
  state.index = sizeof(uint32_t) * 2;

#if SYMBOL_COUNT != 0
  #if SYMBOL_COUNT != 1
  state.lastSymbols[0] = 0x00;
  state.lastSymbols[1] = 0x7F;
  state.lastSymbols[2] = 0xFF;
    #if SYMBOL_COUNT == 7
  state.lastSymbols[3] = 0x01;
  state.lastSymbols[4] = 0x7E;
  state.lastSymbols[5] = 0x80;
  state.lastSymbols[6] = 0xFE;
    #endif
  #else
  state.lastSymbol = 0;
  #endif
#endif

  _DetectCPUFeatures();

#ifndef SINGLE
  state.symbol = ~(*pIn);
#else
  uint8_t rle8_single_compress_get_approx_optimal_symbol_sse2(IN const uint8_t * pIn, const size_t inSize);
  uint8_t rle8_single_compress_get_approx_optimal_symbol_avx2(IN const uint8_t * pIn, const size_t inSize);

  // The AVX2 variant appears to be slower, so we're just always calling the SSE2 version.
  //if (avx2Supported)
  //  state.symbol = rle8_single_compress_get_approx_optimal_symbol_avx2(pIn, inSize);
  //else
  state.symbol = rle8_single_compress_get_approx_optimal_symbol_sse2(pIn, inSize);

  pOut[state.index] = state.symbol;
  state.index++;
#endif

  size_t i = 0;

#ifndef SINGLE
  if (avx2Supported)
    i = CONCAT3(rle8_, CODEC, compress_avx2)(pIn, inSize, pOut, &state);
  else
    i = CONCAT3(rle8_, CODEC, compress_sse2)(pIn, inSize, pOut, &state);
#else
  // The AVX2 variant appears to be slower, so we're just always calling the SSE2 version.
  //if (avx2Supported)
  //  i = CONCAT3(rle8_, CODEC, compress_single_avx2)(pIn, inSize, pOut, &state);
  //else
  i = CONCAT3(rle8_, CODEC, compress_single_sse2)(pIn, inSize, pOut, &state);
#endif

  for (; i < inSize; i++)
  {
    if (pIn[i] == state.symbol)
    {
      state.count++;
    }
    else
    {
      CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, &state);

#ifndef SINGLE
      state.symbol = pIn[i];
      state.count = 1;
#else
      state.count = (pIn[i] == state.symbol);
#endif
    }
  }

  // Copy / Encode remaining bytes.
  {
    const int64_t range = i - state.lastRLE - state.count + 2;

    if (CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, &state))
    {
      pOut[state.index] = (RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
      state.index++;
#if SYMBOL_COUNT == 3
      pOut[state.index] = 0b00000100;
#elif SYMBOL_COUNT == 1
      pOut[state.index] = 0b00001000;
#elif SYMBOL_COUNT == 0
      pOut[state.index] = 0b00001000;
#else
      pOut[state.index] = 0b00000010;
#endif
      state.index++;
      pOut[state.index] = 1;
      state.index++;
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);

#if SYMBOL_COUNT == 0 && !defined(SINGLE)
      pOut[state.index] = 0;
      state.index++;
#endif
    }
    else
    {
      pOut[state.index] = (RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
      state.index++;
#if SYMBOL_COUNT == 3
      pOut[state.index] = 0b00000100;
#elif SYMBOL_COUNT == 1
      pOut[state.index] = 0b00001000;
#elif SYMBOL_COUNT == 0
      pOut[state.index] = 0b00001000;
#else
      pOut[state.index] = 0b00000010;
#endif
      state.index++;
      pOut[state.index] = 0;
      state.index++;
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint32_t *)&pOut[state.index]) = (uint32_t)range;
      state.index += sizeof(uint32_t);

#if SYMBOL_COUNT == 0 && !defined(SINGLE)
      pOut[state.index] = 0;
      state.index++;
#endif

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

#if !defined(SINGLE) && TYPE_SIZE == 8
static int64_t CONCAT3(rle8_, CODEC, compress_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState)
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

        CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, pState);
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
static int64_t CONCAT3(rle8_, CODEC, compress_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState)
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

        CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, pState);
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
uint32_t CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, compress))(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  ((uint32_t *)pOut)[0] = (uint32_t)inSize;

  CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) state;
  memset(&state, 0, sizeof(state));
  state.index = sizeof(uint32_t) * 2;

  typedef CONCAT3(uint, TYPE_SIZE, _t) symbol_t;

#if SYMBOL_COUNT != 0
#if SYMBOL_COUNT != 1
  state.lastSymbols[0] = 0x00 * VALUE_BROADCAST;
  state.lastSymbols[1] = 0x7F * VALUE_BROADCAST;
  state.lastSymbols[2] = 0xFF * VALUE_BROADCAST;
#if SYMBOL_COUNT == 7
  state.lastSymbols[3] = 0x01 * VALUE_BROADCAST;
  state.lastSymbols[4] = 0x7E * VALUE_BROADCAST;
  state.lastSymbols[5] = 0x80 * VALUE_BROADCAST;
  state.lastSymbols[6] = 0xFE * VALUE_BROADCAST;
#endif
#else
  state.lastSymbol = 0;
#endif
#endif

  state.symbol = ~(*((symbol_t *)(pIn)));

  size_t i = 0;

  while (i < inSize)
  {
    if (state.count)
    {
      if (i + (TYPE_SIZE / 8) <= inSize)
      {
        const symbol_t next = *(symbol_t *)&pIn[i];

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
#elif TYPE_SIZE == 32
          const symbol_t diff = state.symbol ^ *(symbol_t *)&pIn[i];

#ifdef _MSC_VER
          unsigned long offset;
          _BitScanForward(&offset, diff);
#else
          const uint32_t offset = __builtin_ctz(diff);
#endif

          i += (offset / 8);
          state.count += (offset / 8);
#elif TYPE_SIZE == 64
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
      CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, &state);

      state.symbol = *(symbol_t *)(&pIn[i]);

      if (i + (TYPE_SIZE / 8) <= inSize && ((symbol_t *)(&pIn[i]))[1] == state.symbol)
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

    if (CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, &state))
    {
      pOut[state.index] = (RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
      state.index++;
#if SYMBOL_COUNT == 3
      pOut[state.index] = 0b00000100;
#elif SYMBOL_COUNT == 1
      pOut[state.index] = 0b00001000;
#elif SYMBOL_COUNT == 0
      pOut[state.index] = 0b00001000;
#else
      pOut[state.index] = 0b00000010;
#endif
      state.index++;
      pOut[state.index] = 1;
      state.index++;
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);

#if SYMBOL_COUNT == 0 && !defined(SINGLE)
      pOut[state.index] = 0;
      state.index++;
#endif
  }
    else
    {
      pOut[state.index] = (RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID << RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
      state.index++;
#if SYMBOL_COUNT == 3
      pOut[state.index] = 0b00000100;
#elif SYMBOL_COUNT == 1
      pOut[state.index] = 0b00001000;
#elif SYMBOL_COUNT == 0
      pOut[state.index] = 0b00001000;
#else
      pOut[state.index] = 0b00000010;
#endif
      state.index++;
      pOut[state.index] = 0;
      state.index++;
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint32_t *)&pOut[state.index]) = (uint32_t)range;
      state.index += sizeof(uint32_t);

#if SYMBOL_COUNT == 0 && !defined(SINGLE)
      *((symbol_t *)&(pOut[state.index])) = 0;
      state.index += (TYPE_SIZE / 8);
#endif

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

#ifdef SINGLE
static int64_t CONCAT3(rle8_, CODEC, compress_single_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState)
{
  int64_t i = 0;

  const __m128i symbol128 = _mm_set1_epi8(pState->symbol);
  const int64_t endInSize128 = inSize - sizeof(symbol128);

  for (; i < endInSize128; i++)
  {
    const uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(symbol128, _mm_loadu_si128((const __m128i *) & (pIn[i]))));

    if (0xFFFF == mask)
    {
      pState->count += sizeof(symbol128);
      i += sizeof(symbol128) - 1;
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

        CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, pState);
      }

      pState->count = 0;

      while (i < endInSize128)
      {
        const int32_t cmp = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128((const __m128i *)(&pIn[i])), symbol128));

        if (cmp == 0 || ((cmp & 0x8000) == 0 && __builtin_popcount((uint32_t)cmp) < RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT))
        {
          i += sizeof(symbol128);
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
          pState->count = 1;
          break;
        }
      }
    }
  }

  return i;
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static int64_t CONCAT3(rle8_, CODEC, compress_single_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState)
{
  int64_t i = 0;

  const __m256i symbol256 = _mm256_set1_epi8(pState->symbol);
  const int64_t endInSize256 = inSize - sizeof(symbol256);

  for (; i < endInSize256; i++)
  {
    const uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(symbol256, _mm256_loadu_si256((const __m256i *) & (pIn[i]))));

    if (0xFFFFFFFF == mask)
    {
      pState->count += sizeof(symbol256);
      i += sizeof(symbol256) - 1;
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

        CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, pState);
      }

      pState->count = 0;

      while (i < endInSize256)
      {
        const int32_t cmp = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(&pIn[i])), symbol256));

        if (cmp == 0 || ((cmp & 0x80000000) == 0 && __builtin_popcount((uint32_t)cmp) < RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT))
        {
          i += sizeof(symbol256);
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
          pState->count = 1;
          break;
        }
      }
    }
  }

  return i;
}
#endif

//////////////////////////////////////////////////////////////////////////

static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_sse))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m128i symbol = _mm_setzero_si128();
#else
  const __m128i symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
  pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

#if SYMBOL_COUNT > 1
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0xFF * VALUE_BROADCAST);
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0xFE * VALUE_BROADCAST);
  #endif
#endif

  while (true)
  {
    const uint8_t packedValue1 = *pInStart;
    pInStart++;

#if SYMBOL_COUNT != 0
    const uint8_t symbolIndex = packedValue1 >> (RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED + RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
#endif

    const uint8_t count3 = (packedValue1 >> RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED) & RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID;

    if (count3 == RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID) // don't use 3 bit value.
    {
      const uint8_t packedValue2 = *pInStart;
      pInStart++;

      const uint8_t packedValue3 = *pInStart;
      pInStart++;

#if RLE8_XSYMLUT_SHORT_RANGE_BITS != 8
      symbolCount = ((packedValue2 >> (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8))) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8 - (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)));
      offset = packedValue3 | ((uint16_t)(packedValue2 & ((1 << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) - 1)) << 8);
#else
      symbolCount = (packedValue2) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8));
      offset = packedValue3;
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
    }
    else
    {
      symbolCount = count3 + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
      offset = (packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE) + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
    }

#if SYMBOL_COUNT != 0
    switch (symbolIndex)
    {
#if SYMBOL_COUNT != 1
  #if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }

    case 6:
    case 5:
    case 4:
    case 3:
    {
      const __m128i tmp = other[symbolIndex - 1];

      for (size_t q = symbolIndex - 1; q > 0; q--)
        other[q] = other[q - 1];

      other[0] = symbol;
      symbol = tmp;
      break;
    }
  #else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
    symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
    pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

    offset -= RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_SSE;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

    // memset.
    SET_SEGMENT_SSE;
  }
}

#ifndef _MSC_VER
__attribute__((target("sse4.1")))
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_sse41))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m128i symbol = _mm_setzero_si128();
#else
  const __m128i symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
  pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

#if SYMBOL_COUNT > 1
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0xFF * VALUE_BROADCAST);
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm_set1_epi, TYPE_SIZE)(0xFE * VALUE_BROADCAST);
  #endif
#endif

  while (true)
  {
    const uint8_t packedValue1 = *pInStart;
    pInStart++;

#if SYMBOL_COUNT != 0
    const uint8_t symbolIndex = packedValue1 >> (RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED + RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
#endif

    const uint8_t count3 = (packedValue1 >> RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED) & RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID;

    if (count3 == RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID) // don't use 3 bit value.
    {
      const uint8_t packedValue2 = *pInStart;
      pInStart++;

      const uint8_t packedValue3 = *pInStart;
      pInStart++;

#if RLE8_XSYMLUT_SHORT_RANGE_BITS != 8
      symbolCount = ((packedValue2 >> (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8))) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8 - (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)));
      offset = packedValue3 | ((uint16_t)(packedValue2 & ((1 << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) - 1)) << 8);
#else
      symbolCount = (packedValue2) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8));
      offset = packedValue3;
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
    }
    else
    {
      symbolCount = count3 + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
      offset = (packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE) + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
    }

#if SYMBOL_COUNT != 0
    switch (symbolIndex)
    {
#if SYMBOL_COUNT != 1
  #if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }

    case 6:
    case 5:
    case 4:
    case 3:
    {
      const __m128i tmp = other[symbolIndex - 1];

      for (size_t q = symbolIndex - 1; q > 0; q--)
        other[q] = other[q - 1];

      other[0] = symbol;
      symbol = tmp;
      break;
    }
  #else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
    symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
    pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

    offset -= RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_SSE41;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

    // memset.
    SET_SEGMENT_SSE41;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m256i symbol = _mm256_setzero_si256();
#else
  const __m256i symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
  pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

#if SYMBOL_COUNT > 1
  __m256i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0xFF * VALUE_BROADCAST);
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0xFE * VALUE_BROADCAST);
  #endif
#endif

  while (true)
  {
    const uint8_t packedValue1 = *pInStart;
    pInStart++;

#if SYMBOL_COUNT != 0
    const uint8_t symbolIndex = packedValue1 >> (RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED + RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
#endif

    const uint8_t count3 = (packedValue1 >> RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED) & RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID;

    if (count3 == RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID) // don't use 3 bit value.
    {
      const uint8_t packedValue2 = *pInStart;
      pInStart++;

      const uint8_t packedValue3 = *pInStart;
      pInStart++;

#if RLE8_XSYMLUT_SHORT_RANGE_BITS != 8
      symbolCount = ((packedValue2 >> (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8))) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8 - (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)));
      offset = packedValue3 | ((uint16_t)(packedValue2 & ((1 << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) - 1)) << 8);
#else
      symbolCount = (packedValue2) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8));
      offset = packedValue3;
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
    }
    else
    {
      symbolCount = count3 + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
      offset = (packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE) + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
    }

#if SYMBOL_COUNT != 0
    switch (symbolIndex)
    {
#if SYMBOL_COUNT != 1
  #if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }

    case 6:
    case 5:
    case 4:
    case 3:
    {
      const __m256i tmp = other[symbolIndex - 1];

      for (size_t q = symbolIndex - 1; q > 0; q--)
        other[q] = other[q - 1];

      other[0] = symbol;
      symbol = tmp;
      break;
    }
  #else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
    symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
    pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

    offset -= RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_AVX;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

    // memset.
    SET_SEGMENT_AVX;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx2))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m256i symbol = _mm256_setzero_si256();
#else
  const __m256i symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
  pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

#if SYMBOL_COUNT > 1
  __m256i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0xFF * VALUE_BROADCAST);
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(0xFE * VALUE_BROADCAST);
  #endif
#endif

  while (true)
  {
    const uint8_t packedValue1 = *pInStart;
    pInStart++;

#if SYMBOL_COUNT != 0
    const uint8_t symbolIndex = packedValue1 >> (RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED + RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
#endif

    const uint8_t count3 = (packedValue1 >> RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED) & RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID;

    if (count3 == RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID) // don't use 3 bit value.
    {
      const uint8_t packedValue2 = *pInStart;
      pInStart++;

      const uint8_t packedValue3 = *pInStart;
      pInStart++;

#if RLE8_XSYMLUT_SHORT_RANGE_BITS != 8
      symbolCount = ((packedValue2 >> (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8))) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8 - (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)));
      offset = packedValue3 | ((uint16_t)(packedValue2 & ((1 << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) - 1)) << 8);
#else
      symbolCount = (packedValue2) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8));
      offset = packedValue3;
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
    }
    else
    {
      symbolCount = count3 + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
      offset = (packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE) + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
    }

#if SYMBOL_COUNT != 0
    switch (symbolIndex)
    {
#if SYMBOL_COUNT != 1
  #if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }

    case 6:
    case 5:
    case 4:
    case 3:
    {
      const __m256i tmp = other[symbolIndex - 1];

      for (size_t q = symbolIndex - 1; q > 0; q--)
        other[q] = other[q - 1];

      other[0] = symbol;
      symbol = tmp;
      break;
    }
  #else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
    symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
    pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

    offset -= RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_AVX2;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

    // memset.
    SET_SEGMENT_AVX2;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx512f")))
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx512f))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m512i symbol = _mm512_setzero_si512();
#else
  const __m512i symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
  pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

#if SYMBOL_COUNT > 1
  __m512i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(0x7F * VALUE_BROADCAST);
  other[1] = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(0xFF * VALUE_BROADCAST);
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(0x01 * VALUE_BROADCAST);
  other[3] = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(0x7E * VALUE_BROADCAST);
  other[4] = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(0x80 * VALUE_BROADCAST);
  other[5] = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(0xFE * VALUE_BROADCAST);
  #endif
#endif

  while (true)
  {
    const uint8_t packedValue1 = *pInStart;
    pInStart++;

#if SYMBOL_COUNT != 0
    const uint8_t symbolIndex = packedValue1 >> (RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED + RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED);
#endif

    const uint8_t count3 = (packedValue1 >> RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED) & RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID;

    if (count3 == RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID) // don't use 3 bit value.
    {
      const uint8_t packedValue2 = *pInStart;
      pInStart++;

      const uint8_t packedValue3 = *pInStart;
      pInStart++;

#if RLE8_XSYMLUT_SHORT_RANGE_BITS != 8
      symbolCount = ((packedValue2 >> (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8))) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8 - (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)));
      offset = packedValue3 | ((uint16_t)(packedValue2 & ((1 << (RLE8_XSYMLUT_SHORT_RANGE_BITS - 8)) - 1)) << 8);
#else
      symbolCount = (packedValue2) | (((uint16_t)(packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE)) << (8));
      offset = packedValue3;
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
    }
    else
    {
      symbolCount = count3 + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
      offset = (packedValue1 & RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE) + RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET;
    }

#if SYMBOL_COUNT != 0
    switch (symbolIndex)
    {
#if SYMBOL_COUNT != 1
  #if SYMBOL_COUNT == 7
    case 7:
    {
      other[5] = other[4];
      other[4] = other[3];
      other[3] = other[2];
      other[2] = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }

    case 6:
    case 5:
    case 4:
    case 3:
    {
      const __m512i tmp = other[symbolIndex - 1];

      for (size_t q = symbolIndex - 1; q > 0; q--)
        other[q] = other[q - 1];

      other[0] = symbol;
      symbol = tmp;
      break;
    }
  #else
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
      pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
      break;
    }
#endif

    case 0:
      break;
    }

#elif !defined(SINGLE)
    symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(CONCAT3(uint, TYPE_SIZE, _t) *)pInStart);
    pInStart += sizeof(CONCAT3(uint, TYPE_SIZE, _t));
#endif

    offset -= RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;

    // memcpy.
    COPY_SEGMENT_AVX512;

    if (!symbolCount)
      return;

#if defined(UNBOUND) || TYPE_SIZE == 8
    symbolCount += (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET);
#else
    symbolCount = (symbolCount + (RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT / (TYPE_SIZE / 8)) - RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET) * (TYPE_SIZE / 8);
#endif

    // memset.
    SET_SEGMENT_AVX512;
  }
}

//////////////////////////////////////////////////////////////////////////

#if TYPE_SIZE == 8
uint32_t CONCAT3(rle8_, CODEC, decompress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
#else
uint32_t CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress))(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
#endif
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
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx512f))(pIn, pOut);
  else if (avx2Supported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx2))(pIn, pOut);
  else if (avxSupported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx))(pIn, pOut);
  else if (sse41Supported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_sse41))(pIn, pOut);
  else
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_sse))(pIn, pOut);

  return (uint32_t)expectedOutSize;
}

#undef RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT
#undef RLE8_XSYMLUT_SHORT_MIN_RANGE_LONG

#undef RLE8_XSYMLUT_SHORT_LUT_BITS
#undef RLE8_XSYMLUT_SHORT_COUNT_BITS_PACKED

#undef RLE8_XSYMLUT_SHORT_RANGE_BITS_PACKED
#undef RLE8_XSYMLUT_SHORT_MAX_PACKED_RANGE
#undef RLE8_XSYMLUT_SHORT_MAX_PACKED_COUNT
#undef RLE8_XSYMLUT_SHORT_PACKED_COUNT_INVALID
#undef RLE8_XSYMLUT_SHORT_COUNT_BITS
#undef RLE8_XSYMLUT_SHORT_RANGE_BITS
#undef RLE8_XSYMLUT_SHORT_MAX_TINY_COUNT
#undef RLE8_XSYMLUT_SHORT_MAX_TINY_RANGE
#undef RLE8_XSYMLUT_SHORT_COUNT_VALUE_OFFSET
#undef RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET

#undef CODEC

#if TYPE_SIZE == 64
  #undef _mm_set1_epi64
  #undef _mm256_set1_epi64
#endif
