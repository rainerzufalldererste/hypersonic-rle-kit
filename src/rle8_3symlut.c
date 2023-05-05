#include "rle8.h"

#include "rleX_extreme_common.h"

//////////////////////////////////////////////////////////////////////////

#define RLE8_3SYMLUT_MIN_RANGE_SHORT (2 + 1)
#define RLE8_3SYMLUT_MIN_RANGE_LONG (2 + 4 + 4 + 1)

#define RLE8_3SYMLUT_COUNT_BITS (7)
#define RLE8_3SYMLUT_RANGE_BITS (14 - RLE8_3SYMLUT_COUNT_BITS)
#define RLE8_3SYMLUT_MAX_TINY_COUNT ((1 << RLE8_3SYMLUT_COUNT_BITS) - 1)
#define RLE8_3SYMLUT_MAX_TINY_RANGE ((1 << RLE8_3SYMLUT_RANGE_BITS) - 1)

//////////////////////////////////////////////////////////////////////////

static int64_t rle8_3symlut_compress_sse2(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, IN OUT uint8_t *pSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE, uint8_t *pLastSymbol);
static int64_t rle8_3symlut_compress_avx2(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, IN OUT uint8_t *pSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE, uint8_t *pLastSymbol);

//////////////////////////////////////////////////////////////////////////

inline bool _rle8_3symlut_process_symbol(IN const uint8_t *pIn, OUT uint8_t *pOut, IN OUT size_t *pIndex, const int64_t count, const size_t i, int64_t *pLastRLE, const uint8_t symbol, uint8_t *pLastSymbols)
{
  size_t symbolMatchIndex = 0;

  for (; symbolMatchIndex < 3; symbolMatchIndex++)
    if (symbol == pLastSymbols[symbolMatchIndex])
      break;

  const int64_t range = i - *pLastRLE - count + 2;
  const int64_t storedCount = count - RLE8_3SYMLUT_MIN_RANGE_SHORT + 2;
  const size_t penalty = (range <= 0xFFFFF ? (range <= RLE8_3SYMLUT_MAX_TINY_RANGE ? 0 : 2) : 4) + (storedCount <= 0xFFFFF ? (storedCount <= (RLE8_3SYMLUT_MAX_TINY_COUNT) ? 0 : 2) : 4) + (size_t)(symbolMatchIndex == 3);

  if (count >= RLE8_3SYMLUT_MIN_RANGE_LONG || (count >= RLE8_3SYMLUT_MIN_RANGE_SHORT + penalty))
  {
    switch (symbolMatchIndex)
    {
    case 3:
    case 2:
      pLastSymbols[2] = pLastSymbols[1];
#ifdef __cplusplus
      [[fallthrough]]; // intentional fallthrough!
#endif

    case 1:
      pLastSymbols[1] = pLastSymbols[0];
      pLastSymbols[0] = symbol;
      break;

    case 0:
      break;
    }

    uint16_t storedCount7;
    uint16_t range7;

    if (storedCount <= RLE8_3SYMLUT_MAX_TINY_COUNT)
      storedCount7 = (uint8_t)storedCount;
    else if (storedCount <= 0xFFFF)
      storedCount7 = 1;
    else
      storedCount7 = 0;

    if (range <= RLE8_3SYMLUT_MAX_TINY_RANGE)
      range7 = (uint8_t)range;
    else if (range <= 0xFFFF)
      range7 = 1;
    else
      range7 = 0;

    size_t index = *pIndex;
    const uint16_t value = (uint16_t)(symbolMatchIndex << 14) | (storedCount7 << RLE8_3SYMLUT_RANGE_BITS) | range7;

    *((uint16_t *)&pOut[index]) = value;
    index += sizeof(uint16_t);

    if (symbolMatchIndex == 3)
    {
      pOut[index] = symbol;
      index++;
    }

    if (storedCount != storedCount7) // when decoding this should also check for storedCount being 0, because that implies that a uint32_t will follow containing the full count.
    {
      if (storedCount <= 0xFFFF)
      {
        *((uint16_t *)&pOut[index]) = (uint16_t)storedCount;
        index += sizeof(uint16_t);
      }
      else
      {
        *((uint32_t *)&pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }
    }

    if (range != range7) // when decoding this should also check for offset being 0, because that implies that a uint32_t will follow containing the full count.
    {
      if (range <= 0xFFFF)
      {
        *((uint16_t *)&pOut[index]) = (uint16_t)range;
        index += sizeof(uint16_t);
      }
      else
      {
        *((uint32_t *)&pOut[index]) = (uint32_t)range;
        index += sizeof(uint32_t);
      }
    }

    const size_t copySize = i - count - *pLastRLE;

    memcpy(pOut + index, pIn + *pLastRLE, copySize);
    index += copySize;

    *pLastRLE = i;
    *pIndex = index;

    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_3symlut_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_compress_bounds(inSize))
    return 0;

  ((uint32_t *)pOut)[0] = (uint32_t)inSize;

  size_t index = sizeof(uint32_t) * 2;
  int64_t i = 0;
  int64_t lastRLE = 0;

  int64_t count = 0;
  uint8_t symbol = ~(*pIn);
  uint8_t lastSymbols[3] = { 0x00, 0x7F, 0xFF };

  _DetectCPUFeatures();

  if (avx2Supported)
    i = rle8_3symlut_compress_avx2(pIn, inSize, pOut, &index, &symbol, &count, &lastRLE, lastSymbols);
  else
    i = rle8_3symlut_compress_sse2(pIn, inSize, pOut, &index, &symbol, &count, &lastRLE, lastSymbols);

  for (; i < inSize; i++)
  {
    if (pIn[i] == symbol)
    {
      count++;
    }
    else
    {
      _rle8_3symlut_process_symbol(pIn, pOut, &index, count, i, &lastRLE, symbol, lastSymbols);

      symbol = pIn[i];
      count = 1;
    }
  }

  // Copy / Encode remaining bytes.
  {
    const int64_t range = i - lastRLE - count + 2;

    if (_rle8_3symlut_process_symbol(pIn, pOut, &index, count, i, &lastRLE, symbol, lastSymbols))
    {
      *((uint16_t *)&pOut[index]) = 0b0000000010000001;
      index += sizeof(uint16_t);
      *((uint16_t *)&pOut[index]) = 0;
      index += sizeof(uint16_t);
      *((uint16_t *)&pOut[index]) = 0;
      index += sizeof(uint16_t);
    }
    else
    {
      *((uint16_t *)&pOut[index]) = 0b0000000010000000;
      index += sizeof(uint16_t);
      *((uint16_t *)&pOut[index]) = 0;
      index += sizeof(uint16_t);
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);

      const size_t copySize = i - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;
    }
  }

  // Store compressed length.
  ((uint32_t *)pOut)[1] = (uint32_t)index;

  return (uint32_t)index;
}

//////////////////////////////////////////////////////////////////////////

static int64_t rle8_3symlut_compress_sse2(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, IN OUT uint8_t *pSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE, uint8_t *pLastSymbol)
{
  const int64_t endInSize128 = inSize - sizeof(__m128i);
  int64_t i = 0;
  size_t index = *pOutIndex;
  uint8_t symbol = *pSymbol;
  __m128i symbol128 = _mm_set1_epi8(symbol);
  int64_t count = 0;
  int64_t lastRLE = 0;

#ifdef PACKED
  uint8_t lastSymbol = 0;
#endif

  while (i < endInSize128)
  {
    const uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(symbol128, _mm_loadu_si128((const __m128i *) & (pIn[i]))));

    if (0xFFFF == mask)
    {
      count += sizeof(symbol128);
      i += sizeof(symbol128);
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
        i += _zero;

        _rle8_3symlut_process_symbol(pIn, pOut, &index, count, i, pLastRLE, symbol, pLastSymbol);
      }

      while (i < endInSize128)
      {
        const __m128i current = _mm_loadu_si128((const __m128i *)(&pIn[i]));
        const __m128i next = _mm_bsrli_si128(current, 1);
        const int32_t cmp = 0x7FFF & _mm_movemask_epi8(_mm_cmpeq_epi8(current, next));

        if (cmp == 0)
        {
          i += sizeof(symbol128) - 1;
        }
        else
        {
#ifdef _MSC_VER
          unsigned long _zero;
          _BitScanForward64(&_zero, cmp);
#else
          const uint64_t _zero = __builtin_ctzl(cmp);
#endif

          i += _zero;
          break;
        }
      }

      symbol = pIn[i];
      symbol128 = _mm_set1_epi8(symbol);
      count = 1;
      i++;
    }
  }

  *pOutIndex = index;
  *pSymbol = symbol;
  *pCount = count;
  *pLastRLE = lastRLE;

  return i;
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static int64_t rle8_3symlut_compress_avx2(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, IN OUT uint8_t *pSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE, uint8_t *pLastSymbol)
{
  const int64_t endInSize256 = inSize - sizeof(__m256i);
  int64_t i = 0;
  uint8_t symbol = *pSymbol;
  __m256i symbol256 = _mm256_set1_epi8(symbol);
  int64_t count = 0;

#ifdef PACKED
  uint8_t lastSymbol = 0;
#endif

  while (i < endInSize256)
  {
    const uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(symbol256, _mm256_loadu_si256((const __m256i *) & (pIn[i]))));

    if (0xFFFFFFFF == mask)
    {
      count += sizeof(symbol256);
      i += sizeof(symbol256);
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
        i += _zero;

        _rle8_3symlut_process_symbol(pIn, pOut, pOutIndex, count, i, pLastRLE, symbol, pLastSymbol);
      }

      while (i < endInSize256)
      {
        const __m256i current = _mm256_loadu_si256((const __m256i *)(&pIn[i]));
        const __m256i next = _mm256_loadu_si256((const __m256i *)(&pIn[i + 1]));
        const int32_t cmp = 0x7FFFFFFF & _mm256_movemask_epi8(_mm256_cmpeq_epi8(current, next));

        if (cmp == 0)
        {
          i += sizeof(symbol256) - 1;
        }
        else
        {
#ifdef _MSC_VER
          unsigned long _zero;
          _BitScanForward64(&_zero, cmp);
#else
          const uint64_t _zero = __builtin_ctzl(cmp);
#endif

          i += _zero;
          break;
        }
      }

      symbol = pIn[i];
      symbol256 = _mm256_set1_epi8(symbol);
      count = 1;
      i++;
    }
  }

  *pSymbol = symbol;
  *pCount = count;

  return i;
}

//////////////////////////////////////////////////////////////////////////

static void rle8_3symlut_decompress_sse(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();
  __m128i other[2];

  other[0] = _mm_set1_epi8(0x7F);
  other[1] = _mm_set1_epi8(0xFF);

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    uint32_t offset = value & RLE8_3SYMLUT_MAX_TINY_RANGE;
    uint32_t symbolCount = (value >> RLE8_3SYMLUT_RANGE_BITS) & RLE8_3SYMLUT_MAX_TINY_COUNT;
    const uint16_t symbolIndex = value >> 14;

    switch (symbolIndex)
    {
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 2:
    {
      const __m128i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m128i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    offset -= 2;

    // memcpy.
    MEMCPY_SSE;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_3SYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_SSE;
  }
}

#ifndef _MSC_VER
__attribute__((target("sse4.1")))
#endif
static void rle8_3symlut_decompress_sse41(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();
  __m128i other[2];

  other[0] = _mm_set1_epi8(0x7F);
  other[1] = _mm_set1_epi8(0xFF);

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    uint32_t offset = value & RLE8_3SYMLUT_MAX_TINY_RANGE;
    uint32_t symbolCount = (value >> RLE8_3SYMLUT_RANGE_BITS) & RLE8_3SYMLUT_MAX_TINY_COUNT;
    const uint16_t symbolIndex = value >> 14;

    switch (symbolIndex)
    {
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 2:
    {
      const __m128i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m128i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    offset -= 2;

    // memcpy.
    MEMCPY_SSE41;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_3SYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_SSE;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#endif
static void rle8_3symlut_decompress_avx(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();
  __m256i other[2];

  other[0] = _mm256_set1_epi8(0x7F);
  other[1] = _mm256_set1_epi8(0xFF);

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    uint32_t offset = value & RLE8_3SYMLUT_MAX_TINY_RANGE;
    uint32_t symbolCount = (value >> RLE8_3SYMLUT_RANGE_BITS) & RLE8_3SYMLUT_MAX_TINY_COUNT;
    const uint16_t symbolIndex = value >> 14;

    switch (symbolIndex)
    {
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm256_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 2:
    {
      const __m256i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m256i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    offset -= 2;

    // memcpy.
    MEMCPY_AVX;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_3SYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_AVX;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static void rle8_3symlut_decompress_avx2(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();
  __m256i other[2];

  other[0] = _mm256_set1_epi8(0x7F);
  other[1] = _mm256_set1_epi8(0xFF);

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    uint32_t offset = value & RLE8_3SYMLUT_MAX_TINY_RANGE;
    uint32_t symbolCount = (value >> RLE8_3SYMLUT_RANGE_BITS) & RLE8_3SYMLUT_MAX_TINY_COUNT;
    const uint16_t symbolIndex = value >> 14;

    switch (symbolIndex)
    {
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm256_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 2:
    {
      const __m256i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m256i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    offset -= 2;

    // memcpy.
    MEMCPY_AVX2;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_3SYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_AVX2;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx512f")))
#endif
static void rle8_3symlut_decompress_avx512f(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m512i symbol = _mm512_setzero_si512();
  __m512i other[2];

  other[0] = _mm512_set1_epi8(0x7F);
  other[1] = _mm512_set1_epi8(0xFF);

  while (true)
  {
    const uint16_t value = *(uint16_t *)pInStart;
    pInStart += sizeof(uint16_t);

    uint32_t offset = value & RLE8_3SYMLUT_MAX_TINY_RANGE;
    uint32_t symbolCount = (value >> RLE8_3SYMLUT_RANGE_BITS) & RLE8_3SYMLUT_MAX_TINY_COUNT;
    const uint16_t symbolIndex = value >> 14;

    switch (symbolIndex)
    {
    case 3:
    {
      other[1] = other[0];
      other[0] = symbol;
      symbol = _mm512_set1_epi8(*pInStart);
      pInStart++;
      break;
    }

    case 2:
    {
      const __m512i tmp = other[1];
      other[1] = other[0];
      other[0] = symbol;
      symbol = tmp;
      break;
    }

    case 1:
    {
      const __m512i tmp = symbol;
      symbol = other[0];
      other[0] = tmp;
      break;
    }

    case 0:
      break;
    }

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (symbolCount == 1)
    {
      symbolCount = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);
    }

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
    else if (offset == 1)
    {
      offset = *(uint16_t *)pInStart;
      pInStart += sizeof(uint16_t);

      if (offset == 0)
        return;
    }

    if (offset == 0)
      return;

    offset -= 2;

    // memcpy.
    MEMCPY_AVX512;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_3SYMLUT_MIN_RANGE_SHORT - 2);

    // memset.
    MEMSET_AVX512;
  }
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_3symlut_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[1];
  const size_t expectedOutSize = ((uint32_t *)pIn)[0];
  pIn += sizeof(uint32_t) * 2;

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  _DetectCPUFeatures();

  if (avx512FSupported)
    rle8_3symlut_decompress_avx512f(pIn, pOut);
  else if (avx2Supported)
    rle8_3symlut_decompress_avx2(pIn, pOut);
  else if (avxSupported)
    rle8_3symlut_decompress_avx(pIn, pOut);
  else if (sse41Supported)
    rle8_3symlut_decompress_sse41(pIn, pOut);
  else
    rle8_3symlut_decompress_sse(pIn, pOut);

  return (uint32_t)expectedOutSize;
}
