#include "rle8.h"
#include "rleX_extreme_common.h"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

//////////////////////////////////////////////////////////////////////////

uint32_t rle_mmtf128_encode_aligned_sse41(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);
uint32_t rle_mmtf128_encode_unaligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);

uint32_t rle_mmtf128_decode_aligned_sse41(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);
uint32_t rle_mmtf128_decode_unaligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);

uint32_t rle_mmtf256_encode_aligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);
uint32_t rle_mmtf256_encode_unaligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);
uint32_t rle_mmtf256_encode_unaligned_sse2(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);

uint32_t rle_mmtf256_decode_aligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);
uint32_t rle_mmtf256_decode_unaligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);
uint32_t rle_mmtf256_decode_unaligned_sse2(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut);

//////////////////////////////////////////////////////////////////////////

uint32_t rle_mmtf_bounds(const uint32_t inSize)
{
  return inSize;
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle_mmtf128_encode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (inSize > outSize)
    return 0;

  _DetectCPUFeatures();

  if (!sse2Supported)
    return 0;

  if (((uint64_t)pIn & 15) == 0 && ((uint64_t)pOut & 31) == 0 && sse41Supported)
    return rle_mmtf128_encode_aligned_sse41(pIn, inSize, pOut);
  else
    return rle_mmtf128_encode_unaligned(pIn, inSize, pOut);
}

uint32_t rle_mmtf128_decode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (inSize > outSize)
    return 0;

  _DetectCPUFeatures();

  if (!sse2Supported)
    return 0;

  if (((uint64_t)pIn & 15) == 0 && ((uint64_t)pOut & 31) == 0 && sse41Supported)
    return rle_mmtf128_decode_aligned_sse41(pIn, inSize, pOut);
  else
    return rle_mmtf128_decode_unaligned(pIn, inSize, pOut);
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle_mmtf256_encode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (inSize > outSize)
    return 0;

  _DetectCPUFeatures();
  
  if (!sse2Supported)
    return 0;

  if (!avx2Supported)
    return rle_mmtf256_encode_unaligned_sse2(pIn, inSize, pOut);

  if (((uint64_t)pIn & 31) == 0 && ((uint64_t)pOut & 31) == 0)
    return rle_mmtf256_encode_aligned(pIn, inSize, pOut);
  else
    return rle_mmtf256_encode_unaligned(pIn, inSize, pOut);
}

uint32_t rle_mmtf256_decode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (inSize > outSize)
    return 0;

  _DetectCPUFeatures();

  if (!sse2Supported)
    return 0;

  if (!avx2Supported)
    return rle_mmtf256_decode_unaligned_sse2(pIn, inSize, pOut);

  if (((uint64_t)pIn & 31) == 0 && ((uint64_t)pOut & 31) == 0)
    return rle_mmtf256_decode_aligned(pIn, inSize, pOut);
  else
    return rle_mmtf256_decode_unaligned(pIn, inSize, pOut);
}

//////////////////////////////////////////////////////////////////////////

#ifndef _MSC_VER
__attribute__((target("sse4.1")))
#endif
uint32_t rle_mmtf128_encode_aligned_sse41(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(16) uint8_t history[sizeof(__m128i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm_store_si128((__m128i *)(history) + i, _mm_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;
  const __m128i one = _mm_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m128i))
  {
    inSizeRemaining -= sizeof(__m128i);

    const __m128i symbols = _mm_stream_load_si128((__m128i *)pIn);
    pIn += sizeof(__m128i);

    __m128i index = _mm_setzero_si128();
    __m128i currentHist = _mm_load_si128((__m128i *)history);
    __m128i matched = _mm_cmpeq_epi8(symbols, currentHist);
    __m128i out = _mm_setzero_si128();
    __m128i prevMatched = matched;
    __m128i history0 = _mm_and_si128(matched, currentHist);
    __m128i lastHist;
    __m128i *pHistory = ((__m128i *)history) + 1;

    while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
    {
      lastHist = currentHist;
      currentHist = _mm_load_si128(pHistory);

      matched = _mm_cmpeq_epi8(symbols, currentHist);
      index = _mm_add_epi8(index, one);
      out = _mm_or_si128(out, _mm_and_si128(matched, index));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm_or_si128(history0, _mm_and_si128(matched, currentHist));
      _mm_store_si128(pHistory, _mm_or_si128(_mm_andnot_si128(prevMatched, lastHist), _mm_and_si128(prevMatched, currentHist)));
      
      prevMatched = _mm_or_si128(prevMatched, matched);
      pHistory++;
    }

    _mm_store_si128((__m128i *)history, history0);
    _mm_stream_si128((__m128i *)pOut, out);

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

uint32_t rle_mmtf128_encode_unaligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(16) uint8_t history[sizeof(__m128i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm_store_si128((__m128i *)(history) + i, _mm_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;
  const __m128i one = _mm_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m128i))
  {
    inSizeRemaining -= sizeof(__m128i);

    const __m128i symbols = _mm_loadu_si128((__m128i *)pIn);
    pIn += sizeof(__m128i);

    __m128i index = _mm_setzero_si128();
    __m128i currentHist = _mm_load_si128((__m128i *)history);
    __m128i matched = _mm_cmpeq_epi8(symbols, currentHist);
    __m128i out = _mm_setzero_si128();
    __m128i prevMatched = matched;
    __m128i history0 = _mm_and_si128(matched, currentHist);
    __m128i lastHist;
    __m128i *pHistory = ((__m128i *)history) + 1;

    while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
    {
      lastHist = currentHist;
      currentHist = _mm_load_si128(pHistory);

      matched = _mm_cmpeq_epi8(symbols, currentHist);
      index = _mm_add_epi8(index, one);
      out = _mm_or_si128(out, _mm_and_si128(matched, index));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm_or_si128(history0, _mm_and_si128(matched, currentHist));
      _mm_store_si128(pHistory, _mm_or_si128(_mm_andnot_si128(prevMatched, lastHist), _mm_and_si128(prevMatched, currentHist)));
      
      prevMatched = _mm_or_si128(prevMatched, matched);
      pHistory++;
    }

    _mm_store_si128((__m128i *)history, history0);
    _mm_storeu_si128((__m128i *)pOut, out);

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

//////////////////////////////////////////////////////////////////////////

#ifndef _MSC_VER
__attribute__((target("sse4.1")))
#endif
uint32_t rle_mmtf128_decode_aligned_sse41(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(16) uint8_t history[sizeof(__m128i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm_store_si128((__m128i *)(history) + i, _mm_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;
  const __m128i one = _mm_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m128i))
  {
    inSizeRemaining -= sizeof(__m128i);

    const __m128i indices = _mm_stream_load_si128((__m128i *)pIn);
    pIn += sizeof(__m128i);

    __m128i index = _mm_setzero_si128();
    __m128i matched = _mm_cmpeq_epi8(indices, index);
    __m128i currentHist = _mm_load_si128((__m128i *)history);
    __m128i out = _mm_and_si128(currentHist, matched);
    __m128i prevMatched = matched;
    __m128i history0 = out;
    __m128i lastHist;
    __m128i *pHistory = ((__m128i *)history) + 1;
    
    while(0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
    {
      lastHist = currentHist;
      index = _mm_add_epi8(index, one);
      matched = _mm_cmpeq_epi8(indices, index);

      currentHist = _mm_load_si128(pHistory);

      out = _mm_or_si128(out, _mm_and_si128(matched, currentHist));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm_or_si128(history0, _mm_and_si128(matched, currentHist));
      _mm_store_si128(pHistory, _mm_or_si128(_mm_andnot_si128(prevMatched, lastHist), _mm_and_si128(prevMatched, currentHist)));

      prevMatched = _mm_or_si128(prevMatched, matched);
      pHistory++;
    }

    _mm_store_si128((__m128i *)history, history0);
    _mm_stream_si128((__m128i *)pOut, out);

    pOut += sizeof(__m128i);
  }

  for (size_t i = 0; i < inSizeRemaining; i++)
  {
    const uint8_t index = *pIn;
    pIn++;

    *pOut = history[(uint64_t)index * 16 + i];
    pOut++;
  }

  return inSize;
}

uint32_t rle_mmtf128_decode_unaligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(16) uint8_t history[sizeof(__m128i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm_store_si128((__m128i *)(history) + i, _mm_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;
  const __m128i one = _mm_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m128i))
  {
    inSizeRemaining -= sizeof(__m128i);

    const __m128i indices = _mm_loadu_si128((__m128i *)pIn);
    pIn += sizeof(__m128i);

    __m128i index = _mm_setzero_si128();
    __m128i matched = _mm_cmpeq_epi8(indices, index);
    __m128i currentHist = _mm_load_si128((__m128i *)history);
    __m128i out = _mm_and_si128(currentHist, matched);
    __m128i prevMatched = matched;
    __m128i history0 = out;
    __m128i lastHist;
    __m128i *pHistory = ((__m128i *)history) + 1;
    
    while(0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
    {
      lastHist = currentHist;
      index = _mm_add_epi8(index, one);
      matched = _mm_cmpeq_epi8(indices, index);

      currentHist = _mm_load_si128(pHistory);

      out = _mm_or_si128(out, _mm_and_si128(matched, currentHist));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm_or_si128(history0, _mm_and_si128(matched, currentHist));
      _mm_store_si128(pHistory, _mm_or_si128(_mm_andnot_si128(prevMatched, lastHist), _mm_and_si128(prevMatched, currentHist)));

      prevMatched = _mm_or_si128(prevMatched, matched);
      pHistory++;
    }

    _mm_store_si128((__m128i *)history, history0);
    _mm_storeu_si128((__m128i *)pOut, out);

    pOut += sizeof(__m128i);
  }

  for (size_t i = 0; i < inSizeRemaining; i++)
  {
    const uint8_t index = *pIn;
    pIn++;

    *pOut = history[(uint64_t)index * 16 + i];
    pOut++;
  }

  return inSize;
}

//////////////////////////////////////////////////////////////////////////

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
uint32_t rle_mmtf256_encode_aligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(32) uint8_t history[sizeof(__m256i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm256_store_si256((__m256i *)(history) + i, _mm256_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;
  const __m256i one = _mm256_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m256i))
  {
    inSizeRemaining -= sizeof(__m256i);

    const __m256i symbols = _mm256_stream_load_si256((__m256i *)pIn);
    pIn += sizeof(__m256i);

    __m256i index = _mm256_setzero_si256();
    __m256i currentHist = _mm256_load_si256((__m256i *)history);
    __m256i matched = _mm256_cmpeq_epi8(symbols, currentHist);
    __m256i out = _mm256_setzero_si256();
    __m256i prevMatched = matched;
    __m256i history0 = _mm256_and_si256(matched, currentHist);
    __m256i lastHist;
    __m256i *pHistory = ((__m256i *)history) + 1;

    while (0xFFFFFFFF != _mm256_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
    {
      lastHist = currentHist;
      currentHist = _mm256_load_si256(pHistory);

      matched = _mm256_cmpeq_epi8(symbols, currentHist);
      index = _mm256_add_epi8(index, one);
      out = _mm256_or_si256(out, _mm256_and_si256(matched, index));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm256_or_si256(history0, _mm256_and_si256(matched, currentHist));
      _mm256_store_si256(pHistory, _mm256_or_si256(_mm256_andnot_si256(prevMatched, lastHist), _mm256_and_si256(prevMatched, currentHist)));
      
      prevMatched = _mm256_or_si256(prevMatched, matched);
      pHistory++;
    }

    _mm256_store_si256((__m256i *)history, history0);
    _mm256_stream_si256((__m256i *)pOut, out);

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

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
uint32_t rle_mmtf256_encode_unaligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(32) uint8_t history[sizeof(__m256i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm256_store_si256((__m256i *)(history) + i, _mm256_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;
  const __m256i one = _mm256_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m256i))
  {
    inSizeRemaining -= sizeof(__m256i);

    const __m256i symbols = _mm256_loadu_si256((__m256i *)pIn);
    pIn += sizeof(__m256i);

    __m256i index = _mm256_setzero_si256();
    __m256i currentHist = _mm256_load_si256((__m256i *)history);
    __m256i matched = _mm256_cmpeq_epi8(symbols, currentHist);
    __m256i out = _mm256_setzero_si256();
    __m256i prevMatched = matched;
    __m256i history0 = _mm256_and_si256(matched, currentHist);
    __m256i lastHist;
    __m256i *pHistory = ((__m256i *)history) + 1;

    while (0xFFFFFFFF != _mm256_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
    {
      lastHist = currentHist;
      currentHist = _mm256_load_si256(pHistory);

      matched = _mm256_cmpeq_epi8(symbols, currentHist);
      index = _mm256_add_epi8(index, one);
      out = _mm256_or_si256(out, _mm256_and_si256(matched, index));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm256_or_si256(history0, _mm256_and_si256(matched, currentHist));
      _mm256_store_si256(pHistory, _mm256_or_si256(_mm256_andnot_si256(prevMatched, lastHist), _mm256_and_si256(prevMatched, currentHist)));
      
      prevMatched = _mm256_or_si256(prevMatched, matched);
      pHistory++;
    }

    _mm256_store_si256((__m256i *)history, history0);
    _mm256_storeu_si256((__m256i *)pOut, out);

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

uint32_t rle_mmtf256_encode_unaligned_sse2(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(16) uint8_t history[sizeof(__m128i) * 2 * 256];

  for (size_t i = 0; i < 256; i++)
  {
    _mm_store_si128((__m128i *)(history) + i * 2, _mm_set1_epi8((char)i));
    _mm_store_si128((__m128i *)(history) + i * 2 + 1, _mm_set1_epi8((char)i));
  }

  uint32_t inSizeRemaining = inSize;
  const __m128i one = _mm_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m128i) * 2)
  {
    inSizeRemaining -= sizeof(__m128i) * 2;

    const __m128i symbols_lo = _mm_loadu_si128((__m128i *)pIn);
    const __m128i symbols_hi = _mm_loadu_si128((__m128i *)pIn + 1);
    pIn += sizeof(__m128i) * 2;

    __m128i index = _mm_setzero_si128();
    __m128i currentHist_lo = _mm_load_si128((__m128i *)history);
    __m128i currentHist_hi = _mm_load_si128((__m128i *)history + 1);
    __m128i matched_lo = _mm_cmpeq_epi8(symbols_lo, currentHist_lo);
    __m128i matched_hi = _mm_cmpeq_epi8(symbols_hi, currentHist_hi);
    __m128i out_lo = _mm_setzero_si128();
    __m128i out_hi = _mm_setzero_si128();
    __m128i prevMatched_lo = matched_lo;
    __m128i prevMatched_hi = matched_hi;
    __m128i history0_lo = _mm_and_si128(matched_lo, currentHist_lo);
    __m128i history0_hi = _mm_and_si128(matched_hi, currentHist_hi);
    __m128i lastHist_lo;
    __m128i lastHist_hi;
    __m128i *pHistory = ((__m128i *)history) + 2;

    while (0xFFFF != _mm_movemask_epi8(prevMatched_lo) || 0xFFFF != _mm_movemask_epi8(prevMatched_hi)) // We're gonna find that byte eventually.
    {
      lastHist_lo = currentHist_lo;
      lastHist_hi = currentHist_hi;
      currentHist_lo = _mm_load_si128(pHistory);
      currentHist_hi = _mm_load_si128(pHistory + 1);

      matched_lo = _mm_cmpeq_epi8(symbols_lo, currentHist_lo);
      matched_hi = _mm_cmpeq_epi8(symbols_hi, currentHist_hi);
      index = _mm_add_epi8(index, one);
      out_lo = _mm_or_si128(out_lo, _mm_and_si128(matched_lo, index));
      out_hi = _mm_or_si128(out_hi, _mm_and_si128(matched_hi, index));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0_lo = _mm_or_si128(history0_lo, _mm_and_si128(matched_lo, currentHist_lo));
      history0_hi = _mm_or_si128(history0_hi, _mm_and_si128(matched_hi, currentHist_hi));
      _mm_store_si128(pHistory, _mm_or_si128(_mm_andnot_si128(prevMatched_lo, lastHist_lo), _mm_and_si128(prevMatched_lo, currentHist_lo)));
      _mm_store_si128(pHistory + 1, _mm_or_si128(_mm_andnot_si128(prevMatched_hi, lastHist_hi), _mm_and_si128(prevMatched_hi, currentHist_hi)));

      prevMatched_lo = _mm_or_si128(prevMatched_lo, matched_lo);
      prevMatched_hi = _mm_or_si128(prevMatched_hi, matched_hi);
      pHistory += 2;
    }

    _mm_store_si128((__m128i *)history + 0, history0_lo);
    _mm_store_si128((__m128i *)history + 1, history0_hi);
    _mm_storeu_si128((__m128i *)pOut, out_lo);
    _mm_storeu_si128((__m128i *)pOut + 1, out_hi);

    pOut += sizeof(__m128i) * 2;
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

//////////////////////////////////////////////////////////////////////////

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
uint32_t rle_mmtf256_decode_aligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(32) uint8_t history[sizeof(__m256i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm256_store_si256((__m256i *)(history) + i, _mm256_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;
  const __m256i one = _mm256_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m256i))
  {
    inSizeRemaining -= sizeof(__m256i);

    const __m256i indices = _mm256_stream_load_si256((__m256i *)pIn);
    pIn += sizeof(__m256i);

    __m256i index = _mm256_setzero_si256();
    __m256i matched = _mm256_cmpeq_epi8(indices, index);
    __m256i currentHist = _mm256_load_si256((__m256i *)history);
    __m256i out = _mm256_and_si256(currentHist, matched);
    __m256i prevMatched = matched;
    __m256i history0 = out;
    __m256i lastHist;
    __m256i *pHistory = ((__m256i *)history) + 1;

    while (0xFFFFFFFF != _mm256_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
    {
      lastHist = currentHist;
      index = _mm256_add_epi8(index, one);
      matched = _mm256_cmpeq_epi8(indices, index);

      currentHist = _mm256_load_si256(pHistory);

      out = _mm256_or_si256(out, _mm256_and_si256(matched, currentHist));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm256_or_si256(history0, _mm256_and_si256(matched, currentHist));
      _mm256_store_si256(pHistory, _mm256_or_si256(_mm256_andnot_si256(prevMatched, lastHist), _mm256_and_si256(prevMatched, currentHist)));

      prevMatched = _mm256_or_si256(prevMatched, matched);
      pHistory++;
    }

    _mm256_store_si256((__m256i *)history, history0);
    _mm256_stream_si256((__m256i *)pOut, out);

    pOut += sizeof(__m256i);
  }

  for (size_t i = 0; i < inSizeRemaining; i++)
  {
    const uint8_t index = *pIn;
    pIn++;

    *pOut = history[(uint64_t)index * 16 + i];
    pOut++;
  }

  return inSize;
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
uint32_t rle_mmtf256_decode_unaligned(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(32) uint8_t history[sizeof(__m256i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm256_store_si256((__m256i *)(history) + i, _mm256_set1_epi8((char)i));

  uint32_t inSizeRemaining = inSize;
  const __m256i one = _mm256_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m256i))
  {
    inSizeRemaining -= sizeof(__m256i);

    const __m256i indices = _mm256_loadu_si256((__m256i *)pIn);
    pIn += sizeof(__m256i);

    __m256i index = _mm256_setzero_si256();
    __m256i matched = _mm256_cmpeq_epi8(indices, index);
    __m256i currentHist = _mm256_load_si256((__m256i *)history);
    __m256i out = _mm256_and_si256(currentHist, matched);
    __m256i prevMatched = matched;
    __m256i history0 = out;
    __m256i lastHist;
    __m256i *pHistory = ((__m256i *)history) + 1;

    while (0xFFFFFFFF != _mm256_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
    {
      lastHist = currentHist;
      index = _mm256_add_epi8(index, one);
      matched = _mm256_cmpeq_epi8(indices, index);

      currentHist = _mm256_load_si256(pHistory);

      out = _mm256_or_si256(out, _mm256_and_si256(matched, currentHist));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0 = _mm256_or_si256(history0, _mm256_and_si256(matched, currentHist));
      _mm256_store_si256(pHistory, _mm256_or_si256(_mm256_andnot_si256(prevMatched, lastHist), _mm256_and_si256(prevMatched, currentHist)));

      prevMatched = _mm256_or_si256(prevMatched, matched);
      pHistory++;
    }

    _mm256_store_si256((__m256i *)history, history0);
    _mm256_storeu_si256((__m256i *)pOut, out);

    pOut += sizeof(__m256i);
  }

  for (size_t i = 0; i < inSizeRemaining; i++)
  {
    const uint8_t index = *pIn;
    pIn++;

    *pOut = history[(uint64_t)index * 16 + i];
    pOut++;
  }

  return inSize;
}

uint32_t rle_mmtf256_decode_unaligned_sse2(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut)
{
  ALIGN(16) uint8_t history[sizeof(__m128i) * 2 * 256];

  for (size_t i = 0; i < 256; i++)
  {
    _mm_store_si128((__m128i *)(history) + i * 2 + 0, _mm_set1_epi8((char)i));
    _mm_store_si128((__m128i *)(history) + i * 2 + 1, _mm_set1_epi8((char)i));
  }

  uint32_t inSizeRemaining = inSize;
  const __m128i one = _mm_set1_epi8(1);

  while (inSizeRemaining >= sizeof(__m128i) * 2)
  {
    inSizeRemaining -= sizeof(__m128i) * 2;

    const __m128i indices_lo = _mm_loadu_si128((__m128i *)pIn + 0);
    const __m128i indices_hi = _mm_loadu_si128((__m128i *)pIn + 1);
    pIn += sizeof(__m128i) * 2;

    __m128i index = _mm_setzero_si128();
    __m128i matched_lo = _mm_cmpeq_epi8(indices_lo, index);
    __m128i matched_hi = _mm_cmpeq_epi8(indices_hi, index);
    __m128i currentHist_lo = _mm_load_si128((__m128i *)history + 0);
    __m128i currentHist_hi = _mm_load_si128((__m128i *)history + 1);
    __m128i out_lo = _mm_and_si128(currentHist_lo, matched_lo);
    __m128i out_hi = _mm_and_si128(currentHist_hi, matched_hi);
    __m128i prevMatched_lo = matched_lo;
    __m128i prevMatched_hi = matched_hi;
    __m128i history0_lo = out_lo;
    __m128i history0_hi = out_hi;
    __m128i lastHist_lo;
    __m128i lastHist_hi;
    __m128i *pHistory = ((__m128i *)history) + 2;

    while (0xFFFF != _mm_movemask_epi8(prevMatched_lo) || 0xFFFF != _mm_movemask_epi8(prevMatched_hi)) // We're gonna find that byte eventually.
    {
      lastHist_lo = currentHist_lo;
      lastHist_hi = currentHist_hi;
      index = _mm_add_epi8(index, one);
      matched_lo = _mm_cmpeq_epi8(indices_lo, index);
      matched_hi = _mm_cmpeq_epi8(indices_hi, index);

      currentHist_lo = _mm_load_si128(pHistory + 0);
      currentHist_hi = _mm_load_si128(pHistory + 1);

      out_lo = _mm_or_si128(out_lo, _mm_and_si128(matched_lo, currentHist_lo));
      out_hi = _mm_or_si128(out_hi, _mm_and_si128(matched_hi, currentHist_hi));

      // Overwrite History With Last History If Previously Not Matched, Or The Current History.
      history0_lo = _mm_or_si128(history0_lo, _mm_and_si128(matched_lo, currentHist_lo));
      history0_hi = _mm_or_si128(history0_hi, _mm_and_si128(matched_hi, currentHist_hi));
      _mm_store_si128(pHistory + 0, _mm_or_si128(_mm_andnot_si128(prevMatched_lo, lastHist_lo), _mm_and_si128(prevMatched_lo, currentHist_lo)));
      _mm_store_si128(pHistory + 1, _mm_or_si128(_mm_andnot_si128(prevMatched_hi, lastHist_hi), _mm_and_si128(prevMatched_hi, currentHist_hi)));

      prevMatched_lo = _mm_or_si128(prevMatched_lo, matched_lo);
      prevMatched_hi = _mm_or_si128(prevMatched_hi, matched_hi);
      pHistory += 2;
    }

    _mm_store_si128((__m128i *)history + 0, history0_lo);
    _mm_store_si128((__m128i *)history + 1, history0_hi);
    _mm_storeu_si128((__m128i *)pOut + 0, out_lo);
    _mm_storeu_si128((__m128i *)pOut + 1, out_hi);

    pOut += sizeof(__m128i) * 2;
  }

  for (size_t i = 0; i < inSizeRemaining; i++)
  {
    const uint8_t index = *pIn;
    pIn++;

    *pOut = history[(uint64_t)index * 16 + i];
    pOut++;
  }

  return inSize;
}