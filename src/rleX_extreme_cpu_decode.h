#if defined(IMPL_SSE2)
  #define SIMD_FUNC_NAME _decompress_sse2
#elif defined(IMPL_SSE41)
  #define SIMD_FUNC_NAME _decompress_sse41
#elif defined(IMPL_AVX)
  #define SIMD_FUNC_NAME _decompress_avx
#elif defined(IMPL_AVX2)
  #define SIMD_FUNC_NAME _decompress_avx2
#elif defined(IMPL_AVX512F)
  #define SIMD_FUNC_NAME _decompress_avx512f
#else
#fail IMPL NOT AVAILABLE
#endif

#ifndef _MSC_VER
#if defined(IMPL_SSE41)
__attribute__((target("sse4.1")))
#elif defined(IMPL_AVX)
__attribute__((target("avx")))
#elif defined(IMPL_AVX2)
__attribute__((target("avx2")))
#elif defined(IMPL_AVX512F)
__attribute__((target("avx512f")))
#else
#endif
#endif
static void CONCAT3(rle, TYPE_SIZE, CONCAT3(_, CODEC, SIMD_FUNC_NAME))(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;

#if defined(IMPL_SSE2) || defined(IMPL_SSE41)
  __m128i symbol = _mm_setzero_si128();
#elif defined(IMPL_AVX) || defined(IMPL_AVX2)
  __m256i symbol = _mm256_setzero_si256();
#elif defined(IMPL_AVX512F)
  __m512i symbol = _mm512_setzero_si512();
#else
  #fail IMPL NOT AVAILABLE
#endif

  typedef CONCAT3(uint, TYPE_SIZE, _t) symbol_t;

  while (true)
  {
#ifndef PACKED

#if defined(IMPL_SSE2) || defined(IMPL_SSE41)
    symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
#elif defined(IMPL_AVX) || defined(IMPL_AVX2)
    symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
#elif defined(IMPL_AVX512F)
    symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
#else
    #fail IMPL NOT AVAILABLE
#endif

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
#if defined(IMPL_SSE2) || defined(IMPL_SSE41)
      symbol = CONCAT2(_mm_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
#elif defined(IMPL_AVX) || defined(IMPL_AVX2)
      symbol = CONCAT2(_mm256_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
#elif defined(IMPL_AVX512F)
      symbol = CONCAT2(_mm512_set1_epi, TYPE_SIZE)(*(symbol_t *)pInStart);
#else
      #fail IMPL NOT AVAILABLE
#endif
      
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
#if defined(IMPL_SSE2)
    MEMCPY_SSE_MULTI;
#elif defined(IMPL_SSE41)
    MEMCPY_SSE41_MULTI;
#elif defined(IMPL_AVX)
    MEMCPY_AVX_MULTI;
#elif defined(IMPL_AVX2)
    MEMCPY_AVX2_MULTI;
#elif defined(IMPL_AVX512F)
    MEMCPY_AVX512_MULTI;
#else
    #fail IMPL NOT AVAILABLE
#endif

    if (!symbolCount)
      return;

#ifndef UNBOUND
    symbolCount = (symbolCount + (RLEX_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
    symbolCount = (symbolCount + RLEX_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

    // memset.
#if defined(IMPL_SSE2) || defined(IMPL_SSE41)
    MEMSET_SSE_MULTI;
#elif defined(IMPL_AVX) || defined(IMPL_AVX2)
    MEMSET_AVX_MULTI;
#elif defined(IMPL_AVX512F)
    MEMSET_AVX512_MULTI;
#else
    #fail IMPL NOT AVAILABLE
#endif
  }
}

#undef SIMD_FUNC_NAME
