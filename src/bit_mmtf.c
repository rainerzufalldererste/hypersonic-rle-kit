#include "rle.h"

#ifdef _MSC_VER
  #include <intrin.h>
#else
  #include <x86intrin.h>
#endif

//////////////////////////////////////////////////////////////////////////

uint32_t bitmmtf_bounds(const uint32_t inSize)
{
  return inSize;
}

//////////////////////////////////////////////////////////////////////////

uint32_t bitmmtf8_encode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize > outSize)
    return 0;

  __m128i state = _mm_setzero_si128();
  const __m128i mask = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0b10000000, 0b1000000, 0b100000, 0b10000, 0b1000, 0b100, 0b10, 0b1);

  for (size_t i = 0; i < inSize; i++)
  {
    const __m128i current = _mm_and_si128(mask, _mm_set1_epi8(pIn[i]));
    const __m128i match = _mm_cmpeq_epi8(current, state);
    
    state = current;

    const uint32_t bitmask = ~_mm_movemask_epi8(match);
    pOut[i] = (uint8_t)bitmask;
  }

  return inSize;
}

uint32_t bitmmtf8_decode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize > outSize)
    return 0;

  __m128i state = _mm_setzero_si128();
  const __m128i mask = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0b10000000, 0b1000000, 0b100000, 0b10000, 0b1000, 0b100, 0b10, 0b1);

  for (size_t i = 0; i < inSize; i++)
  {
    const __m128i eqmask = _mm_cmpeq_epi8(mask, _mm_and_si128(mask, _mm_set1_epi8(pIn[i])));
    const __m128i last = _mm_xor_si128(eqmask, state);
    
    state = last;

    const uint32_t bitmask = _mm_movemask_epi8(last);
    pOut[i] = (uint8_t)bitmask;
  }

  return inSize;
}

//////////////////////////////////////////////////////////////////////////

uint32_t bitmmtf16_encode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize > outSize)
    return 0;

  __m128i state = _mm_setzero_si128();
  const __m128i mask = _mm_set_epi8(0b10000000, 0b1000000, 0b100000, 0b10000, 0b1000, 0b100, 0b10, 0b1, 0b10000000, 0b1000000, 0b100000, 0b10000, 0b1000, 0b100, 0b10, 0b1);
  
  const size_t loopSize = (size_t)inSize & ~(size_t)1;

  for (size_t i = 0; i < loopSize; i += 2)
  {
    const uint16_t v = *(uint16_t *)&pIn[i];

    const __m128i a = _mm_set1_epi8((uint8_t)v);
    const __m128i b = _mm_set1_epi8((uint8_t)(v >> 8));
    const __m128i ab = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), _MM_SHUFFLE(1, 1, 0, 0)));

    const __m128i current = _mm_and_si128(mask, ab);
    const __m128i match = _mm_cmpeq_epi8(current, state);
    
    state = current;

    const uint32_t bitmask = ~_mm_movemask_epi8(match);
    *(uint16_t *)&pOut[i] = (uint16_t)bitmask;
  }

  if (inSize & 1)
    pOut[inSize - 1] = pIn[inSize - 1];

  return inSize;
}

uint32_t bitmmtf16_decode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize > outSize)
    return 0;

  __m128i state = _mm_setzero_si128();
  const __m128i mask = _mm_set_epi8(0b10000000, 0b1000000, 0b100000, 0b10000, 0b1000, 0b100, 0b10, 0b1, 0b10000000, 0b1000000, 0b100000, 0b10000, 0b1000, 0b100, 0b10, 0b1);

  const size_t loopSize = (size_t)inSize & ~(size_t)1;

  for (size_t i = 0; i < loopSize; i += 2)
  {
    const uint16_t v = *(uint16_t *)&pIn[i];

    const __m128i a = _mm_set1_epi8((uint8_t)v);
    const __m128i b = _mm_set1_epi8((uint8_t)(v >> 8));
    const __m128i ab = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), _MM_SHUFFLE(1, 1, 0, 0)));

    const __m128i eqmask = _mm_cmpeq_epi8(mask, _mm_and_si128(mask, ab));
    const __m128i last = _mm_xor_si128(eqmask, state);
    
    state = last;

    const uint32_t bitmask = _mm_movemask_epi8(last);
    *(uint16_t *)&pOut[i] = (uint16_t)bitmask;
  }

  if (inSize & 1)
    pOut[inSize - 1] = pIn[inSize - 1];

  return inSize;
}
