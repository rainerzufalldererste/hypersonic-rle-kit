#include "rle8.h"
#include "rleX_extreme_common.h"
#include "bitpack.h"

//////////////////////////////////////////////////////////////////////////

//#define USE_COPY_LOW_COUNT_SPECIAL_CASE

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_extreme_mmtf128_compress_bounds(const uint32_t inSize)
{
  if (inSize > (1 << 30))
    return 0;

  return inSize + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + 1;
}

uint32_t rle8_extreme_mmtf256_compress_bounds(const uint32_t inSize)
{
  return rle8_extreme_mmtf128_compress_bounds(inSize);
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_extreme_mmtf128_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_extreme_mmtf128_compress_bounds(inSize))
    return 0;

  uint32_t *pHeader = (uint32_t *)pOut;
  pHeader[0] = inSize;
  // pHeader[1] will be the compressed size.

  pOut += sizeof(uint32_t) * 2;

  // Encode & Compress.
  {
    ALIGN(16) uint8_t history[sizeof(__m128i) * 256];

    for (size_t i = 0; i < 256; i++)
      _mm_store_si128((__m128i *)(history) + i, _mm_set1_epi8((char)i));

    uint32_t inSizeRemaining = inSize;
    const __m128i one = _mm_set1_epi8(1);

    uint8_t symbol = 0;
    __m128i symbol128 = _mm_set1_epi8(symbol);
    int64_t count = 0;
    bool copying = 1;
    const __m128i hi4BitMask = _mm_set1_epi8(0xF0);
    //const __m128i hi5BitMask = _mm_set1_epi8((uint8_t)0b11111000);
    //const __m128i hi6BitMask = _mm_set1_epi8((uint8_t)0b11111100);
    __m128i currentBitMask = _mm_undefined_si128();

    uint8_t *pLastHeader = pOut;
    pOut += sizeof(uint32_t);

    __m128i out;

    while (inSizeRemaining >= sizeof(__m128i))
    {
      inSizeRemaining -= sizeof(__m128i);

      const __m128i symbols = _mm_loadu_si128((__m128i *)pIn);
      pIn += sizeof(__m128i);

      out = _mm_setzero_si128();
      __m128i index = _mm_setzero_si128();
      __m128i currentHist = _mm_load_si128((__m128i *)history);
      __m128i matched = _mm_cmpeq_epi8(symbols, currentHist);
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

      if (copying)
      {
        // Should we stop copying?
        const uint8_t potentialSymbol = (uint8_t)_mm_extract_epi16(out, 0);
        const __m128i potentialSymbol128 = _mm_set1_epi8(potentialSymbol);
        const int32_t cmp = _mm_movemask_epi8(_mm_cmpeq_epi8(out, potentialSymbol128));

        if (cmp == 0xFFFF)
        {
          copying = false;
          symbol = potentialSymbol;
          symbol128 = potentialSymbol128;

          uint8_t *pStart = pLastHeader + sizeof(uint32_t);

#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
          if (count < 64)
          {
            *pLastHeader = (uint8_t)count << 2;
            pOut -= 3;

            // Only low bits used.
            if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hiMask, currentBitMask))))
            {
              *pLastHeader |= 2;

              pOut = bitpack_encode4_sse2_unaligned_m128i((const __m128i *)pStart, pStart - 3, count * sizeof(__m128i));
            }
            else
            {
              // Memmove.
              {
                const __m128i *pSrc = (const __m128i *)(pLastHeader + 3);
                __m128i *pDst = (__m128i *)(pLastHeader + 1);
                size_t remainingCount = count;

                while (remainingCount)
                {
                  _mm_storeu_si128(pDst, _mm_loadu_si128(pSrc));
                  pSrc++;
                  pDst++;
                  remainingCount--;
                }
              }
            }
          }
          else
          {
            *(uint32_t *)pLastHeader = (uint32_t)count << 2 | 1;
#else
          {
            *(uint32_t *)pLastHeader = (uint32_t)count << 1;
#endif

            // Only low bits used.
            if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi4BitMask, currentBitMask))))
            {
              //if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi6BitMask, currentBitMask))))
              //else if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi5BitMask, currentBitMask))))
              //else

#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
              *pLastHeader |= 2;
#else
              *pLastHeader |= 1;
#endif

              pOut = bitpack_encode4_sse2_unaligned_m128i((const __m128i *)pStart, pStart, count * sizeof(__m128i));
            }
          }

          pLastHeader = pOut;
          count = 1;
        }
        else
        {
          // This could be a lot smarter, if it detected that it was worth a copy of a bunch of blocks, then a null rle, then start a new block with 0xF0 bits used.
          currentBitMask = _mm_or_si128(currentBitMask, out);
          count++;

          _mm_storeu_si128((__m128i *)pOut, out);
          pOut += sizeof(__m128i);
        }
      }
      else
      {
        const uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(symbol128, out));

        if (mask == 0xFFFF)
        {
          count++;
        }
        else
        {
          // Store Repeat Header.
          if (count < 128)
          {
            pLastHeader[0] = (uint8_t)count << 1;
            pLastHeader[1] = symbol;
            
            pOut = pLastHeader + 2;
          }
          else
          {
            *(uint32_t *)pLastHeader = (uint32_t)count << 1 | 1;
            pLastHeader[sizeof(uint32_t)] = symbol;

            pOut = pLastHeader + sizeof(uint32_t) + 1;
          }

          count = 1;
          pLastHeader = pOut;

          pOut += sizeof(uint32_t);
          _mm_storeu_si128((__m128i *)pOut, out);
          pOut += sizeof(__m128i);
          currentBitMask = out;
          copying = true;
        }
      }
    }

    if (copying)
    {
      // Write Copy Header.
      {
        uint8_t *pStart = pLastHeader + sizeof(uint32_t);

#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
        if (count < 64)
        {
          *pLastHeader = (uint8_t)count << 2;
          pOut -= 3;

          // Only low bits used.
          if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hiMask, currentBitMask))))
          {
            *pLastHeader |= 2;

            size_t remaining = count;
            __m128i *pPacked = (__m128i *)(pStart - 3);

            while (remaining > 1)
            {
              const __m128i hi = _mm_loadu_si128((const __m128i *)pStart);
              const __m128i lo = _mm_loadu_si128((const __m128i *)(pStart + sizeof(__m128i)));

              const __m128i pack = _mm_or_si128(_mm_slli_epi16(hi, 4), lo);

              _mm_storeu_si128(pPacked, pack);

              pPacked++;
              pStart += sizeof(__m128i) * 2;
              remaining -= 2;
            }

            if (remaining)
            {
              _mm_storeu_si128(pPacked, _mm_loadu_si128((const __m128i *)pStart));
              pPacked++;
            }

            pOut = (uint8_t *)pPacked;
          }
          else
          {
            // Memmove.
            {
              const __m128i *pSrc = (const __m128i *)(pLastHeader + 3);
              __m128i *pDst = (__m128i *)(pLastHeader + 1);
              size_t remainingCount = count;

              while (remainingCount)
              {
                _mm_storeu_si128(pDst, _mm_loadu_si128(pSrc));
                pSrc++;
                pDst++;
                remainingCount--;
              }
            }
          }
            }
        else
        {
          *(uint32_t *)pLastHeader = (uint32_t)count << 2 | 1;
#else
        {
          *(uint32_t *)pLastHeader = (uint32_t)count << 1;
#endif

          // Only low bits used.
          if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi4BitMask, currentBitMask))))
          {
            //if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi6BitMask, currentBitMask))))
            //else if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi5BitMask, currentBitMask))))
            //else

#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
            *pLastHeader |= 2;
#else
            *pLastHeader |= 1;
#endif

            size_t remaining = count;
            __m128i *pPacked = (__m128i *)pStart;

            while (remaining > 1)
            {
              const __m128i hi = _mm_loadu_si128((const __m128i *)pStart);
              const __m128i lo = _mm_loadu_si128((const __m128i *)(pStart + sizeof(__m128i)));

              const __m128i pack = _mm_or_si128(_mm_slli_epi16(hi, 4), lo);

              _mm_storeu_si128(pPacked, pack);

              pPacked++;
              pStart += sizeof(__m128i) * 2;
              remaining -= 2;
            }

            if (remaining)
            {
              _mm_storeu_si128(pPacked, _mm_loadu_si128((const __m128i *)pStart));
              pPacked++;
            }

            pOut = (uint8_t *)pPacked;
          }
        }

        *pOut = 0; // write 0 repeat header. count 0 means stop. (maybe this should rather be copy 0xFFFFFFFF)
        pOut++;
      }
    }
    else
    {
      // Store Repeat Header.
      {
        if (count < 128)
        {
          uint8_t *pHdr = pLastHeader;
          pHdr[0] = (uint8_t)count << 1;
          pHdr[1] = symbol;

          pOut = pHdr + 2;
        }
        else
        {
          *(uint32_t *)pLastHeader = (uint32_t)count << 1 | 1;
          uint8_t *pHdr = pLastHeader;
          pHdr[sizeof(uint32_t)] = symbol;

          pOut = pHdr + sizeof(uint32_t) + 1;
        }
      }
    }

    // Store Remaining Bytes.
    for (size_t i = 0; i < inSizeRemaining; i++)
    {
      const uint8_t sym = *pIn;
      pIn++;

      for (size_t d = 0; d < 255; d++)
      {
        if (history[d * 16 + i] == sym)
        {
          *pOut = (uint8_t)d;
          pOut++;
          break;
        }
      }
    }
  }

  return pHeader[1] = (uint32_t)(pOut - (uint8_t *)pHeader);
}

uint32_t rle8_extreme_mmtf128_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  (void)pIn;
  (void)inSize;
  (void)pOut;
  (void)outSize;

  return 0;
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_extreme_mmtf256_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle8_extreme_mmtf256_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
