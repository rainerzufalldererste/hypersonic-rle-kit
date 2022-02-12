#ifdef _MSC_VER
  #include <intrin.h>
#else
  #include <x86intrin.h>
#endif

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////

// `originalSize` must be a multiple of `8 * sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize * 3 / 8`.
inline void bitpack_encode3_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternSeparateHi32 = _mm_set1_epi32(0b11000000001100000000000000000000);
  const __m128i patternSeparateLo32 = _mm_set1_epi32(0b00000000000000000000110000000011);
  const __m128i patternSeparateHi16 = _mm_set1_epi16(0b0000110000000000);

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 8)
  {
    const __m128i p0 = _mm_loadu_si128(pIn128);
    const __m128i p1 = _mm_loadu_si128(pIn128 + 1);
    const __m128i p2 = _mm_loadu_si128(pIn128 + 2);
    const __m128i p3 = _mm_loadu_si128(pIn128 + 3);
    const __m128i p4 = _mm_loadu_si128(pIn128 + 4);
    const __m128i p5 = _mm_loadu_si128(pIn128 + 5);
    const __m128i p6 = _mm_loadu_si128(pIn128 + 6);
    const __m128i p7 = _mm_loadu_si128(pIn128 + 7);

    const __m128i p01p = _mm_or_si128(p0, _mm_slli_epi16(p1, 3));
    const __m128i p2p = _mm_slli_epi16(_mm_and_si128(p2, patternLow2), 6);

    const uint32_t mask2 = _mm_movemask_epi8(_mm_slli_epi16(p2, 5));

    const __m128i combinedA = _mm_or_si128(p01p, p2p);

    const __m128i p34p = _mm_or_si128(p3, _mm_slli_epi16(p4, 3));
    const __m128i p5p = _mm_slli_epi16(_mm_and_si128(p5, patternLow2), 6);

    const uint32_t mask5 = _mm_movemask_epi8(_mm_slli_epi16(p5, 5));
    const __m128i combinedB = _mm_or_si128(p34p, p5p);

    const __m128i p67p = _mm_or_si128(p6, _mm_slli_epi16(p7, 3));

    const uint32_t combinedMask = mask2 | (mask5 << 0x10);

    const __m128i lbit25_ = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      (uint8_t)(combinedMask >> 0x18),
      (uint8_t)(combinedMask >> 0x10),
      (uint8_t)(combinedMask >> 0x8),
      (uint8_t)(combinedMask));

    const __m128i lbit25_d = _mm_unpacklo_epi8(lbit25_, lbit25_);
    const __m128i lbit25_dd = _mm_unpacklo_epi8(lbit25_d, lbit25_d); // 01234567|01234567|01234567|01234567

    const __m128i lbit25_dd_01 = _mm_and_si128(lbit25_dd, patternSeparateLo32); // 01......|..23....|........|........
    const __m128i lbit25_dd_23 = _mm_srli_epi16(_mm_and_si128(lbit25_dd, patternSeparateHi32), 4); // ........|........|....45..|......67 -> ........|........|45......|..67....
    const __m128i lbit25_m = _mm_or_si128(lbit25_dd_01, lbit25_dd_23); // 01......|..23....|45......|..67....
    const __m128i lbit25_m13 = _mm_srli_epi16(_mm_and_si128(lbit25_m, patternSeparateHi16), 2); // ........|..23....|........|..67.... -> ........|23......|........|67......

    const __m128i lbit25 = _mm_and_si128(_mm_or_si128(lbit25_m, lbit25_m13), patternLow2); // 01......|2323....|45......|6767.... -> 01......|23......|45......|67......

    const __m128i lbit25_p = _mm_slli_epi16(lbit25, 6);

    const __m128i combinedC = _mm_or_si128(p67p, lbit25_p);

    _mm_storeu_si128(pOut128, combinedA);
    _mm_storeu_si128(pOut128 + 1, combinedB);
    _mm_storeu_si128(pOut128 + 2, combinedC);

    pIn128 += 8;
    pOut128 += 3;
  }
}

// `originalSize` must be a multiple of `8 * sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize * 3 / 8`.
inline void bitpack_encode3_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternSeparateHi32 = _mm_set1_epi32(0b11000000001100000000000000000000);
  const __m128i patternSeparateLo32 = _mm_set1_epi32(0b00000000000000000000110000000011);
  const __m128i patternSeparateHi16 = _mm_set1_epi16(0b0000110000000000);

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 8)
  {
    const __m128i p0 = _mm_load_si128(pIn128);
    const __m128i p1 = _mm_load_si128(pIn128 + 1);
    const __m128i p2 = _mm_load_si128(pIn128 + 2);
    const __m128i p3 = _mm_load_si128(pIn128 + 3);
    const __m128i p4 = _mm_load_si128(pIn128 + 4);
    const __m128i p5 = _mm_load_si128(pIn128 + 5);
    const __m128i p6 = _mm_load_si128(pIn128 + 6);
    const __m128i p7 = _mm_load_si128(pIn128 + 7);

    const __m128i p01p = _mm_or_si128(p0, _mm_slli_epi16(p1, 3));
    const __m128i p2p = _mm_slli_epi16(_mm_and_si128(p2, patternLow2), 6);

    const uint32_t mask2 = _mm_movemask_epi8(_mm_slli_epi16(p2, 5));

    const __m128i combinedA = _mm_or_si128(p01p, p2p);

    const __m128i p34p = _mm_or_si128(p3, _mm_slli_epi16(p4, 3));
    const __m128i p5p = _mm_slli_epi16(_mm_and_si128(p5, patternLow2), 6);

    const uint32_t mask5 = _mm_movemask_epi8(_mm_slli_epi16(p5, 5));
    const __m128i combinedB = _mm_or_si128(p34p, p5p);

    const __m128i p67p = _mm_or_si128(p6, _mm_slli_epi16(p7, 3));

    const uint32_t combinedMask = mask2 | (mask5 << 0x10);

    const __m128i lbit25_ = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      (uint8_t)(combinedMask >> 0x18),
      (uint8_t)(combinedMask >> 0x10),
      (uint8_t)(combinedMask >> 0x8),
      (uint8_t)(combinedMask));

    const __m128i lbit25_d = _mm_unpacklo_epi8(lbit25_, lbit25_);
    const __m128i lbit25_dd = _mm_unpacklo_epi8(lbit25_d, lbit25_d); // 01234567|01234567|01234567|01234567

    const __m128i lbit25_dd_01 = _mm_and_si128(lbit25_dd, patternSeparateLo32); // 01......|..23....|........|........
    const __m128i lbit25_dd_23 = _mm_srli_epi16(_mm_and_si128(lbit25_dd, patternSeparateHi32), 4); // ........|........|....45..|......67 -> ........|........|45......|..67....
    const __m128i lbit25_m = _mm_or_si128(lbit25_dd_01, lbit25_dd_23); // 01......|..23....|45......|..67....
    const __m128i lbit25_m13 = _mm_srli_epi16(_mm_and_si128(lbit25_m, patternSeparateHi16), 2); // ........|..23....|........|..67.... -> ........|23......|........|67......

    const __m128i lbit25 = _mm_and_si128(_mm_or_si128(lbit25_m, lbit25_m13), patternLow2); // 01......|2323....|45......|6767.... -> 01......|23......|45......|67......

    const __m128i lbit25_p = _mm_slli_epi16(lbit25, 6);

    const __m128i combinedC = _mm_or_si128(p67p, lbit25_p);

    _mm_stream_si128(pOut128, combinedA);
    _mm_stream_si128(pOut128 + 1, combinedB);
    _mm_stream_si128(pOut128 + 2, combinedC);

    pIn128 += 8;
    pOut128 += 3;
  }
}

//////////////////////////////////////////////////////////////////////////

// `encodedSize` must be a multiple of `3 * sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `encodedSize * 8 / 3`.
inline void bitpack_decode3_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternLow3 = _mm_set1_epi8(7);

  const __m128i patternBit12 = _mm_set1_epi16(0b100000000000);
  const __m128i patternBit3 = _mm_set1_epi16(0b100);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i) * 3)
  {
    const __m128i combinedA = _mm_loadu_si128(pIn128);
    const __m128i combinedB = _mm_loadu_si128(pIn128 + 1);
    const __m128i combinedC = _mm_loadu_si128(pIn128 + 2);

    const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
    const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
    const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));
    const __m128i d3 = _mm_and_si128(patternLow3, combinedB);
    const __m128i d4 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedB, 3));
    const __m128i d5lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedB, 6));
    const __m128i d6 = _mm_and_si128(patternLow3, combinedC);
    const __m128i d7 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedC, 3));

    const __m128i d25r = _mm_slli_epi16(_mm_and_si128(patternLow2, _mm_srli_epi16(combinedC, 6)), 2);
    const __m128i d2rd = _mm_unpacklo_epi8(d25r, d25r);
    const __m128i d5rd = _mm_unpackhi_epi8(d25r, d25r);

    const __m128i d2rd_lo = _mm_and_si128(d2rd, patternBit3);
    const __m128i d5rd_lo = _mm_and_si128(d5rd, patternBit3);

    const __m128i d2rd_hi = _mm_srli_epi16(_mm_and_si128(d2rd, patternBit12), 1);
    const __m128i d5rd_hi = _mm_srli_epi16(_mm_and_si128(d5rd, patternBit12), 1);

    const __m128i d2r = _mm_or_si128(d2rd_lo, d2rd_hi);
    const __m128i d5r = _mm_or_si128(d5rd_lo, d5rd_hi);

    const __m128i d2 = _mm_or_si128(d2r, d2lo);
    const __m128i d5 = _mm_or_si128(d5r, d5lo);

    _mm_storeu_si128(pOut128, d0);
    _mm_storeu_si128(pOut128 + 1, d1);
    _mm_storeu_si128(pOut128 + 2, d2);
    _mm_storeu_si128(pOut128 + 3, d3);
    _mm_storeu_si128(pOut128 + 4, d4);
    _mm_storeu_si128(pOut128 + 5, d5);
    _mm_storeu_si128(pOut128 + 6, d6);
    _mm_storeu_si128(pOut128 + 7, d7);

    pIn128 += 3;
    pOut128 += 8;
  }
}

// `encodedSize` must be a multiple of `3 * sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `encodedSize * 8 / 3`.
inline void bitpack_decode3_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternLow3 = _mm_set1_epi8(7);

  const __m128i patternBit12 = _mm_set1_epi16(0b100000000000);
  const __m128i patternBit3 = _mm_set1_epi16(0b100);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i) * 3)
  {
    const __m128i combinedA = _mm_load_si128(pIn128);
    const __m128i combinedB = _mm_load_si128(pIn128 + 1);
    const __m128i combinedC = _mm_load_si128(pIn128 + 2);

    const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
    const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
    const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));
    const __m128i d3 = _mm_and_si128(patternLow3, combinedB);
    const __m128i d4 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedB, 3));
    const __m128i d5lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedB, 6));
    const __m128i d6 = _mm_and_si128(patternLow3, combinedC);
    const __m128i d7 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedC, 3));

    const __m128i d25r = _mm_slli_epi16(_mm_and_si128(patternLow2, _mm_srli_epi16(combinedC, 6)), 2);
    const __m128i d2rd = _mm_unpacklo_epi8(d25r, d25r);
    const __m128i d5rd = _mm_unpackhi_epi8(d25r, d25r);

    const __m128i d2rd_lo = _mm_and_si128(d2rd, patternBit3);
    const __m128i d5rd_lo = _mm_and_si128(d5rd, patternBit3);

    const __m128i d2rd_hi = _mm_srli_epi16(_mm_and_si128(d2rd, patternBit12), 1);
    const __m128i d5rd_hi = _mm_srli_epi16(_mm_and_si128(d5rd, patternBit12), 1);

    const __m128i d2r = _mm_or_si128(d2rd_lo, d2rd_hi);
    const __m128i d5r = _mm_or_si128(d5rd_lo, d5rd_hi);

    const __m128i d2 = _mm_or_si128(d2r, d2lo);
    const __m128i d5 = _mm_or_si128(d5r, d5lo);

    _mm_stream_si128(pOut128, d0);
    _mm_stream_si128(pOut128 + 1, d1);
    _mm_stream_si128(pOut128 + 2, d2);
    _mm_stream_si128(pOut128 + 3, d3);
    _mm_stream_si128(pOut128 + 4, d4);
    _mm_stream_si128(pOut128 + 5, d5);
    _mm_stream_si128(pOut128 + 6, d6);
    _mm_stream_si128(pOut128 + 7, d7);

    pIn128 += 3;
    pOut128 += 8;
  }
}

//////////////////////////////////////////////////////////////////////////

// `originalSize` must be a multiple of `2 * sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize / 2`.
inline void bitpack_encode4_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 2)
  {
    const __m128i hi = _mm_loadu_si128(pIn128);
    const __m128i lo = _mm_loadu_si128(pIn128 + 1);

    const __m128i pack = _mm_or_si128(_mm_slli_epi16(hi, 4), lo);

    _mm_storeu_si128(pOut128, pack);

    pIn128 += 2;
    pOut128++;
  }
}

// `originalSize` must be a multiple of `2 * sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize / 2`.
inline void bitpack_encode4_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 2)
  {
    const __m128i hi = _mm_load_si128(pIn128);
    const __m128i lo = _mm_load_si128(pIn128 + 1);

    const __m128i pack = _mm_or_si128(_mm_slli_epi16(hi, 4), lo);

    _mm_stream_si128(pOut128, pack);

    pIn128 += 2;
    pOut128++;
  }
}

//////////////////////////////////////////////////////////////////////////

// `encodedSize` must be a multiple of `sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `encodedSize * 2`.
inline void bitpack_encode4_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i lo_pattern = _mm_set1_epi8(0x0F);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i))
  {
    const __m128i pack = _mm_loadu_si128(pIn128);

    const __m128i unpack_hi = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
    const __m128i unpack_lo = _mm_and_si128(lo_pattern, pack);

    _mm_storeu_si128(pOut128, unpack_hi);
    _mm_storeu_si128(pOut128 + 1, unpack_lo);

    pIn128++;
    pOut128 += 2;
  }
}

// `encodedSize` must be a multiple of `sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `encodedSize * 2`.
inline void bitpack_encode4_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i lo_pattern = _mm_set1_epi8(0x0F);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i))
  {
    const __m128i pack = _mm_load_si128(pIn128);

    const __m128i unpack_hi = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
    const __m128i unpack_lo = _mm_and_si128(lo_pattern, pack);

    _mm_stream_si128(pOut128, unpack_hi);
    _mm_stream_si128(pOut128 + 1, unpack_lo);

    pIn128++;
    pOut128 += 2;
  }
}

//////////////////////////////////////////////////////////////////////////

// `originalSize` must be a multiple of `4 * sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize / 4`.
inline void bitpack_encode2_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 4)
  {
    const __m128i p0 = _mm_loadu_si128(pIn128);
    const __m128i p1 = _mm_loadu_si128(pIn128 + 1);
    const __m128i p2 = _mm_loadu_si128(pIn128 + 2);
    const __m128i p3 = _mm_loadu_si128(pIn128 + 3);

    const __m128i pack = _mm_or_si128(_mm_or_si128(_mm_slli_epi16(p3, 6), _mm_slli_epi16(p2, 4)), _mm_or_si128(_mm_slli_epi16(p1, 2), p0));

    _mm_storeu_si128(pOut128, pack);

    pIn128 += 4;
    pOut128++;
  }
}

// `originalSize` must be a multiple of `4 * sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize / 4`.
inline void bitpack_encode2_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 4)
  {
    const __m128i p0 = _mm_load_si128(pIn128);
    const __m128i p1 = _mm_load_si128(pIn128 + 1);
    const __m128i p2 = _mm_load_si128(pIn128 + 2);
    const __m128i p3 = _mm_load_si128(pIn128 + 3);

    const __m128i pack = _mm_or_si128(_mm_or_si128(_mm_slli_epi16(p3, 6), _mm_slli_epi16(p2, 4)), _mm_or_si128(_mm_slli_epi16(p1, 2), p0));

    _mm_stream_si128(pOut128, pack);

    pIn128 += 4;
    pOut128++;
  }
}

//////////////////////////////////////////////////////////////////////////

// `encodedSize` must be a multiple of `sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `encodedSize * 4`.
inline void bitpack_encode2_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i lo_pattern = _mm_set1_epi8(3);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i))
  {
    const __m128i pack = _mm_loadu_si128(pIn128);

    const __m128i unpack0 = _mm_and_si128(lo_pattern, pack);
    const __m128i unpack1 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 2));
    const __m128i unpack2 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
    const __m128i unpack3 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 6));

    _mm_storeu_si128(pOut128, unpack0);
    _mm_storeu_si128(pOut128 + 1, unpack1);
    _mm_storeu_si128(pOut128 + 2, unpack2);
    _mm_storeu_si128(pOut128 + 3, unpack3);

    pIn128++;
    pOut128 += 4;
  }
}

// `encodedSize` must be a multiple of `sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `encodedSize * 4`.
inline void bitpack_encode2_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i lo_pattern = _mm_set1_epi8(3);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i))
  {
    const __m128i pack = _mm_load_si128(pIn128);

    const __m128i unpack0 = _mm_and_si128(lo_pattern, pack);
    const __m128i unpack1 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 2));
    const __m128i unpack2 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
    const __m128i unpack3 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 6));

    _mm_stream_si128(pOut128, unpack0);
    _mm_stream_si128(pOut128 + 1, unpack1);
    _mm_stream_si128(pOut128 + 2, unpack2);
    _mm_stream_si128(pOut128 + 3, unpack3);

    pIn128++;
    pOut128 += 4;
  }
}
