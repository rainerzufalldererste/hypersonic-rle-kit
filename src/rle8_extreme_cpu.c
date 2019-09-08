#include "rle8.h"

#include "rleX_extreme_common.h"

#define RLE8_EXTREME_MULTI_SIZE_OF_SYMBOL_HEADER (1 + 1 + 1)
#define RLE8_EXTREME_MULTI_MAX_SIZE_OF_SYMBOL_HEADER (1 + 1 + 4 + 1 + 4)
#define RLE8_EXTREME_MULTI_MIN_RANGE_SHORT (6)
#define RLE8_EXTREME_MULTI_MIN_RANGE_LONG (9)

#define RLE8_EXTREME_SINGLE_SIZE_OF_SYMBOL_HEADER (1 + 1)
#define RLE8_EXTREME_SINGLE_MAX_SIZE_OF_SYMBOL_HEADER (1 + 4 + 1 + 4)
#define RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT (4)
#define RLE8_EXTREME_SINGLE_MIN_RANGE_LONG (8)

#define RLE8_EXTREME_MODE_MULTI 0
#define RLE8_EXTREME_MODE_SINGLE 1

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

#ifdef _MSC_VER
#pragma pack(1)
typedef struct
#else
typedef struct
__attribute__((packed))
#endif
{
  uint8_t symbol;
  uint8_t count; // + RLE8_EXTREME_MULTI_MIN_RANGE_SHORT
  uint8_t offset;
  uint32_t offsetIfNull;
} rle8_extreme_multi_symbol_debug_t;

#ifdef _MSC_VER
#pragma pack(1)
typedef struct
#else
typedef struct
__attribute__((packed))
#endif
{
  uint8_t count; // + RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT
  uint8_t offset;
  uint32_t offsetIfNull;
} rle8_extreme_single_symbol_debug_t;

void rle8_extreme_decompress_multi_sse(IN const uint8_t *pInStart, OUT uint8_t *pOut);
void rle8_extreme_decompress_multi_avx(IN const uint8_t *pInStart, OUT uint8_t *pOut);
void rle8_extreme_decompress_single_sse(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t symbol);
void rle8_extreme_decompress_single_avx(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t symbol);

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_extreme_compress_bounds(const uint32_t inSize)
{
  if (inSize > (1 << 30))
    return 0;

  return inSize + (max(RLE8_EXTREME_MULTI_MAX_SIZE_OF_SYMBOL_HEADER, RLE8_EXTREME_SINGLE_MAX_SIZE_OF_SYMBOL_HEADER) + 64) * 2 + sizeof(rle_extreme_t) + 1;
}

uint32_t rle8_extreme_decompress_additional_size()
{
  return 128; // just to be on the safe side.
}

uint32_t rle8_extreme_multi_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_extreme_compress_bounds(inSize))
    return 0;

  rle_extreme_t header;
  header.uncompressedLength = inSize;
  header.mode = RLE8_EXTREME_MODE_MULTI;

  memcpy(pOut, &header, sizeof(rle_extreme_t));

  size_t index = sizeof(rle_extreme_t);
  int64_t i = 0;
  int64_t lastRLE = 0;

  int64_t count = 0;
  uint8_t symbol = ~(*pIn);
  uint64_t symbol64 = symbol * (uint64_t)0x0101010101010101;

  for (; i < inSize; i++)
  {
    uint64_t current64 = *(const uint64_t *)&pIn[i];

    if (current64 == symbol64)
    {
      count += sizeof(symbol64);
      i += sizeof(symbol64) - 1;
    }
    else
    {
#ifdef _MSC_VER
      unsigned long _zero;
      _BitScanForward64(&_zero, current64 ^ symbol64);
#else
      const uint64_t _zero = __builtin_ctzl(current64 ^ symbol64) / 8;
#endif

      _zero /= 8;
      count += _zero;
      i += _zero;

      {
        const int64_t range = i - lastRLE - count + 1;

        if (range <= 255 && count >= RLE8_EXTREME_MULTI_MIN_RANGE_SHORT)
        {
          pOut[index] = symbol;
          index++;

          const int64_t storedCount = count - RLE8_EXTREME_MULTI_MIN_RANGE_SHORT + 1;

          if (storedCount <= 255)
          {
            pOut[index] = (uint8_t)storedCount;
            index++;
          }
          else
          {
            pOut[index] = 0;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          pOut[index] = (uint8_t)range;
          index++;

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
        else if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_LONG)
        {
          pOut[index] = symbol;
          index++;

          const int64_t storedCount = count - RLE8_EXTREME_MULTI_MIN_RANGE_SHORT + 1;

          if (storedCount <= 255)
          {
            pOut[index] = (uint8_t)storedCount;
            index++;
          }
          else
          {
            pOut[index] = 0;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          pOut[index] = 0;
          index++;
          *((uint32_t *)&pOut[index]) = (uint32_t)range;
          index += sizeof(uint32_t);

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
      }

      symbol = pIn[i];
      symbol64 = symbol * (uint64_t)0x0101010101010101;
      count = 1;
    }
  }

  {
    const int64_t range = i - lastRLE - count + 1;

    if (range <= 255 && count >= RLE8_EXTREME_MULTI_MIN_RANGE_SHORT)
    {
      pOut[index] = symbol;
      index++;

      const int64_t storedCount = count - RLE8_EXTREME_MULTI_MIN_RANGE_SHORT + 1;

      if (storedCount <= 255)
      {
        pOut[index] = (uint8_t)storedCount;
        index++;
      }
      else
      {
        pOut[index] = 0;
        index++;
        *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }

      pOut[index] = (uint8_t)range;
      index++;

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

      pOut[index] = 0;
      index++;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);

      lastRLE = i;
    }
    else if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_LONG)
    {
      pOut[index] = symbol;
      index++;

      const int64_t storedCount = count - RLE8_EXTREME_MULTI_MIN_RANGE_SHORT + 1;

      if (storedCount <= 255)
      {
        pOut[index] = (uint8_t)storedCount;
        index++;
      }
      else
      {
        pOut[index] = 0;
        index++;
        *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }

      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

      pOut[index] = 0;
      index++;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);

      lastRLE = i;
    }
    else
    {
      pOut[index] = 0;
      index++;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);

      const size_t copySize = i - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;
    }
  }

  // Store compressed length.
  ((rle_extreme_t *)pOut)->compressedLength = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t rle8_extreme_single_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_extreme_compress_bounds(inSize))
    return 0;

  rle_extreme_t header;
  header.uncompressedLength = inSize;
  header.mode = RLE8_EXTREME_MODE_SINGLE;

  memcpy(pOut, &header, sizeof(rle_extreme_t));

  uint8_t maxFreqSymbol = 0;

  // TODO: This is inaccurate and just stolen from rle8.
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

    for (size_t i = 0; i < inSize; i++)
    {
      if (pIn[i] == lastSymbol)
      {
        count++;
      }
      else
      {
        prob[lastSymbol] += count;
        pcount[lastSymbol]++;
        count = 1;
        lastSymbol = pIn[i];
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

    maxFreqSymbol = (uint8_t)maxBytesSavedIndexIndex;
  }

  size_t index = sizeof(rle_extreme_t);
  
  pOut[index] = maxFreqSymbol;
  index++;
  
  int64_t i = 0;
  int64_t lastRLE = 0;

  int64_t count = 0;

  // TODO: This can be improved by using sse / avx2 memchr equivalents.
  for (; i < inSize; i++)
  {
    if (pIn[i] == maxFreqSymbol)
    {
      count++;
    }
    else
    {
      {
        const int64_t range = i - lastRLE - count + 1;

        if (range <= 255 && count >= RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT)
        {
          const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;
        
          if (storedCount <= 255)
          {
            pOut[index] = (uint8_t)storedCount;
            index++;
          }
          else
          {
            pOut[index] = 0;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }
        
          pOut[index] = (uint8_t)range;
          index++;

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
        else if (count >= RLE8_EXTREME_SINGLE_MIN_RANGE_LONG)
        {
          const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

          if (storedCount <= 255)
          {
            pOut[index] = (uint8_t)storedCount;
            index++;
          }
          else
          {
            pOut[index] = 0;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          pOut[index] = 0;
          index++;
          *((uint32_t *)&pOut[index]) = (uint32_t)range;
          index += sizeof(uint32_t);

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
      }

      count = (pIn[i] == maxFreqSymbol);
    }
  }

  {
    const int64_t range = i - lastRLE - count + 1;

    if (range <= 255 && count >= RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT)
    {
      const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

      if (storedCount <= 255)
      {
        pOut[index] = (uint8_t)storedCount;
        index++;
      }
      else
      {
        pOut[index] = 0;
        index++;
        *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }

      pOut[index] = (uint8_t)range;
      index++;

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);

      lastRLE = i;
    }
    else if (count >= RLE8_EXTREME_SINGLE_MIN_RANGE_LONG)
    {
      const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

      if (storedCount <= 255)
      {
        pOut[index] = (uint8_t)storedCount;
        index++;
      }
      else
      {
        pOut[index] = 0;
        index++;
        *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
        index += sizeof(uint32_t);
      }

      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);

      lastRLE = i;
    }
    else
    {
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)(range + count);
      index += sizeof(uint32_t);

      const size_t copySize = i - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;
    }
  }

  // Store compressed length.
  ((rle_extreme_t *)pOut)->compressedLength = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t rle8_extreme_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((rle_extreme_t *)pIn)->compressedLength;
  const size_t expectedOutSize = ((rle_extreme_t *)pIn)->uncompressedLength;
  const uint8_t mode = ((rle_extreme_t *)pIn)->mode;

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  size_t index = sizeof(rle_extreme_t);

  _DetectCPUFeatures();

  switch (mode)
  {
  case RLE8_EXTREME_MODE_MULTI:
  {
    pIn += index;

    if (avxSupported)
      rle8_extreme_decompress_multi_avx(pIn, pOut);
    else
      rle8_extreme_decompress_multi_sse(pIn, pOut);
    
    break;
  }

  case RLE8_EXTREME_MODE_SINGLE:
  {
    const uint8_t symbol = pIn[index];
    index++;

    pIn += index;

    if (avxSupported)
      rle8_extreme_decompress_single_avx(pIn, pOut, symbol);
    else
      rle8_extreme_decompress_single_sse(pIn, pOut, symbol);

    break;
  }

  default:
    return 0;
  }

  return (uint32_t)expectedOutSize;
}

//////////////////////////////////////////////////////////////////////////

void rle8_extreme_decompress_multi_sse(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol;

  while (true)
  {
#ifdef _DEBUG
    rle8_extreme_multi_symbol_debug_t *pSymbol = (rle8_extreme_multi_symbol_debug_t *)pInStart;
    (void)pSymbol;
#endif

    symbol = _mm_set1_epi8(*pInStart);
    pInStart++;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    offset = (size_t)*pInStart;
    pInStart++;

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }

    offset--;

    // memcpy.
    MEMCPY_SSE;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_MULTI_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_SSE;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#endif
void rle8_extreme_decompress_multi_avx(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol;

  while (true)
  {
#ifdef _DEBUG
    rle8_extreme_multi_symbol_debug_t *pSymbol = (rle8_extreme_multi_symbol_debug_t *)pInStart;
    (void)pSymbol;
#endif

    symbol = _mm256_set1_epi8(*pInStart);
    pInStart++;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    offset = (size_t)*pInStart;
    pInStart++;

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }

    offset--;

    // memcpy.
    MEMCPY_AVX;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_MULTI_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_AVX;
  }
}

void rle8_extreme_decompress_single_sse(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t singleSymbol)
{
  size_t offset, symbolCount;
  const __m128i symbol = _mm_set1_epi8(singleSymbol);

  while (true)
  {
#ifdef _DEBUG
    rle8_extreme_single_symbol_debug_t *pSymbol = (rle8_extreme_single_symbol_debug_t *)pInStart;
    (void)pSymbol;
#endif

    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    offset = (size_t)*pInStart;
    pInStart++;

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }

    offset--;

    // memcpy.
    MEMCPY_SSE;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_SSE;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#endif
void rle8_extreme_decompress_single_avx(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t singleSymbol)
{
  size_t offset, symbolCount;
  const __m256i symbol = _mm256_set1_epi8(singleSymbol);

  while (true)
  {
#ifdef _DEBUG
    rle8_extreme_single_symbol_debug_t *pSymbol = (rle8_extreme_single_symbol_debug_t *)pInStart;
    (void)pSymbol;
#endif

    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    offset = (size_t)*pInStart;
    pInStart++;

    if (offset == 0)
    {
      offset = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }

    offset--;

    // memcpy.
    MEMCPY_AVX;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_AVX;
  }
}
