#ifdef _MSC_VER
  #include <intrin.h>
#else
  #include <x86intrin.h>
#endif

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////

// Bitpack lowest 3 bits from 3 * __m128i into __m128i + uint16_t.
// `originalSize` must be a multiple of 3 * __m128i.
// `pOut` must contain at least `originalSize * 3 / 8` bytes.
// Marginally slower than `bitpack_encode3_6_sse2`.
inline void bitpack_encode3_3_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i patternLow2 = _mm_set1_epi8(3);

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 3)
  {
    const __m128i p0 = _mm_load_si128(pIn128);
    const __m128i p1 = _mm_load_si128(pIn128 + 1);
    const __m128i p2 = _mm_load_si128(pIn128 + 2);

    pIn128 += 3;

    const __m128i p01p = _mm_or_si128(p0, _mm_slli_epi16(p1, 3));
    const __m128i p2p = _mm_slli_epi16(_mm_and_si128(p2, patternLow2), 6);

    const __m128i combinedA = _mm_or_si128(p01p, p2p);

    _mm_storeu_si128((__m128i *)pOut, combinedA);
    pOut += sizeof(__m128i);

    const uint32_t mask2 = _mm_movemask_epi8(_mm_slli_epi16(p2, 5));

    *(uint16_t *)pOut = (uint16_t)mask2;
    pOut += sizeof(uint16_t);
  }
}

// Bitpack lowest 3 bits from 3 * __m128i into __m128i + uint16_t.
// `originalSize` must be a multiple of 3 * __m128i.
// `pOut` must contain at least `originalSize * 3 / 8` bytes.
// Marginally slower than `bitpack_encode3_6_sse2`.
inline void bitpack_encode3_3_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i patternLow2 = _mm_set1_epi8(3);

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 3)
  {
    const __m128i p0 = _mm_loadu_si128(pIn128);
    const __m128i p1 = _mm_loadu_si128(pIn128 + 1);
    const __m128i p2 = _mm_loadu_si128(pIn128 + 2);

    pIn128 += 3;

    const __m128i p01p = _mm_or_si128(p0, _mm_slli_epi16(p1, 3));
    const __m128i p2p = _mm_slli_epi16(_mm_and_si128(p2, patternLow2), 6);

    const __m128i combinedA = _mm_or_si128(p01p, p2p);

    _mm_storeu_si128((__m128i *)pOut, combinedA);
    pOut += sizeof(__m128i);

    const uint32_t mask2 = _mm_movemask_epi8(_mm_slli_epi16(p2, 5));

    *(uint16_t *)pOut = (uint16_t)mask2;
    pOut += sizeof(uint16_t);
  }
}

//////////////////////////////////////////////////////////////////////////

// Bitpack 3 bits from __m128i + uint16_t into 3 * __m128i.
// `originalSize` must be a multiple of __m128i + uint16_t.
// `pOut` must contain at least `encodedSize * 8 / 3` bytes.
// Marginally slower than `bitpack_decode3_6_sse2`.
inline void bitpack_decode3_3_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;

  const __m128i patternLow1 = _mm_set1_epi8(1);
  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternLow3 = _mm_set1_epi8(7);
  const __m128i patternBitSelect = _mm_set1_epi64x((int64_t)0x8040201008040201);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i) + sizeof(uint16_t))
  {
    const __m128i combinedA = _mm_loadu_si128((const __m128i *)pIn);

    const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
    const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
    const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));

    _mm_stream_si128(pOut128, d0);
    _mm_stream_si128(pOut128 + 1, d1);

    pIn += sizeof(__m128i);

    const uint16_t mask2 = *(uint16_t *)pIn;
    pIn += sizeof(uint16_t);

    __m128i v = _mm_set1_epi16(mask2);

    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);

    v = _mm_and_si128(v, patternBitSelect);
    v = _mm_min_epu8(v, patternLow1);

    const __m128i d2 = _mm_or_si128(d2lo, _mm_slli_epi16(v, 2));

    _mm_stream_si128(pOut128 + 2, d2);

    pOut128 += 3;
  }
}

// Bitpack 3 bits from __m128i + uint16_t into 3 * __m128i.
// `originalSize` must be a multiple of __m128i + uint16_t.
// `pOut` must contain at least `encodedSize * 8 / 3` bytes.
// Marginally slower than `bitpack_decode3_6_sse2`.
inline void bitpack_decode3_3_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;

  const __m128i patternLow1 = _mm_set1_epi8(1);
  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternLow3 = _mm_set1_epi8(7);
  const __m128i patternBitSelect = _mm_set1_epi64x((int64_t)0x8040201008040201);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i) + sizeof(uint16_t))
  {
    const __m128i combinedA = _mm_loadu_si128((const __m128i *)pIn);

    const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
    const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
    const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));

    _mm_storeu_si128(pOut128, d0);
    _mm_storeu_si128(pOut128 + 1, d1);

    pIn += sizeof(__m128i);

    const uint16_t mask2 = *(uint16_t *)pIn;
    pIn += sizeof(uint16_t);

    __m128i v = _mm_set1_epi16(mask2);

    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);

    v = _mm_and_si128(v, patternBitSelect);
    v = _mm_min_epu8(v, patternLow1);

    const __m128i d2 = _mm_or_si128(d2lo, _mm_slli_epi16(v, 2));

    _mm_storeu_si128(pOut128 + 2, d2);

    pOut128 += 3;
  }
}

//////////////////////////////////////////////////////////////////////////

// Bitpack lowest 3 bits from 6 * __m128i into 2 * __m128i + uint32_t.
// `originalSize` must be a multiple of 6 * __m128i.
// `pOut` must contain at least `originalSize * 3 / 8` bytes.
// Marginally faster than `bitpack_encode3_3_sse2`.
inline void bitpack_encode3_6_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i patternLow2 = _mm_set1_epi8(3);

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 6)
  {
    const __m128i p0 = _mm_load_si128(pIn128);
    const __m128i p1 = _mm_load_si128(pIn128 + 1);
    const __m128i p2 = _mm_load_si128(pIn128 + 2);
    const __m128i p3 = _mm_load_si128(pIn128 + 3);
    const __m128i p4 = _mm_load_si128(pIn128 + 4);
    const __m128i p5 = _mm_load_si128(pIn128 + 5);

    pIn128 += 6;

    const __m128i p01p = _mm_or_si128(p0, _mm_slli_epi16(p1, 3));
    const __m128i p2p = _mm_slli_epi16(_mm_and_si128(p2, patternLow2), 6);

    const __m128i combinedA = _mm_or_si128(p01p, p2p);

    _mm_storeu_si128((__m128i *)pOut, combinedA);

    const __m128i p34p = _mm_or_si128(p3, _mm_slli_epi16(p4, 3));
    const __m128i p5p = _mm_slli_epi16(_mm_and_si128(p5, patternLow2), 6);

    const __m128i combinedB = _mm_or_si128(p34p, p5p);

    _mm_storeu_si128((__m128i *)(pOut + sizeof(__m128i)), combinedB);
    pOut += sizeof(__m128i) * 2;

    const uint32_t mask25 = _mm_movemask_epi8(_mm_slli_epi16(p2, 5)) | (_mm_movemask_epi8(_mm_slli_epi16(p5, 5)) << 16);

    *(uint32_t *)pOut = (uint32_t)mask25;
    pOut += sizeof(uint32_t);
  }
}

// Bitpack lowest 3 bits from 6 * __m128i into 2 * __m128i + uint32_t.
// `originalSize` must be a multiple of 6 * __m128i.
// `pOut` must contain at least `originalSize * 3 / 8` bytes.
// Marginally faster than `bitpack_encode3_3_sse2`.
inline void bitpack_encode3_6_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  const __m128i *pIn128 = (const __m128i *)pIn;

  const __m128i patternLow2 = _mm_set1_epi8(3);

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i) * 6)
  {
    const __m128i p0 = _mm_loadu_si128(pIn128);
    const __m128i p1 = _mm_loadu_si128(pIn128 + 1);
    const __m128i p2 = _mm_loadu_si128(pIn128 + 2);
    const __m128i p3 = _mm_loadu_si128(pIn128 + 3);
    const __m128i p4 = _mm_loadu_si128(pIn128 + 4);
    const __m128i p5 = _mm_loadu_si128(pIn128 + 5);

    pIn128 += 6;

    const __m128i p01p = _mm_or_si128(p0, _mm_slli_epi16(p1, 3));
    const __m128i p2p = _mm_slli_epi16(_mm_and_si128(p2, patternLow2), 6);

    const __m128i combinedA = _mm_or_si128(p01p, p2p);

    _mm_storeu_si128((__m128i *)pOut, combinedA);

    const __m128i p34p = _mm_or_si128(p3, _mm_slli_epi16(p4, 3));
    const __m128i p5p = _mm_slli_epi16(_mm_and_si128(p5, patternLow2), 6);

    const __m128i combinedB = _mm_or_si128(p34p, p5p);

    _mm_storeu_si128((__m128i *)(pOut + sizeof(__m128i)), combinedB);
    pOut += sizeof(__m128i) * 2;

    const uint32_t mask25 = _mm_movemask_epi8(_mm_slli_epi16(p2, 5)) | (_mm_movemask_epi8(_mm_slli_epi16(p5, 5)) << 16);

    *(uint32_t *)pOut = (uint32_t)mask25;
    pOut += sizeof(uint32_t);
  }
}

//////////////////////////////////////////////////////////////////////////

// Bitpack 3 bits from 2 * __m128i + uint32_t into 6 * __m128i.
// `originalSize` must be a multiple of 2 * __m128i + uint32_t.
// `pOut` must contain at least `encodedSize * 8 / 3` bytes.
// Marginally faster than `bitpack_decode3_3_sse2`.
inline void bitpack_decode3_6_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;

  const __m128i patternLow1 = _mm_set1_epi8(1);
  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternLow3 = _mm_set1_epi8(7);
  const __m128i patternBitSelect = _mm_set1_epi64x((int64_t)0x8040201008040201);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i) * 2 + sizeof(uint32_t))
  {
    const __m128i combinedA = _mm_loadu_si128((const __m128i *)(pIn));
    const __m128i combinedB = _mm_loadu_si128((const __m128i *)(pIn + sizeof(__m128i)));

    const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
    const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
    const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));
    const __m128i d3 = _mm_and_si128(patternLow3, combinedB);
    const __m128i d4 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedB, 3));
    const __m128i d5lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedB, 6));

    _mm_stream_si128(pOut128, d0);
    _mm_stream_si128(pOut128 + 1, d1);

    pIn += sizeof(__m128i) * 2;

    const uint32_t mask25 = *(uint32_t *)pIn;
    pIn += sizeof(uint32_t);

    __m128i v = _mm_set1_epi32(mask25);

    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);
    __m128i vlo = _mm_unpacklo_epi8(v, v);
    __m128i vhi = _mm_unpackhi_epi8(v, v);

    vlo = _mm_and_si128(vlo, patternBitSelect);
    vhi = _mm_and_si128(vhi, patternBitSelect);
    vlo = _mm_min_epu8(vlo, patternLow1);
    vhi = _mm_min_epu8(vhi, patternLow1);

    const __m128i d2 = _mm_or_si128(d2lo, _mm_slli_epi16(vlo, 2));
    const __m128i d5 = _mm_or_si128(d5lo, _mm_slli_epi16(vhi, 2));

    _mm_stream_si128(pOut128 + 2, d2);
    _mm_stream_si128(pOut128 + 3, d3);
    _mm_stream_si128(pOut128 + 4, d4);
    _mm_stream_si128(pOut128 + 5, d5);

    pOut128 += 6;
  }
}

// Bitpack 3 bits from 2 * __m128i + uint32_t into 6 * __m128i.
// `originalSize` must be a multiple of 2 * __m128i + uint32_t.
// `pOut` must contain at least `encodedSize * 8 / 3` bytes.
// Marginally faster than `bitpack_decode3_3_sse2`.
inline void bitpack_decode3_6_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  __m128i *pOut128 = (__m128i *)pOut;

  const __m128i patternLow1 = _mm_set1_epi8(1);
  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternLow3 = _mm_set1_epi8(7);
  const __m128i patternBitSelect = _mm_set1_epi64x((int64_t)0x8040201008040201);

  for (size_t i = 0; i < encodedSize; i += sizeof(__m128i) * 2 + sizeof(uint32_t))
  {
    const __m128i combinedA = _mm_loadu_si128((const __m128i *)(pIn));
    const __m128i combinedB = _mm_loadu_si128((const __m128i *)(pIn + sizeof(__m128i)));

    const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
    const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
    const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));
    const __m128i d3 = _mm_and_si128(patternLow3, combinedB);
    const __m128i d4 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedB, 3));
    const __m128i d5lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedB, 6));

    _mm_storeu_si128(pOut128, d0);
    _mm_storeu_si128(pOut128 + 1, d1);

    pIn += sizeof(__m128i) * 2;

    const uint32_t mask25 = *(uint32_t *)pIn;
    pIn += sizeof(uint32_t);

    __m128i v = _mm_set1_epi32(mask25);

    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);
    __m128i vlo = _mm_unpacklo_epi8(v, v);
    __m128i vhi = _mm_unpackhi_epi8(v, v);

    vlo = _mm_and_si128(vlo, patternBitSelect);
    vhi = _mm_and_si128(vhi, patternBitSelect);
    vlo = _mm_min_epu8(vlo, patternLow1);
    vhi = _mm_min_epu8(vhi, patternLow1);

    const __m128i d2 = _mm_or_si128(d2lo, _mm_slli_epi16(vlo, 2));
    const __m128i d5 = _mm_or_si128(d5lo, _mm_slli_epi16(vhi, 2));

    _mm_storeu_si128(pOut128 + 2, d2);
    _mm_storeu_si128(pOut128 + 3, d3);
    _mm_storeu_si128(pOut128 + 4, d4);
    _mm_storeu_si128(pOut128 + 5, d5);

    pOut128 += 6;
  }
}

//////////////////////////////////////////////////////////////////////////

// `originalSize` must be a multiple of __m128i.
// `pOut` must contain at least `originalSize * 3 / 8` bytes.
// returns `pOut` one byte after the last one that was written to.
inline uint8_t * bitpack_encode3_sse2_unaligned_m128i(const __m128i *pIn, uint8_t *pOut, const size_t originalSize)
{
  const __m128i patternLow2 = _mm_set1_epi8(3);

  int64_t i = 0;
  int64_t originalSize6 = (int64_t)originalSize - sizeof(__m128i) * 6;
  int64_t originalSize3 = (int64_t)originalSize - sizeof(__m128i) * 3;

  for (; i <= originalSize6; i += sizeof(__m128i) * 6)
  {
    const __m128i p0 = _mm_loadu_si128(pIn);
    const __m128i p1 = _mm_loadu_si128(pIn + 1);
    const __m128i p2 = _mm_loadu_si128(pIn + 2);
    const __m128i p3 = _mm_loadu_si128(pIn + 3);
    const __m128i p4 = _mm_loadu_si128(pIn + 4);
    const __m128i p5 = _mm_loadu_si128(pIn + 5);

    pIn += 6;

    const __m128i p01p = _mm_or_si128(p0, _mm_slli_epi16(p1, 3));
    const __m128i p2p = _mm_slli_epi16(_mm_and_si128(p2, patternLow2), 6);

    const __m128i combinedA = _mm_or_si128(p01p, p2p);

    _mm_storeu_si128((__m128i *)pOut, combinedA);

    const __m128i p34p = _mm_or_si128(p3, _mm_slli_epi16(p4, 3));
    const __m128i p5p = _mm_slli_epi16(_mm_and_si128(p5, patternLow2), 6);

    const __m128i combinedB = _mm_or_si128(p34p, p5p);

    _mm_storeu_si128((__m128i *)(pOut + sizeof(__m128i)), combinedB);
    pOut += sizeof(__m128i) * 2;

    const uint32_t mask25 = _mm_movemask_epi8(_mm_slli_epi16(p2, 5)) | (_mm_movemask_epi8(_mm_slli_epi16(p5, 5)) << 16);

    *(uint32_t *)pOut = (uint32_t)mask25;
    pOut += sizeof(uint32_t);
  }

  if (i <= originalSize3)
  {
    i += sizeof(__m128i) * 3;

    const __m128i p0 = _mm_loadu_si128(pIn);
    const __m128i p1 = _mm_loadu_si128(pIn + 1);
    const __m128i p2 = _mm_loadu_si128(pIn + 2);

    const __m128i p01p = _mm_or_si128(p0, _mm_slli_epi16(p1, 3));
    const __m128i p2p = _mm_slli_epi16(_mm_and_si128(p2, patternLow2), 6);

    const __m128i combinedA = _mm_or_si128(p01p, p2p);

    _mm_storeu_si128((__m128i *)pOut, combinedA);
    pOut += sizeof(__m128i);

    const uint32_t mask2 = _mm_movemask_epi8(_mm_slli_epi16(p2, 5));

    *(uint16_t *)pOut = (uint16_t)mask2;
    pOut += sizeof(uint16_t);
  }

  for (; i < (int64_t)originalSize; i += sizeof(__m128i))
  {
    const __m128i p0 = _mm_loadu_si128(pIn);
    pIn++;

    const uint32_t mask0 = _mm_movemask_epi8(_mm_slli_epi16(p0, 7));
    const uint32_t mask1 = _mm_movemask_epi8(_mm_slli_epi16(p0, 6));
    const uint32_t mask2 = _mm_movemask_epi8(_mm_slli_epi16(p0, 5));

    uint16_t *pOut16 = (uint16_t *)pOut;

    pOut16[0] = (uint16_t)mask0;
    pOut16[1] = (uint16_t)mask1;
    pOut16[2] = (uint16_t)mask2;

    pOut += sizeof(uint16_t) * 3;
  }

  return pOut;
}

// `originalSize` must be a multiple of __m128i.
// `pIn` must contain at least `originalSize * 3 / 8` bytes.
// returns `pIn` after the last byte read.
inline const uint8_t * bitpack_decode3_sse2_unaligned_m128i(const uint8_t *pIn, __m128i *pOut, const size_t originalSize)
{
  int64_t i = 0;
  int64_t originalSize6 = (int64_t)originalSize - sizeof(__m128i) * 6;
  int64_t originalSize3 = (int64_t)originalSize - sizeof(__m128i) * 3;

  const __m128i patternLow1 = _mm_set1_epi8(1);
  const __m128i patternLow2 = _mm_set1_epi8(3);
  const __m128i patternLow3 = _mm_set1_epi8(7);
  const __m128i patternBitSelect = _mm_set1_epi64x((int64_t)0x8040201008040201);

  for (; i <= originalSize6; i += sizeof(__m128i) * 6)
  {
    const __m128i combinedA = _mm_loadu_si128((const __m128i *)(pIn));
    const __m128i combinedB = _mm_loadu_si128((const __m128i *)(pIn + sizeof(__m128i)));

    const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
    const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
    const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));
    const __m128i d3 = _mm_and_si128(patternLow3, combinedB);
    const __m128i d4 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedB, 3));
    const __m128i d5lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedB, 6));

    _mm_storeu_si128(pOut, d0);
    _mm_storeu_si128(pOut + 1, d1);

    pIn += sizeof(__m128i) * 2;

    const uint32_t mask25 = *(uint32_t *)pIn;
    pIn += sizeof(uint32_t);

    __m128i v = _mm_set1_epi32(mask25);

    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);
    __m128i vlo = _mm_unpacklo_epi8(v, v);
    __m128i vhi = _mm_unpackhi_epi8(v, v);

    vlo = _mm_and_si128(vlo, patternBitSelect);
    vhi = _mm_and_si128(vhi, patternBitSelect);
    vlo = _mm_min_epu8(vlo, patternLow1);
    vhi = _mm_min_epu8(vhi, patternLow1);

    const __m128i d2 = _mm_or_si128(d2lo, _mm_slli_epi16(vlo, 2));
    const __m128i d5 = _mm_or_si128(d5lo, _mm_slli_epi16(vhi, 2));

    _mm_storeu_si128(pOut + 2, d2);
    _mm_storeu_si128(pOut + 3, d3);
    _mm_storeu_si128(pOut + 4, d4);
    _mm_storeu_si128(pOut + 5, d5);

    pOut += 6;
  }

  if (i <= originalSize3)
  {
    i += sizeof(__m128i) * 3;

    const __m128i combinedA = _mm_loadu_si128((const __m128i *)pIn);

    const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
    const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
    const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));

    _mm_storeu_si128(pOut, d0);
    _mm_storeu_si128(pOut + 1, d1);

    pIn += sizeof(__m128i);

    const uint16_t mask2 = *(uint16_t *)pIn;
    pIn += sizeof(uint16_t);

    __m128i v = _mm_set1_epi16(mask2);

    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);

    v = _mm_and_si128(v, patternBitSelect);
    v = _mm_min_epu8(v, patternLow1);

    const __m128i d2 = _mm_or_si128(d2lo, _mm_slli_epi16(v, 2));

    _mm_storeu_si128(pOut + 2, d2);

    pOut += 3;
  }

  for (; i < (int64_t)originalSize; i += sizeof(__m128i))
  {
    const uint16_t *pIn16 = (const uint16_t *)pIn;

    const uint16_t mask0 = pIn16[0];
    const uint16_t mask1 = pIn16[1];
    const uint16_t mask2 = pIn16[2];

    pIn += sizeof(uint16_t) * 3;

    __m128i v0 = _mm_set1_epi16(mask0);
    __m128i v1 = _mm_set1_epi16(mask1);
    __m128i v2 = _mm_set1_epi16(mask2);

    v0 = _mm_unpacklo_epi8(v0, v0);
    v1 = _mm_unpacklo_epi8(v1, v1);
    v2 = _mm_unpacklo_epi8(v2, v2);

    v0 = _mm_unpacklo_epi8(v0, v0);
    v1 = _mm_unpacklo_epi8(v1, v1);
    v2 = _mm_unpacklo_epi8(v2, v2);
    
    v0 = _mm_unpacklo_epi8(v0, v0);
    v1 = _mm_unpacklo_epi8(v1, v1);
    v2 = _mm_unpacklo_epi8(v2, v2);
    
    v0 = _mm_and_si128(v0, patternBitSelect);
    v1 = _mm_and_si128(v1, patternBitSelect);
    v2 = _mm_and_si128(v2, patternBitSelect);

    v0 = _mm_min_epu8(v0, patternLow1);
    v1 = _mm_min_epu8(v1, patternLow1);
    v2 = _mm_min_epu8(v2, patternLow1);

    v1 = _mm_slli_epi16(v1, 1);
    v2 = _mm_slli_epi16(v2, 2);

    const __m128i combined = _mm_or_si128(_mm_or_si128(v1, v2), v0);
    
    _mm_storeu_si128(pOut, combined);
    pOut++;
  }

  return pIn;
}

//////////////////////////////////////////////////////////////////////////

// `pOut` should point to a block of memory with a minimum size of `ceil(originalSize / 2)`.
// returns `pOut` on the byte after the one that has been written to.
inline uint8_t * bitpack_encode4_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  int64_t i = 0;
  const int64_t lastOriginalSize = (int64_t)originalSize - sizeof(__m128i) * 2;

  for (; i <= lastOriginalSize; i += sizeof(__m128i) * 2)
  {
    const __m128i hi = _mm_loadu_si128(pIn128);
    const __m128i lo = _mm_loadu_si128(pIn128 + 1);

    const __m128i pack = _mm_or_si128(_mm_slli_epi16(hi, 4), lo);

    _mm_storeu_si128(pOut128, pack);

    pIn128 += 2;
    pOut128++;
  }

  pIn = (const uint8_t *)pIn128;
  pOut = (uint8_t *)pOut128;

  for (; i < (int64_t)originalSize - 1; i += 2)
  {
    *pOut = pIn[0] | (pIn[1] << 4);

    pIn += 2;
    pOut++;
  }

  if (i < (int64_t)originalSize)
  {
    *pOut = *pIn;
    pOut++;
  }

  return pOut;
}

// `pOut` should point to a block of memory with a minimum size of `ceil(originalSize / 2)`.
// returns `pOut` on the byte after the one that has been written to.
inline uint8_t * bitpack_encode4_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  int64_t i = 0;
  const int64_t lastOriginalSize = (int64_t)originalSize - sizeof(__m128i) * 2;

  for (; i <= lastOriginalSize; i += sizeof(__m128i) * 2)
  {
    const __m128i hi = _mm_load_si128(pIn128);
    const __m128i lo = _mm_load_si128(pIn128 + 1);

    const __m128i pack = _mm_or_si128(_mm_slli_epi16(hi, 4), lo);

    _mm_stream_si128(pOut128, pack);

    pIn128 += 2;
    pOut128++;
  }

  pIn = (const uint8_t *)pIn128;
  pOut = (uint8_t *)pOut128;

  for (; i < (int64_t)originalSize - 1; i += 2)
  {
    *pOut = pIn[0] | (pIn[1] << 4);

    pIn += 2;
    pOut++;
  }

  if (i < (int64_t)originalSize)
  {
    *pOut = *pIn;
    pOut++;
  }

  return pOut;
}

//////////////////////////////////////////////////////////////////////////

// `pIn` should point to a block of memory with a minimum size of `ceil(originalSize / 2)`.
// returns `pIn` after the last byte read.
inline const uint8_t * bitpack_decode4_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  int64_t i = 0;
  const int64_t lastOriginalSize = (int64_t)originalSize - sizeof(__m128i) * 2;

  const __m128i lo_pattern = _mm_set1_epi8(0x0F);

  for (; i <= lastOriginalSize; i += sizeof(__m128i) * 2)
  {
    const __m128i pack = _mm_loadu_si128(pIn128);

    const __m128i unpack_hi = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
    const __m128i unpack_lo = _mm_and_si128(lo_pattern, pack);

    _mm_storeu_si128(pOut128, unpack_hi);
    _mm_storeu_si128(pOut128 + 1, unpack_lo);

    pIn128++;
    pOut128 += 2;
  }

  pIn = (const uint8_t *)pIn128;
  pOut = (uint8_t *)pOut128;

  for (; i < (int64_t)originalSize - 1; i += 2)
  {
    const uint8_t sym = pIn[0];

    pOut[0] = sym & 0xF;
    pOut[1] = sym >> 4;

    pIn++;
    pOut += 2;
  }

  if (i < (int64_t)originalSize)
  {
    *pOut = *pIn;
    pIn++;
  }

  return pIn;
}

// `pIn` should point to a block of memory with a minimum size of `ceil(originalSize / 2)`.
// returns `pIn` after the last byte read.
inline const uint8_t * bitpack_decode4_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  const __m128i *pIn128 = (const __m128i *)pIn;

  int64_t i = 0;
  const int64_t lastOriginalSize = (int64_t)originalSize - sizeof(__m128i) * 2;

  const __m128i lo_pattern = _mm_set1_epi8(0x0F);

  for (; i <= lastOriginalSize; i += sizeof(__m128i) * 2)
  {
    const __m128i pack = _mm_load_si128(pIn128);

    const __m128i unpack_hi = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
    const __m128i unpack_lo = _mm_and_si128(lo_pattern, pack);

    _mm_stream_si128(pOut128, unpack_hi);
    _mm_stream_si128(pOut128 + 1, unpack_lo);

    pIn128++;
    pOut128 += 2;
  }

  pIn = (const uint8_t *)pIn128;
  pOut = (uint8_t *)pOut128;

  for (; i < (int64_t)originalSize - 1; i += 2)
  {
    const uint8_t sym = pIn[0];

    pOut[0] = sym & 0xF;
    pOut[1] = sym >> 4;

    pIn++;
    pOut += 2;
  }

  if (i < (int64_t)originalSize)
  {
    *pOut = *pIn;
    pIn++;
  }

  return pIn;
}

//////////////////////////////////////////////////////////////////////////

// `originalSize` must be a multiple of `sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize / 2`.
// returns `pOut` on the byte after the one that has been written to.
inline uint8_t *bitpack_encode4_sse2_unaligned_m128i(const __m128i *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;

  int64_t i = 0;
  const int64_t lastOriginalSize = (int64_t)originalSize - sizeof(__m128i) * 2;

  for (; i <= lastOriginalSize; i += sizeof(__m128i) * 2)
  {
    const __m128i hi = _mm_loadu_si128(pIn);
    const __m128i lo = _mm_loadu_si128(pIn + 1);

    const __m128i pack = _mm_or_si128(_mm_slli_epi16(hi, 4), lo);

    _mm_storeu_si128(pOut128, pack);

    pIn += 2;
    pOut128++;
  }

  pOut = (uint8_t *)pOut128;

  if (i < (int64_t)originalSize)
  {
    uint64_t *pIn64 = (uint64_t *)pIn;

    *(uint64_t *)pOut = pIn64[0] | (pIn64[1] << 4);

    pOut += sizeof(uint64_t);
  }

  return pOut;
}

// `originalSize` must be a multiple of `sizeof(__m128i)`.
// `pIn` should point to a block of memory with a minimum size of `originalSize / 2`.
// returns `pIn` after the last byte read.
inline const uint8_t * bitpack_decode4_sse2_unaligned_m128i(const uint8_t *pIn, __m128i *pOut, const size_t originalSize)
{
  const __m128i *pIn128 = (const __m128i *)pIn;

  int64_t i = 0;
  const int64_t lastOriginalSize = (int64_t)originalSize - sizeof(__m128i) * 2;

  const __m128i lo_pattern = _mm_set1_epi8(0x0F);

  for (; i <= lastOriginalSize; i += sizeof(__m128i) * 2)
  {
    const __m128i pack = _mm_loadu_si128(pIn128);

    const __m128i unpack_hi = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
    const __m128i unpack_lo = _mm_and_si128(lo_pattern, pack);

    _mm_storeu_si128(pOut, unpack_hi);
    _mm_storeu_si128(pOut + 1, unpack_lo);

    pIn128++;
    pOut += 2;
  }

  pIn = (const uint8_t *)pIn128;

  if (i < (int64_t)originalSize)
  {
    const uint64_t fourBitPattern = 0x0F0F0F0F0F0F0F0F;
    const uint64_t in = *(const uint64_t *)pIn;

    uint64_t *pOut64 = (uint64_t *)pOut;

    pOut64[0] = in & fourBitPattern;
    pOut64[1] = (in >> 4) & fourBitPattern;

    pIn += sizeof(uint64_t);
  }

  return pIn;
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
inline void bitpack_decode2_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
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
inline void bitpack_decode2_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
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

//////////////////////////////////////////////////////////////////////////

// `originalSize` must be a multiple of `sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize / 4`.
// returns `pOut` one byte after the last one that was written to.
inline uint8_t * bitpack_encode2_sse2_unaligned_m128i(const __m128i *pIn, uint8_t *pOut, const size_t originalSize)
{
  __m128i *pOut128 = (__m128i *)pOut;
  int64_t i = 0;
  int64_t originalSize4 = originalSize - sizeof(__m128i) * 4;

  for (; i <= originalSize4; i += sizeof(__m128i) * 4)
  {
    const __m128i p0 = _mm_loadu_si128(pIn);
    const __m128i p1 = _mm_loadu_si128(pIn + 1);
    const __m128i p2 = _mm_loadu_si128(pIn + 2);
    const __m128i p3 = _mm_loadu_si128(pIn + 3);

    const __m128i pack = _mm_or_si128(_mm_or_si128(_mm_slli_epi16(p3, 6), _mm_slli_epi16(p2, 4)), _mm_or_si128(_mm_slli_epi16(p1, 2), p0));

    _mm_storeu_si128(pOut128, pack);

    pIn += 4;
    pOut128++;
  }

  pOut = (uint8_t *)pOut128;

  if ((int64_t)(i + sizeof(uint64_t) * 4) <= (int64_t)originalSize)
  {
    i += sizeof(uint64_t) * 4;

    const uint64_t *pIn64 = (const uint64_t *)pIn;
    pIn += 2;

    const uint64_t p0 = pIn64[0];
    const uint64_t p1 = pIn64[1];
    const uint64_t p2 = pIn64[2];
    const uint64_t p3 = pIn64[3];

    const uint64_t combined = p0 | (p1 << 2) | (p2 << 4) | (p3 << 6);

    *(uint64_t *)pOut = combined;
    pOut += sizeof(uint64_t);
  }

  if (i < (int64_t)originalSize)
  {
    const uint32_t *pIn32 = (const uint32_t *)pIn;

    const uint32_t p0 = pIn32[0];
    const uint32_t p1 = pIn32[1];
    const uint32_t p2 = pIn32[2];
    const uint32_t p3 = pIn32[3];

    const uint32_t combined = p0 | (p1 << 2) | (p2 << 4) | (p3 << 6);

    *(uint32_t *)pOut = combined;
    pOut += sizeof(uint32_t);
  }

  return pOut;
}

// `originalSize` must be a multiple of `sizeof(__m128i)`.
// `pIn` should point to a block of memory with a minimum size of `originalSize / 4`.
// returns `pIn` after the last byte read.
inline const uint8_t * bitpack_decode2_sse2_unaligned_m128i(const uint8_t *pIn, __m128i *pOut, const size_t originalSize)
{
  const __m128i *pIn128 = (const __m128i *)pIn;

  int64_t i = 0;
  int64_t originalSize4 = originalSize - sizeof(__m128i) * 4;

  const __m128i lo_pattern = _mm_set1_epi8(3);

  for (; i <= originalSize4; i += sizeof(__m128i) * 4)
  {
    const __m128i pack = _mm_loadu_si128(pIn128);

    const __m128i unpack0 = _mm_and_si128(lo_pattern, pack);
    const __m128i unpack1 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 2));
    const __m128i unpack2 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
    const __m128i unpack3 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 6));

    _mm_storeu_si128(pOut, unpack0);
    _mm_storeu_si128(pOut + 1, unpack1);
    _mm_storeu_si128(pOut + 2, unpack2);
    _mm_storeu_si128(pOut + 3, unpack3);

    pIn128++;
    pOut += 4;
  }

  pIn = (uint8_t *)pIn128;

  if ((int64_t)(i + sizeof(uint64_t) * 4) <= (int64_t)originalSize)
  {
    i += sizeof(uint64_t) * 4;

    const uint64_t combined = *(uint64_t *)pIn;
    pIn += sizeof(uint64_t);

    const uint64_t lo_pattern_64 = 0x0303030303030303;

    uint64_t *pOut64 = (uint64_t *)pOut;

    pOut64[0] = combined & lo_pattern_64;
    pOut64[1] = (combined >> 2) & lo_pattern_64;
    pOut64[2] = (combined >> 4) & lo_pattern_64;
    pOut64[3] = (combined >> 6) & lo_pattern_64;

    pOut += 2;
  }

  if (i < (int64_t)originalSize)
  {
    const uint32_t combined = *(uint32_t *)pIn;
    pIn += sizeof(uint32_t);

    const uint32_t lo_pattern_32 = 0x03030303;

    uint32_t *pOut32 = (uint32_t *)pOut;

    pOut32[0] = combined & lo_pattern_32;
    pOut32[1] = (combined >> 2) & lo_pattern_32;
    pOut32[2] = (combined >> 4) & lo_pattern_32;
    pOut32[3] = (combined >> 6) & lo_pattern_32;
  }

  return pIn;
}

//////////////////////////////////////////////////////////////////////////

// Bitpack lowest bit from __m128i into uint16_t.
// `originalSize` must be a multiple of __m128i.
// `pOut` must contain at least `originalSize / 8` bytes.
inline void bitpack_encode1_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  const __m128i *pIn128 = (const __m128i *)pIn;
  uint16_t *pOut16 = (uint16_t *)pOut;

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i))
  {
    const __m128i v = _mm_load_si128(pIn128);

    const uint32_t mask = _mm_movemask_epi8(_mm_slli_epi16(v, 7));

    *pOut16 = (uint16_t)mask;

    pOut16++;
    pIn128++;
  }
}

// Bitpack lowest bit from __m128i into uint16_t.
// `originalSize` must be a multiple of __m128i.
// `pOut` must contain at least `originalSize / 8` bytes.
inline void bitpack_encode1_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t originalSize)
{
  const __m128i *pIn128 = (const __m128i *)pIn;
  uint16_t *pOut16 = (uint16_t *)pOut;

  for (size_t i = 0; i < originalSize; i += sizeof(__m128i))
  {
    const __m128i v = _mm_loadu_si128(pIn128);

    const uint32_t mask = _mm_movemask_epi8(_mm_slli_epi16(v, 7));

    *pOut16 = (uint16_t)mask;

    pOut16++;
    pIn128++;
  }
}

//////////////////////////////////////////////////////////////////////////

// Bitpack 1 bit from uint16_t into __m128i.
// `originalSize` must be a multiple of uint16_t.
// `pOut` must contain at least `encodedSize * 8` bytes.
inline void bitpack_decode1_sse2_aligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  const uint16_t *pIn16 = (uint16_t *)pIn;
  __m128i *pOut128 = (__m128i *)pOut;

  const __m128i patternLow1 = _mm_set1_epi8(1);
  const __m128i patternBitSelect = _mm_set1_epi64x((int64_t)0x8040201008040201);

  for (size_t i = 0; i < encodedSize; i += sizeof(uint16_t))
  {
    const uint16_t mask = *pIn16;

    __m128i v = _mm_set1_epi16(mask);

    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);

    v = _mm_and_si128(v, patternBitSelect);
    v = _mm_min_epu8(v, patternLow1);

    _mm_stream_si128(pOut128, v);

    pIn16++;
    pOut128++;
  }
}

// Bitpack 1 bit from uint16_t into __m128i.
// `originalSize` must be a multiple of uint16_t.
// `pOut` must contain at least `encodedSize * 8` bytes.
inline void bitpack_decode1_sse2_unaligned(const uint8_t *pIn, uint8_t *pOut, const size_t encodedSize)
{
  const uint16_t *pIn16 = (uint16_t *)pIn;
  __m128i *pOut128 = (__m128i *)pOut;

  const __m128i patternLow1 = _mm_set1_epi8(1);
  const __m128i patternBitSelect = _mm_set1_epi64x((int64_t)0x8040201008040201);

  for (size_t i = 0; i < encodedSize; i += sizeof(uint16_t))
  {
    const uint16_t mask = *pIn16;

    __m128i v = _mm_set1_epi16(mask);

    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);
    v = _mm_unpacklo_epi8(v, v);

    v = _mm_and_si128(v, patternBitSelect);
    v = _mm_min_epu8(v, patternLow1);

    _mm_storeu_si128(pOut128, v);

    pIn16++;
    pOut128++;
  }
}
