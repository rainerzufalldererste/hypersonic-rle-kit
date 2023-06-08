#if defined(IMPL_SSE2)
  #define FUNC_NAME compress_sse2
#elif defined(IMPL_SSSE3)
  #define FUNC_NAME compress_ssse3
#elif defined(IMPL_AVX2)
  #define FUNC_NAME compress_avx2
#else
  #define FUNC_NAME compress_base
#endif

#ifndef _MSC_VER
#if defined(IMPL_SSSE3)
__attribute__((target("ssse3")))
#elif defined(IMPL_AVX2)
__attribute__((target("avx2")))
#endif
#endif
uint32_t CONCAT3(CONCAT3(rle, TYPE_SIZE, _), CODEC, FUNC_NAME)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
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

#if defined(IMPL_SSE2) || defined(IMPL_SSSE3)
  __m128i symbolSimd = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(state.symbol);
  const int64_t inSizeSimd = inSize - sizeof(__m128i) - (TYPE_SIZE / 8);
#elif defined(IMPL_AVX2)
  __m256i symbolSimd = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(state.symbol);
  const int64_t inSizeSimd = inSize - sizeof(__m256i) - (TYPE_SIZE / 8);
#endif

#if defined(IMPL_SSE2)
#if TYPE_SIZE == 24
  const __m128i pattern00 = _mm_set_epi8(0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1);
  const __m128i pattern01 = _mm_set_epi8(-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0);
  const __m128i pattern02 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0);
  const __m128i pattern03 = _mm_set_epi8(0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#elif TYPE_SIZE == 48
  const __m128i pattern00 = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1);
  const __m128i pattern02 = _mm_set_epi8(0, 0, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0);
  const __m128i pattern04 = _mm_set_epi8(-1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
#elif defined(IMPL_SSSE3)
#if TYPE_SIZE == 24
  const __m128i shuffle0 = _mm_set_epi8(0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0);
#elif TYPE_SIZE == 48
  const __m128i shuffle0 = _mm_set_epi8(3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
#endif
#elif defined(IMPL_AVX2)
#if TYPE_SIZE == 24
  const __m256i shuffle0 = _mm256_set_epi8(1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0);
#elif TYPE_SIZE == 48
  const __m256i shuffle0 = _mm256_set_epi8(1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0);
#endif
#endif

  size_t i = 0;

  while (i < inSize)
  {
#if defined(IMPL_SSE2) || defined(IMPL_SSSE3) || defined(IMPL_AVX2)
    continue_outer_loop:;

#if defined(IMPL_SSE2) || defined(IMPL_SSSE3)
    symbolSimd = CONCAT2(_mm_set1_epi, SIMD_TYPE_SIZE)(state.symbol);
#elif defined(IMPL_AVX2)
    symbolSimd = CONCAT2(_mm256_set1_epi, SIMD_TYPE_SIZE)(state.symbol);
#endif

#if defined(IMPL_SSE2)
#if TYPE_SIZE == 24
    const __m128i shift1 = _mm_or_si128(_mm_srli_si128(symbolSimd, 1), _mm_slli_si128(symbolSimd, 15));
    const __m128i symbol0 = _mm_or_si128(_mm_and_si128(symbolSimd, pattern00), _mm_and_si128(shift1, pattern01));
    const __m128i shift2 = _mm_or_si128(_mm_srli_si128(symbolSimd, 2), _mm_slli_si128(symbolSimd, 14));
    const __m128i shift3 = _mm_or_si128(_mm_srli_si128(symbolSimd, 3), _mm_slli_si128(symbolSimd, 13));

    symbolSimd = _mm_or_si128(symbol0, _mm_or_si128(_mm_and_si128(shift3, pattern03), _mm_and_si128(shift2, pattern02)));
#elif TYPE_SIZE == 48
    const __m128i shift2 = _mm_or_si128(_mm_srli_si128(symbolSimd, 2), _mm_slli_si128(symbolSimd, 14));
    const __m128i symbol0 = _mm_or_si128(_mm_and_si128(symbolSimd, pattern00), _mm_and_si128(shift2, pattern02));
    const __m128i shift4 = _mm_or_si128(_mm_srli_si128(symbolSimd, 4), _mm_slli_si128(symbolSimd, 12));

    symbolSimd = _mm_or_si128(symbol0, _mm_and_si128(shift4, pattern04));
#endif
#elif defined(IMPL_SSSE3)
#if TYPE_SIZE == 24
    symbolSimd = _mm_shuffle_epi8(symbolSimd, shuffle0);
#elif TYPE_SIZE == 48
    symbolSimd = _mm_shuffle_epi8(symbolSimd, shuffle0);
#endif
#elif defined(IMPL_AVX2)
#if TYPE_SIZE == 24
    symbolSimd = _mm256_shuffle_epi8(symbolSimd, shuffle0);
#elif TYPE_SIZE == 48
    symbolSimd = _mm256_shuffle_epi8(symbolSimd, shuffle0);
#endif
#endif

    while (i < inSizeSimd)
    {
#if defined(IMPL_SSE2) || defined(IMPL_SSSE3)
      const __m128i current = _mm_loadu_si128((const __m128i *)(pIn + i));
      const __m128i matchMask = _mm_cmpeq_epi8(current, symbolSimd);
      uint32_t bitMask = _mm_movemask_epi8(matchMask);
#elif defined(IMPL_AVX2)
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
#if TYPE_SIZE == 8 || TYPE_SIZE == 16 || TYPE_SIZE == 32 || TYPE_SIZE == 64 || TYPE_SIZE == 128
          bit &= ~(uint32_t)(((uint32_t)(TYPE_SIZE / 8)) - 1);
#else
          bit -= (bit % (TYPE_SIZE / 8));
#endif
#endif

          i += bit;
          state.count += bit;

          goto continue_second_step;
        }
        else
        {
#if TYPE_SIZE == 8 || TYPE_SIZE == 16 || TYPE_SIZE == 32 || TYPE_SIZE == 64 || TYPE_SIZE == 128
          i += (sizeof(current));
          state.count += (sizeof(current));
#else
          i += (sizeof(current) - (sizeof(current) % (TYPE_SIZE / 8)));
          state.count += (sizeof(current) - (sizeof(current) % (TYPE_SIZE / 8)));
#endif
        }
      }
      else
      {
        goto continue_second_step;
      }
    }
#endif

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

#if defined(IMPL_SSE2) || defined(IMPL_SSSE3) || defined(IMPL_AVX2)
    continue_second_step:;
#endif

    {
      CONCAT3(CONCAT3(_rle, TYPE_SIZE, _), CODEC, process_symbol)(pIn, pOut, i, &state);

#if defined(IMPL_SSE2) || defined(IMPL_SSSE3) || defined(IMPL_AVX2)
      while (i < inSizeSimd)
      {
#if defined(IMPL_SSE2) || defined(IMPL_SSSE3)
        const __m128i current = _mm_loadu_si128((const __m128i *)(pIn + i));
        const __m128i other = _mm_loadu_si128((const __m128i *)(pIn + i + (TYPE_SIZE / 8)));
        //const __m128i other = _mm_bsrli_si128(current, TYPE_SIZE / 8);
        const __m128i matchMask = _mm_cmpeq_epi8(current, other);
        uint32_t bitMask = _mm_movemask_epi8(matchMask);
#elif defined(IMPL_AVX2)
        const __m256i current = _mm256_loadu_si256((const __m256i *)(pIn + i));
        const __m256i other = _mm256_loadu_si256((const __m256i *)(pIn + i + (TYPE_SIZE / 8)));
        const __m256i matchMask = _mm256_cmpeq_epi8(current, other);
        uint32_t bitMask = _mm256_movemask_epi8(matchMask);
#endif

        //bitMask &= (((uint32_t)1 << (sizeof(current) - (TYPE_SIZE / 8))) - 1);

        // Check if all symbols match:
#if TYPE_SIZE == 16
        bitMask &= (bitMask >> 1);
#elif TYPE_SIZE == 24
        bitMask &= ((bitMask >> 1) & (bitMask >> 2));
#elif TYPE_SIZE == 32
        bitMask &= (bitMask >> 2);
        bitMask &= (bitMask >> 1);
#elif TYPE_SIZE == 48
        bitMask &= (bitMask >> 3);
        bitMask &= ((bitMask >> 1) & (bitMask >> 2));
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

#ifndef SYMBOL_MASK
            state.symbol = *(symbol_t *)(&pIn[i + bit]);
#else
            state.symbol = *(symbol_t *)(&pIn[i + bit]) & SYMBOL_MASK;
#endif

            i += bit + (TYPE_SIZE / 8) * 2;
            state.count = (TYPE_SIZE / 8) * 2;

            goto continue_outer_loop;
          }
      }
#endif

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
      const size_t copySize = i - state.lastRLE;

      *((uint16_t *)&pOut[state.index]) = 1 << RLE8_XSYMLUT_RANGE_BITS;
      state.index += sizeof(uint16_t);
      *((uint16_t *)&pOut[state.index]) = 0;
      state.index += sizeof(uint16_t);
      *((uint32_t *)&pOut[state.index]) = (uint32_t)copySize + RLE8_XSYMLUT_RANGE_VALUE_OFFSET;
      state.index += sizeof(uint32_t);

      memcpy(pOut + state.index, pIn + state.lastRLE, copySize);
      state.index += copySize;
    }
  }

  // Store compressed length.
  return ((uint32_t *)pOut)[1] = (uint32_t)state.index;
}

#undef FUNC_NAME
