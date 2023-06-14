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
  #define VALUE_BROADCAST_INTERNAL 0x01
#elif TYPE_SIZE == 16
  #define VALUE_BROADCAST_INTERNAL 0x0101
#elif TYPE_SIZE == 24
  #define VALUE_BROADCAST_INTERNAL 0x010101
#elif TYPE_SIZE == 32
  #define VALUE_BROADCAST_INTERNAL 0x01010101
#elif TYPE_SIZE == 48
  #define VALUE_BROADCAST_INTERNAL 0x010101010101ULL
#elif TYPE_SIZE == 64
  #define VALUE_BROADCAST_INTERNAL 0x0101010101010101ULL
#endif

#define VALUE_BROADCAST ((uintXX_t)(VALUE_BROADCAST_INTERNAL))

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
#if SYMBOL_COUNT == 1
  uintXX_t lastSymbol;
#elif SYMBOL_COUNT > 0
  uintXX_t lastSymbols[SYMBOL_COUNT];
#endif
  int64_t count;
  int64_t lastRLE;
  size_t index;
} CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t));

//////////////////////////////////////////////////////////////////////////

#if !defined(SINGLE) && TYPE_SIZE == 8
static int64_t CONCAT3(rle8_, CODEC, compress_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState);
static int64_t CONCAT3(rle8_, CODEC, compress_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState);
#elif defined(SINGLE)
static int64_t CONCAT3(rle8_, CODEC, compress_single_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState);
//static int64_t CONCAT3(rle8_, CODEC, compress_single_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState);
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
inline
#endif
bool CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(IN const uint8_t *pIn, OUT uint8_t *pOut, const size_t i, IN OUT CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) *pState)
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
  int64_t penalty = (size_t)(symbolMatchIndex == SYMBOL_COUNT) * (TYPE_SIZE / 8);
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
      #elif !defined(_MSC_VER)
      __attribute__((fallthrough));
      #endif

    case 5:
      pState->lastSymbols[5] = pState->lastSymbols[4];
      #ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
      #elif !defined(_MSC_VER)
      __attribute__((fallthrough));
      #endif

    case 4:
      pState->lastSymbols[4] = pState->lastSymbols[3];
      #ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
      #elif !defined(_MSC_VER)
      __attribute__((fallthrough));
      #endif

    case 3:
      pState->lastSymbols[3] = pState->lastSymbols[2];
      #ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
      #elif !defined(_MSC_VER)
      __attribute__((fallthrough));
      #endif

    #else
    case 3:
    #endif
    case 2:
      pState->lastSymbols[2] = pState->lastSymbols[1];
    #ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
    #elif !defined(_MSC_VER)
      __attribute__((fallthrough));
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
      *((uintXX_t *)&(pOut[pState->index])) = pState->symbol;
      pState->index += (TYPE_SIZE / 8);
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
// from `rle8_extreme`:
uint8_t rle8_single_compress_get_approx_optimal_symbol_sse2(IN const uint8_t *pIn, const size_t inSize);
uint8_t rle8_single_compress_get_approx_optimal_symbol_avx2(IN const uint8_t *pIn, const size_t inSize);

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
      CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, &state);

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
    if (CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, &state))
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
      const size_t copySize = i - state.lastRLE;

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
      *((uint32_t *)&pOut[state.index]) = (uint32_t)copySize + RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;
      state.index += sizeof(uint32_t);

#if SYMBOL_COUNT == 0 && !defined(SINGLE)
      pOut[state.index] = 0;
      state.index++;
#endif

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

        CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, pState);
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

        CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, pState);
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

#define IMPL_SSE2
  #include "rleX_Xsl_short_multibyte_encoder.h"
#undef IMPL_SSE2

#ifdef SYMBOL_MASK
  #define IMPL_SSSE3
    #include "rleX_Xsl_short_multibyte_encoder.h"
  #undef IMPL_SSSE3
#endif

#define IMPL_AVX2
  #include "rleX_Xsl_short_multibyte_encoder.h"
#undef IMPL_AVX2

#include "rleX_Xsl_short_multibyte_encoder.h"

uint32_t CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, compress))(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  _DetectCPUFeatures();

  if (avx2Supported)
    return CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, compress_avx2))(pIn, inSize, pOut, outSize);
  #ifdef SYMBOL_MASK
  else if (ssse3Supported)
    return CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, compress_ssse3))(pIn, inSize, pOut, outSize);
  #endif
  else if (sse2Supported)
    return CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, compress_sse2))(pIn, inSize, pOut, outSize);
  else
    return CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, compress_base))(pIn, inSize, pOut, outSize);
}

  #if defined(UNBOUND) && SYMBOL_COUNT > 0

#ifdef _MSC_VER
inline
#endif
size_t CONCAT3(rleX_Xsl_short_get_match_length, CODEC, TYPE_SIZE)(const uintXX_t a, const uintXX_t b)
{
  if (a == b)
    return (TYPE_SIZE / 8);

#if TYPE_SIZE == 16
  if ((a & 0xFF) == (b & 0xFF))
    return 1;

  return 0;
#elif TYPE_SIZE == 32 || TYPE_SIZE == 24
  const uintXX_t diff = a ^ b;

#ifdef _MSC_VER
  unsigned long offset;
  _BitScanForward(&offset, diff);
#else
  const uint32_t offset = __builtin_ctz(diff);
#endif

  return (offset / 8);
#elif TYPE_SIZE == 64 || TYPE_SIZE == 48
  const uintXX_t diff = a ^ b;

#ifdef _MSC_VER
  unsigned long offset;
  _BitScanForward64(&offset, diff);
#else
  const uint32_t offset = __builtin_ctzl(diff);
#endif

  return (offset / 8);
#else // backup
#fail NOT IMPLEMENTED
#endif
}

uint32_t CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, compress_greedy))(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
    return 0;

  ((uint32_t *)pOut)[0] = (uint32_t)inSize;

  CONCAT3(rle8, TYPE_SIZE, CONCAT3(_, CODEC, compress_state_t)) state;
  memset(&state, 0, sizeof(state));
  state.index = sizeof(uint32_t) * 2;

  typedef uintXX_t symbol_t;

#if SYMBOL_COUNT != 0
#if SYMBOL_COUNT != 1
  state.lastSymbols[0] = (uintXX_t)(0x00 * VALUE_BROADCAST);
  state.lastSymbols[1] = (uintXX_t)(0x7F * VALUE_BROADCAST);
  state.lastSymbols[2] = (uintXX_t)(0xFF * VALUE_BROADCAST);
#if SYMBOL_COUNT == 7
  state.lastSymbols[3] = (uintXX_t)(0x01 * VALUE_BROADCAST);
  state.lastSymbols[4] = (uintXX_t)(0x7E * VALUE_BROADCAST);
  state.lastSymbols[5] = (uintXX_t)(0x80 * VALUE_BROADCAST);
  state.lastSymbols[6] = (uintXX_t)(0xFE * VALUE_BROADCAST);
#endif
#else
  state.lastSymbol = 0;
#endif
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
          uint8_t symBytes[TYPE_SIZE / 8];
          memcpy(symBytes, &state.symbol, TYPE_SIZE / 8);

          for (size_t j = 0; j < (TYPE_SIZE / 8 - 1); j++) // can't reach the absolute max.
          {
            if (pIn[i + j] != symBytes[j])
              break;

            state.count++;
            i++;
          }
#endif
        }
#endif
      }
    }

#if TYPE_SIZE != 16
    not_a_full_match_but_a_match:
#endif
    {
      CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, &state);

#ifndef SYMBOL_MASK
      state.symbol = *(symbol_t *)(&pIn[i]);
#else
      state.symbol = *(symbol_t *)(&pIn[i]) & SYMBOL_MASK;
#endif

      const bool symbolWouldFit = i + (TYPE_SIZE / 8) <= inSize;

#ifndef SYMBOL_MASK
      if (symbolWouldFit && ((symbol_t *)(&pIn[i]))[1] == state.symbol)
#else
      if (symbolWouldFit && (*((symbol_t *)(&pIn[i + (TYPE_SIZE / 8)])) & SYMBOL_MASK) == state.symbol)
#endif
      {
        state.count = (TYPE_SIZE / 8) * 2;
        i += (TYPE_SIZE / 8) * 2;
      }
      else
      {
        if (symbolWouldFit)
        {
#ifndef SYMBOL_MASK
          const symbol_t next = *(symbol_t *)&pIn[i];
#else
          const symbol_t next = *(symbol_t *)&pIn[i] & SYMBOL_MASK;
#endif

          size_t possibleCount = 0;

#if SYMBOL_COUNT == 1
#if TYPE_SIZE != 16
          possibleCount = CONCAT3(rleX_Xsl_short_get_match_length, CODEC, TYPE_SIZE)(state.lastSymbol, next);
#else
          if (state.lastSymbol == next)
            possibleCount = 2;
#endif
#else
          size_t possibleIndex = 0;

          for (size_t j = 0; j < SYMBOL_COUNT; j++)
          {
#if TYPE_SIZE != 16
            size_t possibleSymbolCount = 0;

            if (next == state.lastSymbols[j])
            {
              possibleIndex = j;
              possibleCount = (TYPE_SIZE / 8);
              break;
            }
            else
            {
#if TYPE_SIZE == 32 || TYPE_SIZE == 24
              const uintXX_t diff = next ^ state.lastSymbols[j];

#ifdef _MSC_VER
              unsigned long offset;
              _BitScanForward(&offset, diff);
#else
              const uint32_t offset = __builtin_ctz(diff);
#endif

              possibleSymbolCount = (offset / 8);
#elif TYPE_SIZE == 64 || TYPE_SIZE == 48
              const uintXX_t diff = next ^ state.lastSymbols[j];

#ifdef _MSC_VER
              unsigned long offset;
              _BitScanForward64(&offset, diff);
#else
              const uint32_t offset = __builtin_ctzl(diff);
#endif

              possibleSymbolCount = (offset / 8);
#else // backup
              #fail NOT IMPLEMENTED
#endif

              if (possibleSymbolCount > possibleCount)
              {
                possibleIndex = j;
                possibleCount = possibleSymbolCount;
              }
            }
#else
            if (state.lastSymbols[j] == next)
            {
              possibleCount = 2;
              possibleIndex = j;
              break;
            }
#endif
          }
#endif

#if TYPE_SIZE != 16
          if (possibleCount >= RLE8_XSYMLUT_SHORT_MIN_RANGE_SHORT)
#else
          if (possibleCount)
#endif
          {

            state.count = possibleCount;
            i += possibleCount;
#if SYMBOL_COUNT == 1
            state.symbol = state.lastSymbol;
#else
            state.symbol = state.lastSymbols[possibleIndex];
#endif

#if TYPE_SIZE != 16
            if (state.count < TYPE_SIZE / 8)
              goto not_a_full_match_but_a_match;
#endif
          }
          else
          {
            state.count = 0;
            i++;
          }
        }
        else
        {
          state.count = 0;
          i++;
        }
      }
    }
  }

  // Copy / Encode remaining bytes.
  {
    if (CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, &state))
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
      const size_t copySize = i - state.lastRLE;

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
      *((uint32_t *)&pOut[state.index]) = (uint32_t)copySize + RLE8_XSYMLUT_SHORT_RANGE_VALUE_OFFSET;
      state.index += sizeof(uint32_t);

#if SYMBOL_COUNT == 0 && !defined(SINGLE)
      *((symbol_t *)&(pOut[state.index])) = 0;
      state.index += (TYPE_SIZE / 8);
#endif

      memcpy(pOut + state.index, pIn + state.lastRLE, copySize);
      state.index += copySize;
    }
  }

  // Store compressed length.
  return ((uint32_t *)pOut)[1] = (uint32_t)state.index;
}
  #endif
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

        CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, pState);
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

// this is sadly slower than the sse2 version, so it's unused.
/*
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

        CONCAT3(_rle, TYPE_SIZE, CONCAT3(_, CODEC, process_symbol))(pIn, pOut, i, pState);
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
*/
#endif

//////////////////////////////////////////////////////////////////////////

static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_sse))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m128i symbol = _mm_setzero_si128();
#else
  const __m128i symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
  pInStart += (TYPE_SIZE / 8);
#endif

#if SYMBOL_COUNT > 1
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7F * VALUE_BROADCAST));
  other[1] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFF * VALUE_BROADCAST));
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x01 * VALUE_BROADCAST));
  other[3] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7E * VALUE_BROADCAST));
  other[4] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x80 * VALUE_BROADCAST));
  other[5] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFE * VALUE_BROADCAST));
  #endif
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
    symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
    pInStart += (TYPE_SIZE / 8);
#endif

#if !defined(SINGLE) && defined(SYMBOL_MASK)
#if SYMBOL_COUNT != 0
    if (symbolIndex)
#endif
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
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_ssse3))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m128i symbol = _mm_setzero_si128();
#else
  const __m128i symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
  pInStart += (TYPE_SIZE / 8);
#endif

#if SYMBOL_COUNT > 1
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7F * VALUE_BROADCAST));
  other[1] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFF * VALUE_BROADCAST));
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x01 * VALUE_BROADCAST));
  other[3] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7E * VALUE_BROADCAST));
  other[4] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x80 * VALUE_BROADCAST));
  other[5] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFE * VALUE_BROADCAST));
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
    symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
    pInStart += (TYPE_SIZE / 8);
#endif

#if !defined(SINGLE) && defined(SYMBOL_MASK)
#if SYMBOL_COUNT != 0
    if (symbolIndex)
#endif
    {
#if TYPE_SIZE == 24 || TYPE_SIZE == 48
      symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm_shuffle_epi8(symbol, shuffle2);
#endif
    }
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
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_sse41))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  uint8_t *pOutInitial = pOut;
  (void)pOutInitial;

#ifndef SINGLE
  __m128i symbol = _mm_setzero_si128();
#else
  const __m128i symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
  pInStart += (TYPE_SIZE / 8);
#endif

#if SYMBOL_COUNT > 1
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7F * VALUE_BROADCAST));
  other[1] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFF * VALUE_BROADCAST));
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x01 * VALUE_BROADCAST));
  other[3] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7E * VALUE_BROADCAST));
  other[4] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x80 * VALUE_BROADCAST));
  other[5] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFE * VALUE_BROADCAST));
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
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
    symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
    pInStart += (TYPE_SIZE / 8);
#endif

#if !defined(SINGLE) && defined(SYMBOL_MASK)
#if SYMBOL_COUNT != 0
    if (symbolIndex)
#endif
    {
#if TYPE_SIZE == 24 || TYPE_SIZE == 48
      symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm_shuffle_epi8(symbol, shuffle2);
#endif
    }
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
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifdef SINGLE
  const __m256i symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
  pInStart += (TYPE_SIZE / 8);
#endif

#ifndef SYMBOL_MASK
#ifndef SINGLE
  __m256i symbol = _mm256_setzero_si256();
#endif

#if SYMBOL_COUNT > 1
  typedef __m256i simd_t;
#endif

#if SYMBOL_COUNT > 1
  __m256i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7F * VALUE_BROADCAST));
  other[1] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFF * VALUE_BROADCAST));
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x01 * VALUE_BROADCAST));
  other[3] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7E * VALUE_BROADCAST));
  other[4] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x80 * VALUE_BROADCAST));
  other[5] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFE * VALUE_BROADCAST));
  #endif
#endif
#else
#ifdef SINGLE
#fail NOT SUPPORTED!
#endif

#if SYMBOL_COUNT > 1
  typedef __m128i simd_t;
#endif

  __m128i symbol = _mm_setzero_si128();

#if SYMBOL_COUNT > 1
  __m128i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7F * VALUE_BROADCAST));
  other[1] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFF * VALUE_BROADCAST));
#if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x01 * VALUE_BROADCAST));
  other[3] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7E * VALUE_BROADCAST));
  other[4] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x80 * VALUE_BROADCAST));
  other[5] = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFE * VALUE_BROADCAST));
#endif
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
#ifndef SYMBOL_MASK
      symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#else
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#endif
      pInStart += (TYPE_SIZE / 8);
      break;
    }

    case 6:
    case 5:
    case 4:
    case 3:
    {
      const simd_t tmp = other[symbolIndex - 1];

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
#else
    case 1:
    {
#ifndef SYMBOL_MASK
      symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#else
      symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#endif
      pInStart += (TYPE_SIZE / 8);
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
#ifndef SYMBOL_MASK
    symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#else
    symbol = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
#endif
    pInStart += (TYPE_SIZE / 8);
#endif

#if !defined(SINGLE) && defined(SYMBOL_MASK)
#if SYMBOL_COUNT != 0
    if (symbolIndex)
#endif
    {
#if TYPE_SIZE == 24 || TYPE_SIZE == 48
      symbol0 = _mm_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm_shuffle_epi8(symbol, shuffle2);
#endif
    }
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
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx2))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m256i symbol = _mm256_setzero_si256();
#else
  const __m256i symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
  pInStart += (TYPE_SIZE / 8);
#endif

#if SYMBOL_COUNT > 1
  __m256i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7F * VALUE_BROADCAST));
  other[1] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFF * VALUE_BROADCAST));
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x01 * VALUE_BROADCAST));
  other[3] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7E * VALUE_BROADCAST));
  other[4] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x80 * VALUE_BROADCAST));
  other[5] = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFE * VALUE_BROADCAST));
  #endif
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
      symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
      break;
    }
#endif

    case 0:
      break;
    }
#elif !defined(SINGLE)
    symbol = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
    pInStart += (TYPE_SIZE / 8);
#endif

#if !defined(SINGLE) && defined(SYMBOL_MASK)
#if SYMBOL_COUNT != 0
    if (symbolIndex)
#endif
    {
#if TYPE_SIZE == 24 || TYPE_SIZE == 48
      symbol0 = _mm256_shuffle_epi8(symbol, shuffle0);
      symbol1 = _mm256_shuffle_epi8(symbol, shuffle1);
      symbol2 = _mm256_shuffle_epi8(symbol, shuffle2);
#endif
    }
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
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx512f))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#ifndef SINGLE
  __m512i symbol = _mm512_setzero_si512();
#else
  const __m512i symbol = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
  pInStart += (TYPE_SIZE / 8);
#endif

#if SYMBOL_COUNT > 1
  __m512i other[SYMBOL_COUNT - 1];

  other[0] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7F * VALUE_BROADCAST));
  other[1] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFF * VALUE_BROADCAST));
  #if SYMBOL_COUNT == 7
  other[2] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x01 * VALUE_BROADCAST));
  other[3] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x7E * VALUE_BROADCAST));
  other[4] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0x80 * VALUE_BROADCAST));
  other[5] = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)((uintXX_t)(0xFE * VALUE_BROADCAST));
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
      symbol = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
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
#else
    case 1:
    {
      symbol = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
      pInStart += (TYPE_SIZE / 8);
      break;
    }
#endif

    case 0:
      break;
    }

#elif !defined(SINGLE)
    symbol = CONCAT2(_mm512_set1_epi, SIMD_TYPE_SIZE)(*(uintXX_t *)pInStart);
    pInStart += (TYPE_SIZE / 8);
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
#endif

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

#ifndef SYMBOL_MASK
  if (avx512FSupported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx512f))(pIn, pOut);
  else 
#endif
    if (avx2Supported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx2))(pIn, pOut);
  else if (avxSupported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_avx))(pIn, pOut);
  else if (sse41Supported)
    CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_sse41))(pIn, pOut);
#ifdef SYMBOL_MASK
  else if (ssse3Supported)
      CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, decompress_ssse3))(pIn, pOut);
#endif
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

#undef SIMD_TYPE_SIZE
#undef VALUE_BROADCAST
#undef VALUE_BROADCAST_INTERNAL
#undef uintXX_t

#undef CODEC

#ifdef CODEC_POSTFIX
  #undef CODEC_POSTFIX
#endif

#ifdef SYMBOL_MASK
  #undef SYMBOL_MASK
#endif

#undef COPY_SEGMENT_SSE
#undef COPY_SEGMENT_SSE41
#undef COPY_SEGMENT_AVX
#undef COPY_SEGMENT_AVX2
#undef COPY_SEGMENT_AVX512
#undef SET_SEGMENT_SSE
#undef SET_SEGMENT_SSE41   
#undef SET_SEGMENT_AVX
#undef SET_SEGMENT_AVX2
#undef SET_SEGMENT_AVX512

#if TYPE_SIZE == 64
  #undef _mm_set1_epi64
  #undef _mm256_set1_epi64
#endif
