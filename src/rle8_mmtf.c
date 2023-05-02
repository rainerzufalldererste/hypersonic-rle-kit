#include "rle8.h"
#include "rleX_extreme_common.h"
#include "bitpack.h"

//#define COMMENT(...) printf(__VA_ARGS__)
#define COMMENT(...)

//////////////////////////////////////////////////////////////////////////

#define USE_COPY_LOW_COUNT_SPECIAL_CASE
#define USE_VARIOUS_COPY_SIZES
#define USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE

#ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
  #ifndef USE_VARIOUS_COPY_SIZES
    #define USE_VARIOUS_COPY_SIZES
  #endif
#endif

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_mmtf128_compress_bounds(const uint32_t inSize)
{
  if (inSize > (1 << 30))
    return 0;

  return inSize + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + 1;
}

uint32_t rle8_mmtf256_compress_bounds(const uint32_t inSize)
{
  return rle8_mmtf128_compress_bounds(inSize);
}

//////////////////////////////////////////////////////////////////////////

// simplified version of `bitpack_encode2_sse2_unaligned_m128i`, to reduce decoder code footprint.
// `originalSize` must be a multiple of `sizeof(__m128i)`.
// `pOut` should point to a block of memory with a minimum size of `originalSize / 4`.
// returns `pOut` one byte after the last one that was written to.
inline static uint8_t *bitpack_encode2_simple_sse2_unaligned_m128i(const __m128i *pIn, uint8_t *pOut, const size_t originalSize)
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

  const uint32_t *pIn32 = (const uint32_t *)pIn;

  for (; i < (int64_t)originalSize; i += sizeof(uint32_t) * 4)
  {
    const uint32_t p0 = pIn32[0];
    const uint32_t p1 = pIn32[1];
    const uint32_t p2 = pIn32[2];
    const uint32_t p3 = pIn32[3];

    const uint32_t combined = p0 | (p1 << 2) | (p2 << 4) | (p3 << 6);

    *(uint32_t *)pOut = combined;
    pOut += sizeof(uint32_t);
    pIn32 += 4;
  }

  return pOut;
}

// simplified version of `bitpack_encode3_sse2_unaligned_m128i`, to reduce decoder code footprint.
// `originalSize` must be a multiple of __m128i.
// `pOut` must contain at least `originalSize * 3 / 8` bytes.
// returns `pOut` one byte after the last one that was written to.
inline static uint8_t *bitpack_encode3_simple_sse2_unaligned_m128i(const __m128i *pIn, uint8_t *pOut, const size_t originalSize)
{
  const __m128i patternLow2 = _mm_set1_epi8(3);

  int64_t i = 0;
  int64_t originalSize6 = (int64_t)originalSize - sizeof(__m128i) * 6;

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

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_mmtf128_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_mmtf128_compress_bounds(inSize))
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
    const __m128i hi5BitMask = _mm_set1_epi8((uint8_t)0b11111000);
    const __m128i hi6BitMask = _mm_set1_epi8((uint8_t)0b11111100);

    __m128i currentBitMask = _mm_undefined_si128();
    //__m128i lastOut = _mm_xor_si128(_mm_set1_epi8((int8_t)-1), _mm_loadu_si128((const __m128i *)pIn));

    uint8_t *pLastHeader = pOut;
    pOut += sizeof(uint32_t);

    __m128i out;// = lastOut;

    while (inSizeRemaining >= sizeof(__m128i))
    {
      inSizeRemaining -= sizeof(__m128i);

      const __m128i symbols = _mm_loadu_si128((const __m128i *)pIn);
      pIn += sizeof(__m128i);

      //lastOut = out;
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

        if (/*_mm_movemask_epi8(_mm_cmpeq_epi8(out, lastOut)) == 0xFFFF && */cmp == 0xFFFF)
        {
          COMMENT("COPY %" PRIu64 " blocks.", count);
          copying = false;
          symbol = potentialSymbol;
          symbol128 = potentialSymbol128;

          uint8_t *pStart = pLastHeader + sizeof(uint32_t);

#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
  #ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
          if (count < 32)
  #else
          if (count < 64)
  #endif
          {
  #ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
            *pLastHeader = (uint8_t)count << 3;
  #else
            *pLastHeader = (uint8_t)count << 2;
  #endif
            pOut -= 3;

            // Only low bits used.
            if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi4BitMask, currentBitMask))))
            {
  #ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
              if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi6BitMask, currentBitMask))))
              {
                *pLastHeader |= 0b110;

                pOut = bitpack_encode2_simple_sse2_unaligned_m128i((const __m128i *)pStart, pStart - 3, count * sizeof(__m128i));

                COMMENT(" (2 BIT) (SMALL)");
              }
              else if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi5BitMask, currentBitMask))))
              {
                *pLastHeader |= 0b100;

                pOut = bitpack_encode3_simple_sse2_unaligned_m128i((const __m128i *)pStart, pStart - 3, count * sizeof(__m128i));

                COMMENT(" (3 BIT) (SMALL)");
              }
              else
  #endif
              {
  #ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
                *pLastHeader |= 0b010;
  #else
                *pLastHeader |= 0b10;
  #endif

                pOut = bitpack_encode4_sse2_unaligned_m128i((const __m128i *)pStart, pStart - 3, count * sizeof(__m128i));

                COMMENT(" (4 BIT) (SMALL)");
              }
            }
            else
            {
              // Memmove.
              {
                const __m128i *pSrc = (const __m128i *)(pLastHeader + 4);
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

              COMMENT(" (SMALL)");
            }
          }
          else
          {
  #ifdef USE_VARIOUS_COPY_SIZES
            *(uint32_t *)pLastHeader = (uint32_t)count << 3 | 1;
  #else
            *(uint32_t *)pLastHeader = (uint32_t)count << 2 | 1;
  #endif
#else
          {
  #ifdef USE_VARIOUS_COPY_SIZES
            *(uint32_t *)pLastHeader = (uint32_t)count << 2;
  #else
            *(uint32_t *)pLastHeader = (uint32_t)count << 1;
  #endif
#endif

            // Only low bits used.
            if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi4BitMask, currentBitMask))))
            {
#ifdef USE_VARIOUS_COPY_SIZES
              if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi6BitMask, currentBitMask))))
              {
#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
                *pLastHeader |= 0b110;
#else
                *pLastHeader |= 0b11;
#endif

                pOut = bitpack_encode2_simple_sse2_unaligned_m128i((const __m128i *)pStart, pStart, count * sizeof(__m128i));

                COMMENT(" (2 BIT) (LARGE)");
              }
              else if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi5BitMask, currentBitMask))))
              {
#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
                *pLastHeader |= 0b100;
#else
                *pLastHeader |= 0b10;
#endif

                pOut = bitpack_encode3_simple_sse2_unaligned_m128i((const __m128i *)pStart, pStart, count * sizeof(__m128i));

                COMMENT(" (3 BIT) (LARGE)");
              }
              else
#endif
              {
#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
                *pLastHeader |= 0b010;
#else
                *pLastHeader |= 0b01;
#endif

                pOut = bitpack_encode4_sse2_unaligned_m128i((const __m128i *)pStart, pStart, count * sizeof(__m128i));

                COMMENT(" (4 BIT) (LARGE)");
              }
            }
          }

          pLastHeader = pOut;
          count = 1;

          COMMENT("\n");
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
          COMMENT("RLE %" PRIu64 " blocks of 0x%02" PRIX8 "\n", count, symbol);

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

#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
          const uint8_t potentialSymbol = (uint8_t)_mm_extract_epi16(out, 0);
          const __m128i potentialSymbol128 = _mm_set1_epi8(potentialSymbol);
          const int32_t cmp = _mm_movemask_epi8(_mm_cmpeq_epi8(out, potentialSymbol128));

          if (cmp == 0xFFFF)
          {
            // Store 0 block copy.
            *pOut = 0;
            pOut++;

            COMMENT("[NULL COPY]\n");

            pLastHeader = pOut;
            count = 1;

            symbol = potentialSymbol;
            symbol128 = potentialSymbol128;
          }
          else
#endif
          {
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
    }

    if (copying)
    {
      COMMENT("[LAST] COPY %" PRIu64 " blocks", count);

      // Write Copy Header.
      {
        uint8_t *pStart = pLastHeader + sizeof(uint32_t);

#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
#ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
        if (count < 32)
#else
        if (count < 64)
#endif
        {
#ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
          *pLastHeader = (uint8_t)count << 3;
#else
          *pLastHeader = (uint8_t)count << 2;
#endif
          pOut -= 3;

          // Only low bits used.
          if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi4BitMask, currentBitMask))))
          {
#ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
            if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi6BitMask, currentBitMask))))
            {
              *pLastHeader |= 0b110;

              pOut = bitpack_encode2_simple_sse2_unaligned_m128i((const __m128i *)pStart, pStart - 3, count * sizeof(__m128i));

              COMMENT(" (2 BIT) (SMALL)");
            }
            else if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi5BitMask, currentBitMask))))
            {
              *pLastHeader |= 0b100;

              pOut = bitpack_encode3_simple_sse2_unaligned_m128i((const __m128i *)pStart, pStart - 3, count * sizeof(__m128i));

              COMMENT(" (3 BIT) (SMALL)");
            }
            else
#endif
            {
#ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
              *pLastHeader |= 0b010;
#else
              *pLastHeader |= 0b10;
#endif

              pOut = bitpack_encode4_sse2_unaligned_m128i((const __m128i *)pStart, pStart - 3, count * sizeof(__m128i));

              COMMENT(" (4 BIT) (SMALL)");
            }
          }
          else
          {
            // Memmove.
            {
              const __m128i *pSrc = (const __m128i *)(pLastHeader + 4);
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

            COMMENT(" (SMALL)");
          }
        }
        else
        {
#ifdef USE_VARIOUS_COPY_SIZES
          *(uint32_t *)pLastHeader = (uint32_t)count << 3 | 1;
#else
          *(uint32_t *)pLastHeader = (uint32_t)count << 2 | 1;
#endif
#else
          {
#ifdef USE_VARIOUS_COPY_SIZES
            *(uint32_t *)pLastHeader = (uint32_t)count << 2;
#else
            *(uint32_t *)pLastHeader = (uint32_t)count << 1;
#endif
#endif

            // Only low bits used.
            if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi4BitMask, currentBitMask))))
            {
#ifdef USE_VARIOUS_COPY_SIZES
              if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi6BitMask, currentBitMask))))
              {
#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
                *pLastHeader |= 0b110;
#else
                *pLastHeader |= 0b11;
#endif

                pOut = bitpack_encode2_simple_sse2_unaligned_m128i((const __m128i *)pStart, pStart, count * sizeof(__m128i));

                COMMENT(" (2 BIT) (LARGE)");
              }
              else if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_setzero_si128(), _mm_and_si128(hi5BitMask, currentBitMask))))
              {
#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
                *pLastHeader |= 0b100;
#else
                *pLastHeader |= 0b10;
#endif

                pOut = bitpack_encode3_simple_sse2_unaligned_m128i((const __m128i *)pStart, pStart, count * sizeof(__m128i));

                COMMENT(" (3 BIT) (LARGE)");
              }
              else
#endif
              {
#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
                *pLastHeader |= 0b010;
#else
                *pLastHeader |= 0b01;
#endif

                pOut = bitpack_encode4_sse2_unaligned_m128i((const __m128i *)pStart, pStart, count * sizeof(__m128i));

                COMMENT(" (4 BIT) (LARGE)");
              }
            }
        }

        pLastHeader = pOut;
        count = 1;

        COMMENT("\n");

        // null rle.
        COMMENT("[NULL RLE]\n");
        *pOut = 0; // (maybe this should rather be copy 0xFFFFFFFF)
        pOut++;
      }
    }
    else
    {
      COMMENT("[LAST] RLE %" PRIu64 " blocks of 0x%02" PRIX8 "\n", count, symbol);

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

      // null copy, then null rle.
      COMMENT("[NULL COPY]\n");
      *pOut = 0;
      pOut++;
      COMMENT("[NULL RLE]\n");
      *pOut = 0;
      pOut++;
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

uint32_t rle8_mmtf128_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedOutSize = ((uint32_t *)pIn)[0];
  const size_t expectedInSize = ((uint32_t *)pIn)[1];

  uint8_t *pOutStart = pOut;
  
  pIn += sizeof(uint32_t) * 2;

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  ALIGN(16) uint8_t history[sizeof(__m128i) * 256];

  for (size_t i = 0; i < 256; i++)
    _mm_store_si128((__m128i *)(history) + i, _mm_set1_epi8((char)i));

  while (true)
  {
    // Copy.
    {
      uint32_t inSizeRemaining;

#ifdef USE_VARIOUS_COPY_SIZES
      const uint8_t largeCountShift = 3;
#else
      const uint8_t largeCountShift = 2;
#endif

#ifdef USE_COPY_LOW_COUNT_SPECIAL_CASE
      uint8_t inSizeLastByte = *pIn;

  #ifdef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
      const uint8_t smallCountShift = 3;
  #else
      const uint8_t smallCountShift = 2;
  #endif

      if (inSizeLastByte & 1)
      {
        inSizeRemaining = *(uint32_t *)pIn >> largeCountShift;
        pIn += sizeof(uint32_t);
      }
      else
      {
        inSizeRemaining = inSizeLastByte >> smallCountShift;
        pIn += sizeof(uint8_t);
      }
#else
      inSizeRemaining = *(uint32_t *)pIn >> largeCountShift;
      pIn += sizeof(uint32_t);
#endif

#ifndef USE_VARIOUS_COPY_SIZES
#error NOT IMPLEMENTED
#endif

#ifndef USE_COPY_LOW_COUNT_SPECIAL_CASE
#error NOT IMPLEMENTED
#endif

#ifndef USE_VARIOUS_COPY_SIZES_WITH_LOW_COUNT_SPECIAL_CASE
#error NOT IMPLEMENTED
#endif

      COMMENT("@ % 8" PRIu64 ": COPY %" PRIu32 " blocks", pOut - pOutStart, inSizeRemaining);

      if ((inSizeLastByte & 0b110) == 0)
      {
        const __m128i one = _mm_set1_epi8(1);

        for (; inSizeRemaining; inSizeRemaining--)
        {
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

          while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
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
      }
      else if ((inSizeLastByte & 0b110) == 0b110) // unpack 2 bit.
      {
        COMMENT(" (2 BIT)");

        const __m128i one = _mm_set1_epi8(1);
        const __m128i lo_pattern = _mm_set1_epi8(3);
        ALIGN(16) __m128i decoded[4];

        for (; inSizeRemaining >= 4; inSizeRemaining -= 4)
        {
          // Unpack.
          {
            const __m128i pack = _mm_loadu_si128((__m128i *)pIn);
            pIn += sizeof(__m128i);

            const __m128i unpack0 = _mm_and_si128(lo_pattern, pack);
            const __m128i unpack1 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 2));
            const __m128i unpack2 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
            const __m128i unpack3 = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 6));

            _mm_store_si128(decoded, unpack0);
            _mm_store_si128(decoded + 1, unpack1);
            _mm_store_si128(decoded + 2, unpack2);
            _mm_store_si128(decoded + 3, unpack3);
          }

          // Process & Store.
          for (size_t i = 0; i < 4; i++)
          {
            const __m128i indices = _mm_load_si128(&decoded[i]);

            __m128i index = _mm_setzero_si128();
            __m128i matched = _mm_cmpeq_epi8(indices, index);
            __m128i currentHist = _mm_load_si128((__m128i *)history);
            __m128i out = _mm_and_si128(currentHist, matched);
            __m128i prevMatched = matched;
            __m128i history0 = out;
            __m128i lastHist;
            __m128i *pHistory = ((__m128i *)history) + 1;

            while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
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
        }

        for (; inSizeRemaining; inSizeRemaining--)
        {
          // Unpack.
          const uint32_t combined = *(uint32_t *)pIn;
          pIn += sizeof(uint32_t);

          const uint32_t lo_pattern_32 = 0x03030303;
          const __m128i indices = _mm_and_si128(_mm_set1_epi32(lo_pattern_32), _mm_set_epi32(combined >> 6, combined >> 4, combined >> 2, combined));

          // Process & Store.
          {
            __m128i index = _mm_setzero_si128();
            __m128i matched = _mm_cmpeq_epi8(indices, index);
            __m128i currentHist = _mm_load_si128((__m128i *)history);
            __m128i out = _mm_and_si128(currentHist, matched);
            __m128i prevMatched = matched;
            __m128i history0 = out;
            __m128i lastHist;
            __m128i *pHistory = ((__m128i *)history) + 1;

            while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
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
        }
      }
      else if ((inSizeLastByte & 0b110) == 0b100) // unpack 3 bit.
      {
        COMMENT(" (3 BIT)");

        const __m128i one = _mm_set1_epi8(1);
        const __m128i patternLow1 = _mm_set1_epi8(1);
        const __m128i patternLow2 = _mm_set1_epi8(3);
        const __m128i patternLow3 = _mm_set1_epi8(7);
        const __m128i patternBitSelect = _mm_set1_epi64x((int64_t)0x8040201008040201);
        ALIGN(16) __m128i decoded[6];

        for (; inSizeRemaining >= 6; inSizeRemaining -= 6)
        {
          // Unpack.
          {
            const __m128i combinedA = _mm_loadu_si128((const __m128i *)(pIn));
            const __m128i combinedB = _mm_loadu_si128((const __m128i *)(pIn + sizeof(__m128i)));

            const __m128i d0 = _mm_and_si128(patternLow3, combinedA);
            const __m128i d1 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedA, 3));
            const __m128i d2lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedA, 6));
            const __m128i d3 = _mm_and_si128(patternLow3, combinedB);
            const __m128i d4 = _mm_and_si128(patternLow3, _mm_srli_epi16(combinedB, 3));
            const __m128i d5lo = _mm_and_si128(patternLow2, _mm_srli_epi16(combinedB, 6));

            _mm_store_si128(decoded, d0);
            _mm_store_si128(decoded + 1, d1);

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

            _mm_store_si128(decoded + 2, d2);
            _mm_store_si128(decoded + 3, d3);
            _mm_store_si128(decoded + 4, d4);
            _mm_store_si128(decoded + 5, d5);
          }

          // Process & Store.
          for (size_t i = 0; i < 6; i++)
          {
            const __m128i indices = _mm_load_si128(&decoded[i]);

            __m128i index = _mm_setzero_si128();
            __m128i matched = _mm_cmpeq_epi8(indices, index);
            __m128i currentHist = _mm_load_si128((__m128i *)history);
            __m128i out = _mm_and_si128(currentHist, matched);
            __m128i prevMatched = matched;
            __m128i history0 = out;
            __m128i lastHist;
            __m128i *pHistory = ((__m128i *)history) + 1;

            while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
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
        }

        for (; inSizeRemaining; inSizeRemaining--)
        {
          // Unpack.
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

          const __m128i indices = _mm_or_si128(_mm_or_si128(v1, v2), v0);

          // Process & Store.
          {
            __m128i index = _mm_setzero_si128();
            __m128i matched = _mm_cmpeq_epi8(indices, index);
            __m128i currentHist = _mm_load_si128((__m128i *)history);
            __m128i out = _mm_and_si128(currentHist, matched);
            __m128i prevMatched = matched;
            __m128i history0 = out;
            __m128i lastHist;
            __m128i *pHistory = ((__m128i *)history) + 1;

            while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
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
        }
      }
      else // if ((inSizeLastByte & 0b110) == 0b010) // unpack 4 bit.
      {
        COMMENT(" (4 BIT)");

        const __m128i one = _mm_set1_epi8(1);
        const __m128i lo_pattern = _mm_set1_epi8(0x0F);
        ALIGN(16) __m128i decoded[2];

        for (; inSizeRemaining >= 2; inSizeRemaining -= 2)
        {
          // Unpack.
          {
            const __m128i pack = _mm_loadu_si128((__m128i *)pIn);
            pIn += sizeof(__m128i);

            const __m128i unpack_hi = _mm_and_si128(lo_pattern, _mm_srli_epi16(pack, 4));
            const __m128i unpack_lo = _mm_and_si128(lo_pattern, pack);

            _mm_store_si128(decoded, unpack_hi);
            _mm_store_si128(decoded + 1, unpack_lo);
          }

          // Process & Store.
          for (size_t i = 0; i < 2; i++)
          {
            const __m128i indices = _mm_load_si128(&decoded[i]);

            __m128i index = _mm_setzero_si128();
            __m128i matched = _mm_cmpeq_epi8(indices, index);
            __m128i currentHist = _mm_load_si128((__m128i *)history);
            __m128i out = _mm_and_si128(currentHist, matched);
            __m128i prevMatched = matched;
            __m128i history0 = out;
            __m128i lastHist;
            __m128i *pHistory = ((__m128i *)history) + 1;

            while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
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
        }

        if (inSizeRemaining) // this can be a maximum of 1
        {
          // Unpack.
          const uint64_t in = *(const uint64_t *)pIn;
          pIn += sizeof(uint64_t);

          const uint64_t fourBitPattern = 0x0F0F0F0F0F0F0F0F;
          const __m128i indices = _mm_and_si128(_mm_set1_epi64x(fourBitPattern), _mm_set_epi64x(in >> 4, in));

          // Process & Store.
          {
            __m128i index = _mm_setzero_si128();
            __m128i matched = _mm_cmpeq_epi8(indices, index);
            __m128i currentHist = _mm_load_si128((__m128i *)history);
            __m128i out = _mm_and_si128(currentHist, matched);
            __m128i prevMatched = matched;
            __m128i history0 = out;
            __m128i lastHist;
            __m128i *pHistory = ((__m128i *)history) + 1;

            while (0xFFFF != _mm_movemask_epi8(prevMatched)) // We're gonna find that byte eventually.
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
        }
      }

      COMMENT("\n");
    }

    // RLE.
    {
      uint8_t lastByte = *pIn;
      uint32_t count;

      if (lastByte & 1)
      {
        count = *(uint32_t *)pIn >> 1;
        pIn += sizeof(uint32_t);
      }
      else
      {
        count = lastByte >> 1;
        pIn += sizeof(uint8_t);
      }

      if (count == 0)
        break;

      const uint8_t symbol = *pIn;
      pIn += sizeof(uint8_t);

      COMMENT("@ % 8" PRIu64 ": RLE %" PRIu32 " blocks of 0x%02" PRIX8 "\n", pOut - pOutStart, count, symbol);

      if (symbol == 0)
      {
        const __m128i sym = _mm_load_si128((const __m128i *)history);

        for (; count; count--)
        {
          _mm_storeu_si128((__m128i *)pOut, sym);
          pOut += sizeof(__m128i);
        }
      }
      else if (symbol == 1)
      {
        const __m128i sym0 = _mm_load_si128((const __m128i *)history);
        const __m128i sym1 = _mm_load_si128((const __m128i *)(history + sizeof(__m128i)));

        for (; count >= 2; count -= 2)
        {
          _mm_storeu_si128((__m128i *)pOut, sym1);
          _mm_storeu_si128((__m128i *)(pOut + sizeof(__m128i)), sym0);
          pOut += sizeof(__m128i) * 2;
        }

        if (count)
        {
          _mm_storeu_si128((__m128i *)pOut, sym1);
          pOut += sizeof(__m128i);

          _mm_store_si128((__m128i *)history, sym1);
          _mm_store_si128((__m128i *)(history + sizeof(__m128i)), sym0);
        }
      }
      else if (symbol == 2)
      {
        const __m128i sym0 = _mm_load_si128((const __m128i *)history);
        const __m128i sym1 = _mm_load_si128((const __m128i *)(history + sizeof(__m128i)));
        const __m128i sym2 = _mm_load_si128((const __m128i *)(history + sizeof(__m128i) * 2));

        for (; count >= 3; count -= 3)
        {
          _mm_storeu_si128((__m128i *)pOut, sym2);
          _mm_storeu_si128((__m128i *)(pOut + sizeof(__m128i)), sym1);
          _mm_storeu_si128((__m128i *)(pOut + sizeof(__m128i) * 2), sym0);
          pOut += sizeof(__m128i) * 3;
        }

        if (count == 2)
        {
          _mm_storeu_si128((__m128i *)pOut, sym2);
          _mm_storeu_si128((__m128i *)(pOut + sizeof(__m128i)), sym1);
          pOut += sizeof(__m128i) * 2;

          _mm_store_si128((__m128i *)history, sym1);
          _mm_store_si128((__m128i *)(history + sizeof(__m128i)), sym2);
          _mm_store_si128((__m128i *)(history + sizeof(__m128i) * 2), sym0);
        }
        else if (count == 1)
        {
          _mm_storeu_si128((__m128i *)pOut, sym2);
          pOut += sizeof(__m128i);

          _mm_store_si128((__m128i *)history, sym2);
          _mm_store_si128((__m128i *)(history + sizeof(__m128i)), sym0);
          _mm_store_si128((__m128i *)(history + sizeof(__m128i) * 2), sym1);
        }
      }
      else
      {
        uint32_t symPlusOne = (uint32_t)symbol + 1;

        for (; count >= symPlusOne; count -= symPlusOne)
        {
          for (uint_fast32_t i = 0; i <= symbol; i++)
          {
            const __m128i sym = _mm_load_si128((const __m128i *)history + symbol - i);
            _mm_storeu_si128(((__m128i *)pOut) + i, sym);
          }
        
          pOut += sizeof(__m128i) * symPlusOne;
        }
        
        for (; count; count--)
        {
          const __m128i sym = _mm_load_si128((const __m128i *)history + symbol);
          _mm_storeu_si128((__m128i *)pOut, sym);
          pOut += sizeof(__m128i);

          // Memmove.
          {
            __m128i *pDst = (__m128i *)(history + symbol * sizeof(__m128i));

            for (size_t i = symbol; i > 0; i--)
            {
              __m128i *pSrc = pDst - 1;
              _mm_store_si128(pDst, _mm_load_si128(pSrc));
              pDst = pSrc;
            }
          }

          _mm_store_si128((__m128i *)history, sym);
        }
      }
    }
  }

  // Copy Remaining Bytes.
  const size_t outSizeRemaining = (pOutStart + expectedOutSize) - pOut;

  for (size_t i = 0; i < outSizeRemaining; i++)
  {
    const uint8_t index = *pIn;
    pIn++;

    *pOut = history[(size_t)index * 16 + i];
    pOut++;
  }

  return (uint32_t)expectedOutSize;
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_mmtf256_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle8_mmtf256_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
