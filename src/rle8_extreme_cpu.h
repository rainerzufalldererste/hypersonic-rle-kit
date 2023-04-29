
#ifndef PACKED
  #define RLE8_EXTREME_MULTI_SIZE_OF_SYMBOL_HEADER (1 + 1 + 1)
  #define RLE8_EXTREME_MULTI_MAX_SIZE_OF_SYMBOL_HEADER (1 + 1 + 4 + 1 + 4)
  #define RLE8_EXTREME_MULTI_MIN_RANGE_SHORT (6)
  #define RLE8_EXTREME_MULTI_MIN_RANGE_LONG (9)
  
  #define RLE8_EXTREME_SINGLE_SIZE_OF_SYMBOL_HEADER (1 + 1)
  #define RLE8_EXTREME_SINGLE_MAX_SIZE_OF_SYMBOL_HEADER (1 + 4 + 1 + 4)
  #define RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT (4)
  #define RLE8_EXTREME_SINGLE_MIN_RANGE_LONG (8)
#else
  #define RLE8_EXTREME_MULTI_SIZE_OF_SYMBOL_HEADER (1 + 1)
  #define RLE8_EXTREME_MULTI_MAX_SIZE_OF_SYMBOL_HEADER (1 + 1 + 4 + 4)
  #define RLE8_EXTREME_MULTI_MIN_RANGE_SHORT (1 + 1 + 1)
  #define RLE8_EXTREME_MULTI_MIN_RANGE_MEDIUM (1 + 1 + 1 + 1)
  #define RLE8_EXTREME_MULTI_MIN_RANGE_LONG (1 + 1 + 4 + 4 + 1)
  
  #define RLE8_EXTREME_SINGLE_SIZE_OF_SYMBOL_HEADER (1 + 1)
  #define RLE8_EXTREME_SINGLE_MAX_SIZE_OF_SYMBOL_HEADER (4 + 4 + 1)
  #define RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT (1 + 1)
  #define RLE8_EXTREME_SINGLE_MIN_RANGE_MEDIUM (1 + 1 + 4)
  #define RLE8_EXTREME_SINGLE_MIN_RANGE_LONG (1 + 4 + 1 + 4)
#endif

#define RLE8_EXTREME_MODE_MULTI 0
#define RLE8_EXTREME_MODE_SINGLE 1

#ifndef PACKED
  #define CODEC extreme
#else
  #define CODEC extreme_packed
#endif

//#ifdef PACKED
//  #define SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
//#endif

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
} CONCAT3(rle8_, CODEC, _multi_symbol_debug_t);

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
} CONCAT3(rle8_, CODEC, _single_symbol_debug_t);

static int64_t CONCAT3(rle8_, CODEC, _compress_multi_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, IN OUT uint8_t *pSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE);
static int64_t CONCAT3(rle8_, CODEC, _compress_multi_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, IN OUT uint8_t *pSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE);

static int64_t CONCAT3(rle8_, CODEC, _compress_single_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, const uint8_t maxFreqSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE);
static int64_t CONCAT3(rle8_, CODEC, _compress_single_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, const uint8_t maxFreqSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE);

static void CONCAT3(rle8_, CODEC, _decompress_multi_sse)(IN const uint8_t *pInStart, OUT uint8_t *pOut);
static void CONCAT3(rle8_, CODEC, _decompress_multi_sse41)(IN const uint8_t *pInStart, OUT uint8_t *pOut);
static void CONCAT3(rle8_, CODEC, _decompress_multi_avx)(IN const uint8_t *pInStart, OUT uint8_t *pOut);
static void CONCAT3(rle8_, CODEC, _decompress_multi_avx2)(IN const uint8_t *pInStart, OUT uint8_t *pOut);
static void CONCAT3(rle8_, CODEC, _decompress_multi_avx512f)(IN const uint8_t *pInStart, OUT uint8_t *pOut);

static void CONCAT3(rle8_, CODEC, _decompress_single_sse)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t symbol);
static void CONCAT3(rle8_, CODEC, _decompress_single_sse41)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t symbol);
static void CONCAT3(rle8_, CODEC, _decompress_single_avx)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t symbol);
static void CONCAT3(rle8_, CODEC, _decompress_single_avx2)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t symbol);
static void CONCAT3(rle8_, CODEC, _decompress_single_avx512f)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t symbol);

//////////////////////////////////////////////////////////////////////////

uint32_t CONCAT3(rle8_, CODEC, _compress_bounds)(const uint32_t inSize)
{
  if (inSize > (1 << 30))
    return 0;

  return inSize + (max(RLE8_EXTREME_MULTI_MAX_SIZE_OF_SYMBOL_HEADER, RLE8_EXTREME_SINGLE_MAX_SIZE_OF_SYMBOL_HEADER) + 64) * 2 + sizeof(rle_extreme_t) + 1;
}

uint32_t CONCAT3(rle8_, CODEC, _multi_compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < CONCAT3(rle8_, CODEC, _compress_bounds)(inSize))
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

  _DetectCPUFeatures();

  if (avx2Supported)
    i = CONCAT3(rle8_, CODEC, _compress_multi_avx2)(pIn, inSize, pOut, &index, &symbol, &count, &lastRLE);
  else
    i = CONCAT3(rle8_, CODEC, _compress_multi_sse2)(pIn, inSize, pOut, &index, &symbol, &count, &lastRLE);

  for (; i < inSize; i++)
  {
    if (pIn[i] == symbol)
    {
      count++;
    }
    else
    {
#ifndef PACKED
      if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
      if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_LONG)
#endif
      {
        const int64_t storedCount = count - RLE8_EXTREME_MULTI_MIN_RANGE_SHORT + 1;

#ifndef PACKED
        pOut[index] = symbol;
        index++;

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
#else
        if (storedCount <= 127)
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

        pOut[index] = symbol;
        index++;
#endif

        const int64_t range = i - lastRLE - count + 1;

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
        if (range > 255)
        {
          pOut[index] = 0;
          index++;
          *((uint32_t *)&pOut[index]) = (uint32_t)range;
          index += sizeof(uint32_t);
        }
        else
        {
          pOut[index] = (uint8_t)range;
          index++;
        }
#else
        if (range <= 127)
        {
          pOut[index] = (uint8_t)(range << 1);
          index++;
        }
        else
        {
          *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
          index += sizeof(uint32_t);
        }
#endif

        const size_t copySize = i - count - lastRLE;

        memcpy(pOut + index, pIn + lastRLE, copySize);
        index += copySize;

        lastRLE = i;
      }

      symbol = pIn[i];
      count = 1;
    }
  }

  // Copy / Encode remaining bytes.
  {
    const int64_t range = i - lastRLE - count + 1;

#ifndef PACKED
    if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
    if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_LONG)
#endif
    {
      const int64_t storedCount = count - RLE8_EXTREME_MULTI_MIN_RANGE_SHORT + 1;

#ifndef PACKED
      pOut[index] = symbol;
      index++;

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
#else
      if (storedCount <= 127)
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

      pOut[index] = symbol;
      index++;
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
      if (range > 255)
      {
        pOut[index] = 0;
        index++;
        *((uint32_t *)&pOut[index]) = (uint32_t)range;
        index += sizeof(uint32_t);
      }
      else
      {
        pOut[index] = (uint8_t)range;
        index++;
      }
#else
      if (range <= 127)
      {
        pOut[index] = (uint8_t)(range << 1);
        index++;
      }
      else
      {
        *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
        index += sizeof(uint32_t);
      }
#endif

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

      lastRLE = i;

#ifndef PACKED
      pOut[index] = 0;
      index++;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0b10000000;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      *((uint32_t *)&pOut[index]) = (uint32_t)1;
      index += sizeof(uint32_t);
#endif

      lastRLE = i;
    }
    else
    {
#ifndef PACKED
      pOut[index] = 0;
      index++;
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0b10000000;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)range;
      index += sizeof(uint32_t);
#else
      *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
      index += sizeof(uint32_t);
#endif

      const size_t copySize = i - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;
    }
  }

  // Store compressed length.
  ((rle_extreme_t *)pOut)->compressedLength = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t CONCAT3(rle8_, CODEC, _single_compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < CONCAT3(rle8_, CODEC, _compress_bounds)(inSize))
    return 0;

  rle_extreme_t header;
  header.uncompressedLength = inSize;
  header.mode = RLE8_EXTREME_MODE_SINGLE;

  memcpy(pOut, &header, sizeof(rle_extreme_t));

  _DetectCPUFeatures();

  uint8_t maxFreqSymbol;

  // The AVX2 variant appears to be slower, so we're just always calling the SSE2 version.
  //if (avx2Supported)
  //  maxFreqSymbol = rle8_extreme_single_compress_get_approx_optimal_symbol_avx2(pIn, inSize);
  //else
  maxFreqSymbol = rle8_extreme_single_compress_get_approx_optimal_symbol_sse2(pIn, inSize);

  size_t index = sizeof(rle_extreme_t);

  pOut[index] = maxFreqSymbol;
  index++;

  int64_t i = 0;
  int64_t lastRLE = 0;

  int64_t count = 0;

  // The AVX2 variant appears to be slower, so we're just always calling the SSE2 version.
  //if (avx2Supported)
  //  i = CONCAT3(rle8_, CODEC, _compress_single_avx2)(pIn, inSize, pOut, &index, maxFreqSymbol, &count, &lastRLE);
  //else
  i = CONCAT3(rle8_, CODEC, _compress_single_sse2)(pIn, inSize, pOut, &index, maxFreqSymbol, &count, &lastRLE);

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

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
        if (range <= 255 && count >= RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT)
#else
        if (range <= 127 && count >= RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT)
#endif
        {
          const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
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
#else
          if (storedCount <= 127)
          {
            pOut[index] = (uint8_t)(storedCount << 1);
            index++;
          }
          else
          {
            *(uint32_t *)&(pOut[index]) = (uint32_t)(storedCount << 1) | (uint32_t)1;
            index += sizeof(uint32_t);
          }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
          pOut[index] = (uint8_t)range;
          index++;
#else
          pOut[index] = (uint8_t)(range << 1);
          index++;
#endif

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
        else if (count >= RLE8_EXTREME_SINGLE_MIN_RANGE_LONG)
        {
          const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
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
#else
          if (storedCount <= 127)
          {
            pOut[index] = (uint8_t)(storedCount << 1);
            index++;
          }
          else
          {
            *(uint32_t *)&(pOut[index]) = (uint32_t)(storedCount << 1) | (uint32_t)1;
            index += sizeof(uint32_t);
          }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
          //if (range <= 255)
          //{
          //  pOut[index] = (uint8_t)range;
          //  index++;
          //}
          //else
          {
            pOut[index] = 0;
            index++;
            *((uint32_t *)&pOut[index]) = (uint32_t)range;
            index += sizeof(uint32_t);
          }
#else
          //if (range <= 127)
          //{
          //  pOut[index] = (uint8_t)(range << 1);
          //  index++;
          //}
          //else
          {
            *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
            index += sizeof(uint32_t);
          }
#endif

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

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
    if (range <= 255 && count >= RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT)
#else
    if (range <= 127 && count >= RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT)
#endif
    {
      const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
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
#else
      if (storedCount <= 127)
      {
        pOut[index] = (uint8_t)(storedCount << 1);
        index++;
      }
      else
      {
        *(uint32_t *)&(pOut[index]) = (uint32_t)(storedCount << 1) | (uint32_t)1;
        index += sizeof(uint32_t);
      }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
      pOut[index] = (uint8_t)range;
      index++;
#else
      pOut[index] = (uint8_t)(range << 1);
      index++;
#endif

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
#endif

      lastRLE = i;
    }
    else if (count >= RLE8_EXTREME_SINGLE_MIN_RANGE_LONG)
    {
      const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
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
#else
      if (storedCount <= 127)
      {
        pOut[index] = (uint8_t)(storedCount << 1);
        index++;
      }
      else
      {
        *(uint32_t *)&(pOut[index]) = (uint32_t)(storedCount << 1) | (uint32_t)1;
        index += sizeof(uint32_t);
      }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
      //if (range <= 255)
      //{
      //  pOut[index] = (uint8_t)range;
      //  index++;
      //}
      //else
      {
        pOut[index] = 0;
        index++;
        *((uint32_t *)&pOut[index]) = (uint32_t)range;
        index += sizeof(uint32_t);
      }
#else
      //if (range <= 127)
      //{
      //  pOut[index] = (uint8_t)(range << 1);
      //  index++;
      //}
      //else
      {
        *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
        index += sizeof(uint32_t);
      }
#endif

      const size_t copySize = i - count - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
#endif

      lastRLE = i;
    }
    else
    {
#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = 0;
      index += sizeof(uint32_t);
#else
      pOut[index] = 0;
      index++;
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
      pOut[index] = 0;
      index++;
      *((uint32_t *)&pOut[index]) = (uint32_t)(range + count);
      index += sizeof(uint32_t);
#else
      *((uint32_t *)&pOut[index]) = (uint32_t)((range + count) << 1) | (uint32_t)1;
      index += sizeof(uint32_t);
#endif

      const size_t copySize = i - lastRLE;

      memcpy(pOut + index, pIn + lastRLE, copySize);
      index += copySize;
    }
  }

  // Store compressed length.
  ((rle_extreme_t *)pOut)->compressedLength = (uint32_t)index;

  return (uint32_t)index;
}

uint32_t CONCAT3(rle8_, CODEC, _decompress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
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

    if (avx512FSupported)
      CONCAT3(rle8_, CODEC, _decompress_multi_avx512f)(pIn, pOut);
    else if (avx2Supported)
      CONCAT3(rle8_, CODEC, _decompress_multi_avx2)(pIn, pOut);
    else if (avxSupported)
      CONCAT3(rle8_, CODEC, _decompress_multi_avx)(pIn, pOut);
    else if (sse41Supported)
      CONCAT3(rle8_, CODEC, _decompress_multi_sse41)(pIn, pOut);
    else
      CONCAT3(rle8_, CODEC, _decompress_multi_sse)(pIn, pOut);

    break;
  }

  case RLE8_EXTREME_MODE_SINGLE:
  {
    const uint8_t symbol = pIn[index];
    index++;

    pIn += index;

    if (avx512FSupported)
      CONCAT3(rle8_, CODEC, _decompress_single_avx512f)(pIn, pOut, symbol);
    else if (avx2Supported)
      CONCAT3(rle8_, CODEC, _decompress_single_avx2)(pIn, pOut, symbol);
    else if (avxSupported)
      CONCAT3(rle8_, CODEC, _decompress_single_avx)(pIn, pOut, symbol);
    else if (sse41Supported)
      CONCAT3(rle8_, CODEC, _decompress_single_sse41)(pIn, pOut, symbol);
    else
      CONCAT3(rle8_, CODEC, _decompress_single_sse)(pIn, pOut, symbol);

    break;
  }

  default:
    return 0;
  }

  return (uint32_t)expectedOutSize;
}

//////////////////////////////////////////////////////////////////////////

static int64_t CONCAT3(rle8_, CODEC, _compress_multi_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, IN OUT uint8_t *pSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE)
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
    const uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(symbol128, _mm_loadu_si128((const __m128i *)&(pIn[i]))));

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

#ifndef PACKED
        if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
        const int64_t range = i - lastRLE - count + 1;

        if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_LONG || (range <= 127 && ((symbol == lastSymbol && count >= RLE8_EXTREME_MULTI_MIN_RANGE_SHORT) || (count >= RLE8_EXTREME_MULTI_MIN_RANGE_MEDIUM))))
#endif
        {
          const int64_t storedCount = count - RLE8_EXTREME_MULTI_MIN_RANGE_SHORT + 1;

#ifndef PACKED
          pOut[index] = symbol;
          index++;

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
#else
          const uint8_t isSameSymbolMask = ((symbol == lastSymbol) << 7);
          lastSymbol = symbol;

          if (storedCount <= 127)
          {
            pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
            index++;
          }
          else
          {
            pOut[index] = isSameSymbolMask;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          if (!isSameSymbolMask)
          {
            pOut[index] = symbol;
            index++;
          }
#endif

#ifndef PACKED
          const int64_t range = i - lastRLE - count + 1;
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
          if (range > 255)
          {
            pOut[index] = 0;
            index++;
            *((uint32_t *)&pOut[index]) = (uint32_t)range;
            index += sizeof(uint32_t);
          }
          else
          {
            pOut[index] = (uint8_t)range;
            index++;
          }
#else
          if (range <= 127)
          {
            pOut[index] = (uint8_t)(range << 1);
            index++;
          }
          else
          {
            *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
            index += sizeof(uint32_t);
          }
#endif

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
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
static int64_t CONCAT3(rle8_, CODEC, _compress_multi_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, IN OUT uint8_t *pSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE)
{
  const int64_t endInSize256 = inSize - sizeof(__m256i);
  int64_t i = 0;
  size_t index = *pOutIndex;
  uint8_t symbol = *pSymbol;
  __m256i symbol256 = _mm256_set1_epi8(symbol);
  int64_t count = 0;
  int64_t lastRLE = 0;

#ifdef PACKED
  uint8_t lastSymbol = 0;
#endif

  while (i < endInSize256)
  {
    const uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(symbol256, _mm256_loadu_si256((const __m256i *)&(pIn[i]))));

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

#ifndef PACKED
        if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
        const int64_t range = i - lastRLE - count + 1;

        if (count >= RLE8_EXTREME_MULTI_MIN_RANGE_LONG || (range <= 127 && ((symbol == lastSymbol && count >= RLE8_EXTREME_MULTI_MIN_RANGE_SHORT) || (count >= RLE8_EXTREME_MULTI_MIN_RANGE_MEDIUM))))
#endif
        {
          const int64_t storedCount = count - RLE8_EXTREME_MULTI_MIN_RANGE_SHORT + 1;

#ifndef PACKED
          pOut[index] = symbol;
          index++;

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
#else
          const uint8_t isSameSymbolMask = ((symbol == lastSymbol) << 7);
          lastSymbol = symbol;

          if (storedCount <= 127)
          {
            pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
            index++;
          }
          else
          {
            pOut[index] = isSameSymbolMask;
            index++;
            *(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
            index += sizeof(uint32_t);
          }

          if (!isSameSymbolMask)
          {
            pOut[index] = symbol;
            index++;
          }
#endif

#ifndef PACKED
          const int64_t range = i - lastRLE - count + 1;
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
          if (range > 255)
          {
            pOut[index] = 0;
            index++;
            *((uint32_t *)&pOut[index]) = (uint32_t)range;
            index += sizeof(uint32_t);
          }
          else
          {
            pOut[index] = (uint8_t)range;
            index++;
          }
#else
          if (range <= 127)
          {
            pOut[index] = (uint8_t)(range << 1);
            index++;
          }
          else
          {
            *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
            index += sizeof(uint32_t);
          }
#endif

          //printf("> count: %" PRIu64 ", range: %" PRIu64 "\n", storedCount, range);

          const size_t copySize = i - count - lastRLE;

          memcpy(pOut + index, pIn + lastRLE, copySize);
          index += copySize;

          lastRLE = i;
        }
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

  *pOutIndex = index;
  *pSymbol = symbol;
  *pCount = count;
  *pLastRLE = lastRLE;

  return i;
}

//////////////////////////////////////////////////////////////////////////

static int64_t CONCAT3(rle8_, CODEC, _compress_single_sse2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, const uint8_t maxFreqSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE)
{
  int64_t i = 0;
  size_t index = *pOutIndex;
  int64_t count = 0;
  int64_t lastRLE = 0;
  size_t wastedChances = 0;
  size_t firstWastedChanceIndex = 0;

  const __m128i symbol128 = _mm_set1_epi8(maxFreqSymbol);
  const int64_t endInSize128 = inSize - sizeof(symbol128);

  for (; i < endInSize128; i++)
  {
    const uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(symbol128, _mm_loadu_si128((const __m128i *) & (pIn[i]))));

    if (0xFFFF == mask)
    {
      count += sizeof(symbol128);
      i += sizeof(symbol128) - 1;
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

        const int64_t range = i - lastRLE - count + 1;

        if (count >= RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT)
        {
#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
          if (range <= 255)
#else
          if (range <= 127)
#endif
          {
            const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
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
#else
            if (storedCount <= 127)
            {
              pOut[index] = (uint8_t)(storedCount << 1);
              index++;
            }
            else
            {
              *(uint32_t *)&(pOut[index]) = (uint32_t)(storedCount << 1) | (uint32_t)1;
              index += sizeof(uint32_t);
            }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
            pOut[index] = (uint8_t)range;
            index++;
#else
            pOut[index] = (uint8_t)(range << 1);
            index++;
#endif

            const size_t copySize = i - count - lastRLE;

            memcpy(pOut + index, pIn + lastRLE, copySize);
            index += copySize;

            lastRLE = i;
            wastedChances = 0;
          }
#ifndef PACKED
          else if (count >= RLE8_EXTREME_SINGLE_MIN_RANGE_LONG)
#else
          else if ((count >= RLE8_EXTREME_SINGLE_MIN_RANGE_LONG) || (count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1 <= 255 && count >= RLE8_EXTREME_SINGLE_MIN_RANGE_MEDIUM))
#endif
          {
            const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
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
#else
            if (storedCount <= 127)
            {
              pOut[index] = (uint8_t)(storedCount << 1);
              index++;
            }
            else
            {
              *(uint32_t *)&(pOut[index]) = (uint32_t)(storedCount << 1) | (uint32_t)1;
              index += sizeof(uint32_t);
            }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
            pOut[index] = 0;
            index++;
            *((uint32_t *)&pOut[index]) = (uint32_t)range;
            index += sizeof(uint32_t);
#else
            *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
            index += sizeof(uint32_t);
#endif

            const size_t copySize = i - count - lastRLE;

            memcpy(pOut + index, pIn + lastRLE, copySize);
            index += copySize;

            lastRLE = i;
            wastedChances = 0;
          }
#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
          else
          {
            wastedChances++;

            if (wastedChances == 1 || i - firstWastedChanceIndex > 255)
            {
              firstWastedChanceIndex = i - count;
              wastedChances = 1;
            }
            else if (wastedChances > 2)
            {
              i = firstWastedChanceIndex;
              wastedChances = 0;
              count = 0;

              for (; i < endInSize128; i++)
                if (pIn[i] == maxFreqSymbol)
                  ++count;
                else
                  break;

              const int64_t wastedRange = i - lastRLE - count + 1;
              const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

              pOut[index] = (uint8_t)storedCount;
              index++;

              pOut[index] = 0;
              index++;
              *((uint32_t *)&pOut[index]) = (uint32_t)wastedRange;
              index += sizeof(uint32_t);

              const size_t copySize = i - count - lastRLE;

              memcpy(pOut + index, pIn + lastRLE, copySize);
              index += copySize;

              lastRLE = i;
            }
          }
#endif
        }
      }

      count = 0;

      while (i < endInSize128)
      {
        const int32_t cmp = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128((const __m128i *)(&pIn[i])), symbol128));

        if (cmp == 0 || ((cmp & 0x8000) == 0 && __builtin_popcount((uint32_t)cmp) < RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT))
        {
          i += sizeof(symbol128);
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
          count = 1;
          break;
        }
      }
    }
  }

  *pOutIndex = index;
  *pCount = count;
  *pLastRLE = lastRLE;

  return i;
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static int64_t CONCAT3(rle8_, CODEC, _compress_single_avx2)(IN const uint8_t *pIn, const size_t inSize, OUT uint8_t *pOut, IN OUT size_t *pOutIndex, const uint8_t maxFreqSymbol, OUT int64_t *pCount, OUT int64_t *pLastRLE)
{
  int64_t i = 0;
  size_t index = *pOutIndex;
  int64_t count = 0;
  int64_t lastRLE = 0;
  size_t wastedChances = 0;
  size_t firstWastedChanceIndex = 0;

  const __m256i symbol256 = _mm256_set1_epi8(maxFreqSymbol);
  const int64_t endInSize256 = inSize - sizeof(symbol256);

  for (; i < endInSize256; i++)
  {
    const uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(symbol256, _mm256_loadu_si256((const __m256i *) & (pIn[i]))));

    if (0xFFFFFFFF == mask)
    {
      count += sizeof(symbol256);
      i += sizeof(symbol256) - 1;
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

        const int64_t range = i - lastRLE - count + 1;

        if (count >= RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT)
        {
#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
          if (range <= 255)
#else
          if (range <= 127)
#endif
          {
            const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
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
#else
            if (storedCount <= 127)
            {
              pOut[index] = (uint8_t)(storedCount << 1);
              index++;
            }
            else
            {
              *(uint32_t *)&(pOut[index]) = (uint32_t)(storedCount << 1) | (uint32_t)1;
              index += sizeof(uint32_t);
            }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
            pOut[index] = (uint8_t)range;
            index++;
#else
            pOut[index] = (uint8_t)(range << 1);
            index++;
#endif

            const size_t copySize = i - count - lastRLE;

            memcpy(pOut + index, pIn + lastRLE, copySize);
            index += copySize;

            lastRLE = i;

            wastedChances = 0;
          }
          else if (count >= RLE8_EXTREME_SINGLE_MIN_RANGE_LONG)
          {
            const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
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
#else
            if (storedCount <= 127)
            {
              pOut[index] = (uint8_t)(storedCount << 1);
              index++;
            }
            else
            {
              *(uint32_t *)&(pOut[index]) = (uint32_t)(storedCount << 1) | (uint32_t)1;
              index += sizeof(uint32_t);
            }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
            pOut[index] = 0;
            index++;
            *((uint32_t *)&pOut[index]) = (uint32_t)range;
            index += sizeof(uint32_t);
#else
            *((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | (uint32_t)1;
            index += sizeof(uint32_t);
#endif

            const size_t copySize = i - count - lastRLE;

            memcpy(pOut + index, pIn + lastRLE, copySize);
            index += copySize;

            lastRLE = i;
            wastedChances = 0;
          }
#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
          else
          {
            wastedChances++;

            if (wastedChances == 1 || i - firstWastedChanceIndex > 255)
            {
              firstWastedChanceIndex = i - count;
              wastedChances = 1;
            }
            else if (wastedChances > 2)
            {
              i = firstWastedChanceIndex;
              wastedChances = 0;
              count = 0;

              for (; i < endInSize256; i++)
                if (pIn[i] == maxFreqSymbol)
                  ++count;
                else
                  break;

              const int64_t wastedRange = i - lastRLE - count + 1;
              const int64_t storedCount = count - RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT + 1;

              pOut[index] = (uint8_t)storedCount;
              index++;

              pOut[index] = 0;
              index++;
              *((uint32_t *)&pOut[index]) = (uint32_t)wastedRange;
              index += sizeof(uint32_t);

              const size_t copySize = i - count - lastRLE;

              memcpy(pOut + index, pIn + lastRLE, copySize);
              index += copySize;

              lastRLE = i;
            }
          }
#endif
        }
      }

      count = 0;

      while (i < endInSize256)
      {
        const int32_t cmp = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i *)(&pIn[i])), symbol256));

        if (cmp == 0 || ((cmp & 0x80000000) == 0 && __builtin_popcount((uint32_t)cmp) < RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT))
        {
          i += sizeof(symbol256);
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
          count = 1;
          break;
        }
      }
    }
  }

  *pOutIndex = index;
  *pCount = count;
  *pLastRLE = lastRLE;

  return i;
}

static void CONCAT3(rle8_, CODEC, _decompress_multi_sse)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef PACKED
    symbol = _mm_set1_epi8(*pInStart);
    pInStart++;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;
    pInStart++;

    const uint8_t sameSymbol = (symbolCount & 0b10000000);
    symbolCount &= 0b01111111;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    if (!sameSymbol)
    {
      symbol = _mm_set1_epi8(*pInStart);
      pInStart++;
    }
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

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
__attribute__((target("sse4.1")))
#endif
static void CONCAT3(rle8_, CODEC, _decompress_multi_sse41)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m128i symbol = _mm_setzero_si128();

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef PACKED
    symbol = _mm_set1_epi8(*pInStart);
    pInStart++;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;
    pInStart++;

    const uint8_t sameSymbol = (symbolCount & 0b10000000);
    symbolCount &= 0b01111111;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    if (!sameSymbol)
    {
      symbol = _mm_set1_epi8(*pInStart);
      pInStart++;
    }
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_SSE41;

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
static void CONCAT3(rle8_, CODEC, _decompress_multi_avx)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef PACKED
    symbol = _mm256_set1_epi8(*pInStart);
    pInStart++;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;
    pInStart++;

    const uint8_t sameSymbol = (symbolCount & 0b10000000);
    symbolCount &= 0b01111111;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    if (!sameSymbol)
    {
      symbol = _mm256_set1_epi8(*pInStart);
      pInStart++;
    }
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_AVX;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_MULTI_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_AVX;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static void CONCAT3(rle8_, CODEC, _decompress_multi_avx2)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m256i symbol = _mm256_setzero_si256();

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef PACKED
    symbol = _mm256_set1_epi8(*pInStart);
    pInStart++;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;
    pInStart++;

    const uint8_t sameSymbol = (symbolCount & 0b10000000);
    symbolCount &= 0b01111111;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    if (!sameSymbol)
    {
      symbol = _mm256_set1_epi8(*pInStart);
      pInStart++;
    }
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_AVX2;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_MULTI_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_AVX2;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx512f")))
#endif
static void CONCAT3(rle8_, CODEC, _decompress_multi_avx512f)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
  size_t offset, symbolCount;
  __m512i symbol = _mm512_setzero_si512();

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _multi_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef PACKED
    symbol = _mm512_set1_epi8(*pInStart);
    pInStart++;
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;
    pInStart++;

    const uint8_t sameSymbol = (symbolCount & 0b10000000);
    symbolCount &= 0b01111111;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }

    if (!sameSymbol)
    {
      symbol = _mm512_set1_epi8(*pInStart);
      pInStart++;
    }
#endif

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_AVX512;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_MULTI_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_AVX512;
  }
}

static void CONCAT3(rle8_, CODEC, _decompress_single_sse)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t singleSymbol)
{
  size_t offset, symbolCount;
  const __m128i symbol = _mm_set1_epi8(singleSymbol);

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;

    if (symbolCount & 1)
    {
      symbolCount = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);
    }
    else
    {
      pInStart++;
      symbolCount >>= 1;
    }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

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
__attribute__((target("sse4.1")))
#endif
static void CONCAT3(rle8_, CODEC, _decompress_single_sse41)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t singleSymbol)
{
  size_t offset, symbolCount;
  const __m128i symbol = _mm_set1_epi8(singleSymbol);

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;

    if (symbolCount & 1)
    {
      symbolCount = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);
    }
    else
    {
      pInStart++;
      symbolCount >>= 1;
    }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_SSE41;

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
static void CONCAT3(rle8_, CODEC, _decompress_single_avx)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t singleSymbol)
{
  size_t offset, symbolCount;
  const __m256i symbol = _mm256_set1_epi8(singleSymbol);

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;

    if (symbolCount & 1)
    {
      symbolCount = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);
    }
    else
    {
      pInStart++;
      symbolCount >>= 1;
    }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_AVX;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_AVX;
  }
}

#ifndef _MSC_VER
__attribute__((target("avx2")))
#endif
static void CONCAT3(rle8_, CODEC, _decompress_single_avx2)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t singleSymbol)
{
  size_t offset, symbolCount;
  const __m256i symbol = _mm256_set1_epi8(singleSymbol);

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;

    if (symbolCount & 1)
    {
      symbolCount = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);
    }
    else
    {
      pInStart++;
      symbolCount >>= 1;
    }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_AVX2;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_AVX;
  }
}


#ifndef _MSC_VER
__attribute__((target("avx512f")))
#endif
static void CONCAT3(rle8_, CODEC, _decompress_single_avx512f)(IN const uint8_t *pInStart, OUT uint8_t *pOut, const uint8_t singleSymbol)
{
  size_t offset, symbolCount;
  const __m512i symbol = _mm512_set1_epi8(singleSymbol);

  while (true)
  {
#ifdef _DEBUG
    CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *pSymbol = (CONCAT3(rle8_, CODEC, _single_symbol_debug_t) *)pInStart;
    (void)pSymbol;
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_RLE
    symbolCount = (size_t)*pInStart;
    pInStart++;

    if (symbolCount == 0)
    {
      symbolCount = *(uint32_t *)pInStart;
      pInStart += sizeof(uint32_t);
    }
#else
    symbolCount = (size_t)*pInStart;

    if (symbolCount & 1)
    {
      symbolCount = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);
    }
    else
    {
      pInStart++;
      symbolCount >>= 1;
    }
#endif

#ifndef SINGLE_PREFER_7_BIT_OR_4_BYTE_COPY
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
#else
    offset = (size_t)*pInStart;

    if (offset & 1)
    {
      offset = (*(uint32_t *)pInStart) >> 1;
      pInStart += sizeof(uint32_t);

      if (offset == 0)
        return;
    }
    else
    {
      pInStart++;
      offset >>= 1;
    }

    offset--;
#endif

    // memcpy.
    MEMCPY_AVX512;

    if (!symbolCount)
      return;

    symbolCount += (RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT - 1);

    // memset.
    MEMSET_AVX512;
  }
}

//////////////////////////////////////////////////////////////////////////

#undef RLE8_EXTREME_MULTI_SIZE_OF_SYMBOL_HEADER
#undef RLE8_EXTREME_MULTI_MAX_SIZE_OF_SYMBOL_HEADER
#undef RLE8_EXTREME_MULTI_MIN_RANGE_SHORT
#undef RLE8_EXTREME_MULTI_MIN_RANGE_LONG

#undef RLE8_EXTREME_SINGLE_SIZE_OF_SYMBOL_HEADER
#undef RLE8_EXTREME_SINGLE_MAX_SIZE_OF_SYMBOL_HEADER
#undef RLE8_EXTREME_SINGLE_MIN_RANGE_SHORT
#undef RLE8_EXTREME_SINGLE_MIN_RANGE_LONG

#undef RLE8_EXTREME_MODE_MULTI
#undef RLE8_EXTREME_MODE_SINGLE

#undef CODEC