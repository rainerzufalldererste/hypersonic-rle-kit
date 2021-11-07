#include "rle8.h"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

uint32_t rle_mmtf_compress_bounds(const uint32_t inSize)
{
  return inSize;
}

uint32_t rle_mmtf_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (inSize > outSize)
    return 0;

  ALIGN(128) uint8_t history[16 * 256];

  for (size_t i = 0; i < 256; i++)
    _mm_store_si128(history + i * sizeof(__m128i), _mm_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;

  while (inSizeRemaining >= sizeof(__m128i))
  {
    inSizeRemaining -= sizeof(__m128i);

    const __m128i symbols = _mm_loadu_si128(pIn); // could be stream load with sse 4.1.
    pIn += sizeof(__m128i);

    __m128i currentHist = _mm_load_si128(history);
    __m128i matched = _mm_cmpeq_epi8(symbols, currentHist);
    __m128i out = _mm_and_si128(matched, _mm_setzero_si128());
    __m128i prevMatched = matched;
    __m128i history0 = _mm_and_si128(matched, currentHist);
    __m128i lastHist;

    for (size_t i = 1; i < 256; i++)
    {
      lastHist = currentHist;
      currentHist = _mm_load_si128(history + i * sizeof(__m128i));
      matched = _mm_cmpeq_epi8(symbols, currentHist);
      out = _mm_or_si128(out, _mm_and_si128(matched, _mm_set1_epi8(i)));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm_or_si128(history0, _mm_and_si128(matched, currentHist));
      _mm_store_si128(history + i * sizeof(__m128i), _mm_or_si128(_mm_andnot_si128(prevMatched, lastHist), _mm_and_si128(prevMatched, currentHist)));

      prevMatched = _mm_or_si128(prevMatched, matched);
    }

    _mm_store_si128(history, history0);
    _mm_store_si128(pOut, out);

    pOut += sizeof(__m128i);
  }

  for (size_t i = 0; i < inSizeRemaining; i++)
  {
    const uint8_t symbol = *pIn;
    pIn++;

    for (size_t d = 0; d < 255; d++)
    {
      if (history[d * 16 + i] == symbol)
      {
        *pOut = (uint8_t)d;
        pOut++;
        break;
      }
    }
  }

  return inSize;
}

uint32_t rle_mmtf_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  return 0;
}