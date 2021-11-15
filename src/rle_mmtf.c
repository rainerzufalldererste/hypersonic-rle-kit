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

  ALIGN(128) uint8_t history[sizeof(__m256i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm256_store_si256(history + i * sizeof(__m256i), _mm256_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;

  while (inSizeRemaining >= sizeof(__m256i))
  {
    inSizeRemaining -= sizeof(__m256i);

    const __m256i symbols = _mm256_loadu_si256(pIn); // could be stream load with sse 4.1.
    pIn += sizeof(__m256i);

    __m256i currentHist = _mm256_load_si256(history);
    __m256i matched = _mm256_cmpeq_epi8(symbols, currentHist);
    __m256i out = _mm256_and_si256(matched, _mm256_setzero_si256());
    __m256i prevMatched = matched;
    __m256i history0 = _mm256_and_si256(matched, currentHist);
    __m256i lastHist;

    for (size_t i = 1; i < 256; i++)
    {
      if (0xFFFFFFFF == _mm256_movemask_epi8(prevMatched))
        break;

      lastHist = currentHist;
      currentHist = _mm256_load_si256(history + i * sizeof(__m256i));
      matched = _mm256_cmpeq_epi8(symbols, currentHist);
      out = _mm256_or_si256(out, _mm256_and_si256(matched, _mm256_set1_epi8(i)));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm256_or_si256(history0, _mm256_and_si256(matched, currentHist));
      _mm256_store_si256(history + i * sizeof(__m256i), _mm256_or_si256(_mm256_andnot_si256(prevMatched, lastHist), _mm256_and_si256(prevMatched, currentHist)));

      prevMatched = _mm256_or_si256(prevMatched, matched);
    }

    _mm256_store_si256(history, history0);
    _mm256_store_si256(pOut, out);

    pOut += sizeof(__m256i);
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