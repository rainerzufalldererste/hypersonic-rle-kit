#include "rle8.h"

#include "rleX_extreme_common.h"

#ifdef _MSC_VER
#pragma pack(1)
typedef struct
#else
typedef struct
__attribute__((packed))
#endif
{
  uint32_t uncompressedLength, compressedLength;
  uint8_t mode;
} rle_extreme_t;

uint32_t rle8_decompress_additional_size()
{
  return 128; // just to be on the safe side.
}

uint32_t rle8_compress_bounds(const uint32_t inSize)
{
  if (inSize > (1 << 30))
    return 0;

  return inSize + (16 + 4 + 1 + 4 + 1 + 64) * 2 + (3 * 4) + 1;
}

//////////////////////////////////////////////////////////////////////////

static uint8_t rle8_single_compress_get_approx_optimal_symbol_sse2(IN const uint8_t *pIn, const size_t inSize);
static uint8_t rle8_single_compress_get_approx_optimal_symbol_avx2(IN const uint8_t *pIn, const size_t inSize);

//////////////////////////////////////////////////////////////////////////

#include "rle8_extreme_cpu.h"

#define PACKED

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
  #define PREFER_7_BIT_OR_4_BYTE_COPY
#endif

//#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
//  #define SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
//#endif

#include "rle8_extreme_cpu.h"

//////////////////////////////////////////////////////////////////////////

static uint8_t rle8_single_compress_get_approx_optimal_symbol_sse2(IN const uint8_t *pIn, const size_t inSize)
{
  uint32_t prob[256];
  uint32_t pcount[256];
  bool consumed[256];

  memset(prob, 0, sizeof(prob));
  memset(pcount, 0, sizeof(pcount));
  memset(consumed, 0, sizeof(consumed));

  uint8_t lastSymbol = 0;
  uint32_t count = 0;

  if (pIn[0] != lastSymbol)
    pcount[lastSymbol] = (uint32_t)-1;

  int64_t inIndex = 0;
  lastSymbol = ~pIn[0];
  __m128i lastSymbol128 = _mm_set1_epi8(lastSymbol);
  const int64_t endInSize128 = inSize - sizeof(lastSymbol128);

  // This is far from optimal, but a lot faster than what we had previously. If you prefer accuracy, use what the normal rle8 uses.
  for (; inIndex < endInSize128; inIndex++)
  {
    const uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(lastSymbol128, _mm_loadu_si128((const __m128i *) & (pIn[inIndex]))));

    if (0xFFFF == mask)
    {
      count += sizeof(lastSymbol128) - 1;
      inIndex += sizeof(lastSymbol128) - 1;
    }
    else
    {
      if (mask != 0 || count > 1)
      {
#ifdef _MSC_VER
        unsigned long _zero;
        _BitScanForward64(&_zero, ~mask);
#else
        const uint64_t _zero = __builtin_ctzl(~mask);
#endif

        count += _zero;
        inIndex += _zero;

        prob[lastSymbol] += count;
        pcount[lastSymbol]++;
      }

      while (inIndex < endInSize128)
      {
        const __m128i current = _mm_loadu_si128((const __m128i *)(&pIn[inIndex]));
        const __m128i next = _mm_bsrli_si128(current, 1);
        const int32_t cmp = 0x7FFF & _mm_movemask_epi8(_mm_cmpeq_epi8(current, next));

        if (cmp == 0)
        {
          inIndex += sizeof(lastSymbol128) - 1;
        }
        else
        {
#ifdef _MSC_VER
          unsigned long _zero;
          _BitScanForward64(&_zero, cmp);
#else
          const uint64_t _zero = __builtin_ctzl(cmp);
#endif

          inIndex += _zero;
          break;
        }
      }

      count = 1;
      lastSymbol = pIn[inIndex];
      lastSymbol128 = _mm_set1_epi8(lastSymbol);
    }
  }

  prob[lastSymbol] += count;
  pcount[lastSymbol]++;

  size_t maxBytesSaved = 0;
  size_t maxBytesSavedIndexIndex = 0;

  for (size_t i = 0; i < 256; i++)
  {
    if (pcount[i] > 0 && prob[i] / pcount[i] > 2)
    {
      const size_t saved = prob[i] - (pcount[i] * 2);

      if (saved > maxBytesSaved)
      {
        maxBytesSaved = saved;
        maxBytesSavedIndexIndex = i;
      }
    }
  }

  return (uint8_t)maxBytesSavedIndexIndex;
}

// This appears to be slower than the SSE2 variant, so it's not being used currently.
#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static uint8_t rle8_single_compress_get_approx_optimal_symbol_avx2(IN const uint8_t *pIn, const size_t inSize)
{
  uint32_t prob[256];
  uint32_t pcount[256];
  bool consumed[256];

  memset(prob, 0, sizeof(prob));
  memset(pcount, 0, sizeof(pcount));
  memset(consumed, 0, sizeof(consumed));

  uint8_t lastSymbol = 0;
  uint32_t count = 0;

  if (pIn[0] != lastSymbol)
    pcount[lastSymbol] = (uint32_t)-1;

  int64_t inIndex = 0;
  lastSymbol = ~pIn[0];
  __m256i lastSymbol256 = _mm256_set1_epi8(lastSymbol);
  const int64_t endInSize256 = inSize - sizeof(lastSymbol256);

  // This is far from optimal, but a lot faster than what we had previously. If you prefer accuracy, use what the normal rle8 uses.
  for (; inIndex < endInSize256; inIndex++)
  {
    const uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(lastSymbol256, _mm256_loadu_si256((const __m256i *) & (pIn[inIndex]))));

    if (0xFFFFFFFF == mask)
    {
      count += sizeof(lastSymbol256) - 1;
      inIndex += sizeof(lastSymbol256) - 1;
    }
    else
    {
      if (mask != 0 || count > 1)
      {
#ifdef _MSC_VER
        unsigned long _zero;
        _BitScanForward64(&_zero, ~mask);
#else
        const uint64_t _zero = __builtin_ctzl(~mask);
#endif

        count += _zero;
        inIndex += _zero;

        prob[lastSymbol] += count;
        pcount[lastSymbol]++;
      }

      while (inIndex < endInSize256)
      {
        const __m256i current = _mm256_loadu_si256((const __m256i *)(&pIn[inIndex]));
        const __m256i next = _mm256_loadu_si256((const __m256i *)(&pIn[inIndex + 1]));
        const int32_t cmp = 0x7FFFFFFF & _mm256_movemask_epi8(_mm256_cmpeq_epi8(current, next));

        if (cmp == 0)
        {
          inIndex += sizeof(lastSymbol256) - 1;
        }
        else
        {
#ifdef _MSC_VER
          unsigned long _zero;
          _BitScanForward64(&_zero, cmp);
#else
          const uint64_t _zero = __builtin_ctzl(cmp);
#endif

          inIndex += _zero;
          break;
        }
      }

      count = 1;
      lastSymbol = pIn[inIndex];
      lastSymbol256 = _mm256_set1_epi8(lastSymbol);
    }
  }

  prob[lastSymbol] += count;
  pcount[lastSymbol]++;

  size_t maxBytesSaved = 0;
  size_t maxBytesSavedIndexIndex = 0;

  for (size_t i = 0; i < 256; i++)
  {
    if (pcount[i] > 0 && prob[i] / pcount[i] > 2)
    {
      const size_t saved = prob[i] - (pcount[i] * 2);

      if (saved > maxBytesSaved)
      {
        maxBytesSaved = saved;
        maxBytesSavedIndexIndex = i;
      }
    }
  }

  return (uint8_t)maxBytesSavedIndexIndex;
}
