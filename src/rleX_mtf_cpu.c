#include "rle8.h"
#include "rleX_extreme_common.h"

#define TYPE_SIZE 64

#define CONCAT_LITERALS3(a, b, c) a ## b ## c
#define CONCAT3(a, b, c) CONCAT_LITERALS3(a, b, c)

#define CONCAT_LITERALS2(a, b) a ## b
#define CONCAT2(a, b) CONCAT_LITERALS2(a, b)

#define RLEX_MTF_MIN_SIZE ((TYPE_SIZE) / 8 + 1 + 1 + 1)
#define RLEX_MTF_MAX_SIZE ((TYPE_SIZE) / 8 + 1 + 1 + 4 + 4)

uint32_t rleX_mtf_compress_bounds(const uint32_t inSize)
{
  return inSize + RLEX_MTF_MAX_SIZE * 2;
}

uint32_t rleX_mtf_decompress_additional_size()
{
  return 128; // just to be on the safe side.
}

__declspec(noinline)
uint32_t mtf_encode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || outSize < inSize)
    return 0;

  uint8_t *pOutStart = pOut;
  
  ALIGN(16)
  struct
  {
    ALIGN(16) uint8_t padding[16];
    ALIGN(16) uint8_t symbols[256 + 16];
  } _;

  for (size_t i = 0; i < 256; i++)
    _.symbols[i] = i;

  for (size_t i = 0; i < inSize; i++)
  {
    const uint8_t s = pIn[i];
#if FRONT_MTF_SIMD
    const __m128i sx = _mm_set1_epi8(s);
    
    for (size_t j = 0; j < 256; j += 16)
    {
      const __m128i sb = _mm_load_si128(_.symbols + j);
      const uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(sb, sx));

      if (mask != 0)
      {
#ifdef _MSC_VER
        unsigned long symbolIndex;
        _BitScanForward64(&symbolIndex, mask);
#else
        uint64_t symbolIndex = __builtin_ctzl(mask);
#endif

        symbolIndex += j;
#else
    uint8_t symbolIndex = 0;

    for (; symbolIndex != 255; symbolIndex++)
      if (_.symbols[symbolIndex] == s)
        break;
#endif

        *pOut = symbolIndex;
        pOut++;

        if (symbolIndex != 0)
        {
#if BACK_MTF_SIMD
          int64_t symbolIndexI = (int64_t)symbolIndex;

          do
          {
            _mm_storeu_si128(_.symbols + symbolIndexI - 15, _mm_loadu_si128(_.symbols + symbolIndexI - 16));
            symbolIndexI -= 16;
          }
          while (symbolIndexI > 0);
#else
          for (size_t l = symbolIndex; l >= 1; l--)
            _.symbols[l] = _.symbols[l - 1];
#endif

          _.symbols[0] = s;
        }

#if FRONT_MTF_SIMD
        break;
#endif
      }
#if FRONT_MTF_SIMD
    }
  }
#endif

  return inSize;
}

__declspec(noinline)
uint32_t mtf_decode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || outSize < inSize)
    return 0;

  uint8_t *pOutStart = pOut;

  ALIGN(16) uint8_t symbols[256 + 15];

  for (size_t i = 0; i < 256; i++)
    symbols[i] = i;

  for (size_t i = 0; i < inSize; i++)
  {
    const uint8_t s = pIn[i];
    const uint8_t d = symbols[s];
    *pOut = d;
    pOut++;

    if (s != 0)
    {
      for (size_t l = s; l >= 1; l--)
        symbols[l] = symbols[l - 1];

      symbols[0] = d;
    }
  }

  return inSize;
}

#undef TYPE_SIZE

//////////////////////////////////////////////////////////////////////////

#define TYPE_SIZE 8

uint32_t rle8_mtf_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || outSize < rleX_mtf_compress_bounds(inSize))
    return 0;

  uint8_t *pOutStart = pOut;

  ALIGN(16) uint8_t symbols[256 + 15];

  for (size_t i = 0; i < 256; i++)
    symbols[i] = i;

  typedef CONCAT3(uint8_t) symbol_t;

  int64_t end = inSize - sizeof(symbol_t);
  int64_t i = 0;

  ALIGN(16) uint8_t uncompressed[sizeof(symbol_t)];

  uint8_t symbol = *(symbol_t *)&(pIn[0]);
  size_t count = 0;
  int64_t lastRLE = 0;

  while (i < end)
  {
    for (size_t j = 0; j < sizeof(symbol_t); j++)
    {
      const uint8_t s = pIn[i + j];
      size_t k = 0;

      for (; k < 255; k++) // last one is automatically selected.
        if (symbols[k] == s)
          break;

      uncompressed[j] = k;

      if (k > 0)
      {
        for (size_t l = k; l >= 1; l--)
          symbols[l] = symbols[l - 1];

        symbols[0] = s;
      }
    }

    if (*(symbol_t *)uncompressed == symbol)
    {
      count++;

      if (count < RLEX_MTF_MIN_SIZE)
      {
        *(symbol_t *)&pOut[sizeof(symbol_t) * (count - 1)] = symbol;
      }
    }
    else
    {
      if (count >= RLEX_MTF_MIN_SIZE)
      {
        const int64_t storedCount = (count / sizeof(symbol_t)) - (RLEX_MTF_MIN_SIZE / sizeof(symbol_t)) + 1;

        if (storedCount <= 255)
        {
          *pOut = (uint8_t)storedCount;
          pOut++;
        }
        else
        {
          *pOut = 0;
          pOut++;
          *(uint32_t *)pOut = (uint32_t)storedCount;
          pOut += sizeof(uint32_t);
        }

        const int64_t range = i - lastRLE - count + 1;

        if (range > 255)
        {
          *pOut = (uint8_t)range;
          pOut++;
        }
        else
        {
          *pOut = 0;
          pOut++;
          *(uint32_t *)pOut = (uint32_t)range;
          pOut += sizeof(uint32_t);
        }

        const size_t copySize = i - count - lastRLE;

        memcpy(pOut, pIn + lastRLE, copySize);
        pOut += copySize;

        lastRLE = i;
      }
      else
      {
        pOut += sizeof(symbol_t) * (count - 1);
      }

      count = 1;
      symbol = *(symbol_t *)uncompressed;

      *(symbol_t *)pOut = symbol;
      pOut += sizeof(symbol_t);
    }

    i += sizeof(symbol_t);
  }

  for (; i < inSize; i++)
  {
    const uint8_t s = pIn[i];
    size_t k = 0;

    for (; k < 255; k++) // last one is automatically selected.
      if (symbols[k] == s)
        break;


  }

  return pOut - pOutStart;
}

#undef RLEX_MTF_MIN_SIZE
#undef RLEX_MTF_MAX_SIZE

#undef CONCAT_LITERALS3
#undef CONCAT3

#undef CONCAT_LITERALS2
#undef CONCAT2
