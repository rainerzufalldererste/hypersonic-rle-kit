#include "rle.h"
#include "rleX_extreme_common.h"

#ifndef _MSC_VER
#define inline
#endif

//////////////////////////////////////////////////////////////////////////

typedef struct
{
  uint8_t *pHeaderStart, *pHeader;
  uint8_t nextBit;
  uint32_t headerSize;
} rle8_sh_header_state;

#define rle8_write_to_hist0(...) 
#define rle8_validate_from_hist0(...) 

inline void rle8_sh_write_bits(rle8_sh_header_state *pHeader, const uint8_t bits, const uint8_t count)
{
  uint8_t value = bits;

  for (uint8_t i = 0; i < count; i++)
  {
    *pHeader->pHeader |= (value & 0b1) << pHeader->nextBit;

    value = value >> 1;
    pHeader->nextBit = ((pHeader->nextBit + 1) & 7);

    if (!pHeader->nextBit)
    {
      pHeader->pHeader--;
      *pHeader->pHeader = 0;
      pHeader->headerSize++;
    }
  }
}

typedef struct
{
  uint16_t value;
  int8_t remainingBits;
  const uint8_t *pHeader;
} rle8_sh_read_header;

inline void rle8_sh_consume_bits(rle8_sh_read_header *pHeader, const int8_t bits)
{
  pHeader->remainingBits -= bits;
  pHeader->value >>= bits;

  if (pHeader->remainingBits <= 8)
  {
    pHeader->value |= ((uint16_t)*pHeader->pHeader << pHeader->remainingBits);
    pHeader->remainingBits += 8;
    pHeader->pHeader--;
  }
}

//////////////////////////////////////////////////////////////////////////

#define SH_LAST_RLE_SYMBOL_BITS (1)
#define SH_LAST_RLE_SYMBOL_PATTERN (0b0)
#define SH_COPY_SYMBOL_BITS (2)
#define SH_COPY_SYMBOL_PATTERN (0b01)
#define SH_SECOND_SYMBOL_BITS (3)
#define SH_SECOND_SYMBOL_PATTERN (0b011)
#define SH_THIRD_SYMBOL_BITS (4)
#define SH_THIRD_SYMBOL_PATTERN (0b0111)
#define SH_ENCODED_COPY_THIRD_SYMBOL_BITS (3)
#define SH_ENCODED_COPY_THIRD_SYMBOL_PATTERN (0b111)
#define SH_RLE_SMALL_BLOCK_BITS (5)
#define SH_RLE_SMALL_BLOCK_PATTERN (0b01111)
#define SH_COPY_SMALL_BLOCK_BITS (7)
#define SH_COPY_SMALL_BLOCK_PATTERN (0b0011111)
#define SH_COPY_LARGE_BLOCK_BITS (7)
#define SH_COPY_LARGE_BLOCK_PATTERN (0b0111111)
#define SH_RLE_LARGE_BLOCK_BITS (7)
#define SH_RLE_LARGE_BLOCK_PATTERN (0b1011111)
#define SH_ENCODED_COPY_BLOCK_BITS (7)
#define SH_ENCODED_COPY_BLOCK_PATTERN (0b1111111)

#define SH_MIN_COPY_BLOCK_BYTES (7)
#define SH_MIN_RLE_BLOCK_BYTES (14)
#define SH_MIN_CHANGE_SYMBOL_BYTES (10)
#define SH_MIN_ENCODED_COPY_BLOCK_BYTES (161)

#define SH_INIT_RLE_SYMBOL (0x7F)
#define SH_INIT_SECOND_SYMBOL (0x80)
#define SH_INIT_THIRD_SYMBOL (0x7E)
#define SH_INIT_PREVIOUS_SYMBOL (0x80)

uint32_t rle8_sh_bounds(const uint32_t size)
{
  return size + sizeof(uint32_t) * 2 + 1 + sizeof(uint32_t);
}

inline void rle8_sh_encoded_copy(IN const uint8_t *pBlockStart, OUT uint8_t **ppOut, const size_t copyCount, rle8_sh_header_state *pHeader, const uint8_t lastRleSymbol, uint8_t *pSecondMostImportant, uint8_t *pThirdMostImportant, uint8_t *pLastOccuredSymbol)
{
  size_t remainingCopyCount = copyCount;

  while (remainingCopyCount)
  {
    if (remainingCopyCount > SH_MIN_ENCODED_COPY_BLOCK_BYTES)
    {
      rle8_sh_write_bits(pHeader, SH_ENCODED_COPY_BLOCK_PATTERN, SH_ENCODED_COPY_BLOCK_BITS);

      const size_t consumedCopyCountStoredValue = min(0xFF, remainingCopyCount - SH_MIN_ENCODED_COPY_BLOCK_BYTES);
      const size_t consumedCopyCount = consumedCopyCountStoredValue + SH_MIN_ENCODED_COPY_BLOCK_BYTES;

      **ppOut = (uint8_t)consumedCopyCountStoredValue;
      (*ppOut)++;
      rle8_write_to_hist0(consumedCopyCount);

      for (size_t i = 0; i < consumedCopyCount; i++)
      {
        if (pBlockStart[i] == lastRleSymbol)
        {
          rle8_sh_write_bits(pHeader, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
        }
        else
        {
          const uint8_t sym = pBlockStart[i];

          if (sym == *pSecondMostImportant)
          {
            rle8_sh_write_bits(pHeader, SH_SECOND_SYMBOL_PATTERN, SH_SECOND_SYMBOL_BITS);

            *pLastOccuredSymbol = sym;
          }
          else if (sym == *pThirdMostImportant)
          {
            rle8_sh_write_bits(pHeader, SH_ENCODED_COPY_THIRD_SYMBOL_PATTERN, SH_ENCODED_COPY_THIRD_SYMBOL_BITS);

            *pLastOccuredSymbol = sym;
          }
          else
          {
            rle8_sh_write_bits(pHeader, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

            if (sym == *pLastOccuredSymbol)
            {
              *pThirdMostImportant = *pSecondMostImportant;
              *pSecondMostImportant = sym;
            }

            *pLastOccuredSymbol = sym;

            **ppOut = sym;
            (*ppOut)++;
          }
        }
      }

      remainingCopyCount -= consumedCopyCount;
      pBlockStart += consumedCopyCount;
    }
    else
    {
      for (size_t i = 0; i < remainingCopyCount; i++)
      {
        if (pBlockStart[i] == lastRleSymbol)
        {
          rle8_sh_write_bits(pHeader, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
        }
        else
        {
          const uint8_t sym = pBlockStart[i];

          if (sym == *pSecondMostImportant)
          {
            rle8_sh_write_bits(pHeader, SH_SECOND_SYMBOL_PATTERN, SH_SECOND_SYMBOL_BITS);

            *pLastOccuredSymbol = sym;
          }
          else if (sym == *pThirdMostImportant)
          {
            rle8_sh_write_bits(pHeader, SH_THIRD_SYMBOL_PATTERN, SH_THIRD_SYMBOL_BITS);

            *pLastOccuredSymbol = sym;
          }
          else
          {
            rle8_sh_write_bits(pHeader, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

            if (sym == *pLastOccuredSymbol)
            {
              *pThirdMostImportant = *pSecondMostImportant;
              *pSecondMostImportant = sym;
            }

            *pLastOccuredSymbol = sym;

            **ppOut = sym;
            (*ppOut)++;
          }
        }
      }

      return;
    }
  }
}

inline void rle8_sh_copy(IN const uint8_t *pBlockStart, OUT uint8_t **ppOut, const size_t copyCount, rle8_sh_header_state *pHeader, const uint8_t lastRleSymbol, uint8_t *pSecondMostImportant, uint8_t *pThirdMostImportant, uint8_t *pLastOccuredSymbol)
{
  if (copyCount > 255 + SH_MIN_COPY_BLOCK_BYTES)
  {
    rle8_sh_write_bits(pHeader, SH_COPY_LARGE_BLOCK_PATTERN, SH_COPY_LARGE_BLOCK_BITS);
    *(uint32_t *)*ppOut = (uint32_t)(copyCount - SH_MIN_COPY_BLOCK_BYTES);
    rle8_write_to_hist0(copyCount);
    *ppOut += sizeof(uint32_t);
    memcpy(*ppOut, pBlockStart, copyCount);
    *ppOut += copyCount;
  }
  else if (copyCount >= SH_MIN_COPY_BLOCK_BYTES)
  {
    rle8_sh_write_bits(pHeader, SH_COPY_SMALL_BLOCK_PATTERN, SH_COPY_SMALL_BLOCK_BITS);
    **ppOut = (uint8_t)(copyCount - SH_MIN_COPY_BLOCK_BYTES);
    rle8_write_to_hist0(copyCount);
    (*ppOut)++;
    memcpy(*ppOut, pBlockStart, copyCount);
    *ppOut += copyCount;
  }
  else
  {
    for (size_t i = 0; i < copyCount; i++)
    {
      if (pBlockStart[i] == lastRleSymbol)
      {
        rle8_sh_write_bits(pHeader, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
      }
      else
      {
        const uint8_t sym = pBlockStart[i];

        if (sym == *pSecondMostImportant)
        {
          rle8_sh_write_bits(pHeader, SH_SECOND_SYMBOL_PATTERN, SH_SECOND_SYMBOL_BITS);

          *pLastOccuredSymbol = sym;
        }
        else if (sym == *pThirdMostImportant)
        {
          rle8_sh_write_bits(pHeader, SH_THIRD_SYMBOL_PATTERN, SH_THIRD_SYMBOL_BITS);

          *pLastOccuredSymbol = sym;
        }
        else
        {
          rle8_sh_write_bits(pHeader, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

          if (sym == *pLastOccuredSymbol)
          {
            *pThirdMostImportant = *pSecondMostImportant;
            *pSecondMostImportant = sym;
          }

          *pLastOccuredSymbol = sym;

          **ppOut = sym;
          (*ppOut)++;
        }
      }
    }
  }
}

uint32_t rle8_sh_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_mmtf128_compress_bounds(inSize))
    return 0;

  uint32_t *pFileHeader = (uint32_t *)pOut;
  pFileHeader[0] = inSize;
  // pFileHeader[1] will be the compressed size.

  pOut += sizeof(uint32_t) * 2;

  rle8_sh_header_state header;
  header.pHeader = header.pHeaderStart = (uint32_t *)((uint8_t *)pFileHeader + outSize - 1);
  header.nextBit = 0;
  header.headerSize = 1;

  *header.pHeader = 0;

  uint8_t lastRleSymbol = SH_INIT_RLE_SYMBOL;
  uint8_t lastOccuredSymbol = SH_INIT_PREVIOUS_SYMBOL;
  uint8_t secondMostImportant = SH_INIT_SECOND_SYMBOL;
  uint8_t thirdMostImportant = SH_INIT_THIRD_SYMBOL;
  uint8_t lastSymbol = 0;
  size_t copyCount = 0;
  size_t rleChangeCount = 0;
  size_t rleCount = 0;
  size_t rleSymbolCopyCount = 0;
  bool lastWasSame = false;
  bool lastWasCopy = false;
  const uint8_t *pBlockStart = pIn;

  for (size_t i = 0; i < inSize; i++)
  {
    const uint8_t symbol = pIn[i];

    if (symbol == lastRleSymbol)
    {
      if (!lastWasSame)
      {
        if (rleChangeCount >= SH_MIN_CHANGE_SYMBOL_BYTES)
        {
          const size_t copyCountWithoutRle = copyCount - rleChangeCount;

          rle8_sh_copy(pBlockStart, &pOut, copyCountWithoutRle, &header, lastRleSymbol, &secondMostImportant, &thirdMostImportant, &lastOccuredSymbol);

          pBlockStart += copyCount;
          copyCount = 0;
          lastRleSymbol = lastSymbol;

          rle8_sh_write_bits(&header, SH_RLE_LARGE_BLOCK_PATTERN, SH_RLE_LARGE_BLOCK_BITS);
          *(uint32_t *)pOut = (uint32_t)(rleChangeCount - SH_MIN_RLE_BLOCK_BYTES);
          pOut += sizeof(uint32_t);
          rle8_write_to_hist0(rleChangeCount);
          *pOut = lastSymbol;
          pOut++;

          copyCount = 1;
          rleSymbolCopyCount = 0;
          rleCount = 0;
          lastWasSame = false;
          rleChangeCount = 1;
        }
        else
        {
          rleCount = 1;
          rleSymbolCopyCount++;
          lastWasSame = true;
          rleChangeCount = 0;
        }

        lastSymbol = symbol;
      }
      else
      {
        rleCount++;
        rleSymbolCopyCount++;

        if (rleCount > SH_MIN_RLE_BLOCK_BYTES)
        {
          const size_t realRleCopyCount = rleSymbolCopyCount - rleCount;

          if (realRleCopyCount * 7 > (copyCount - realRleCopyCount) * 2)
          {
            rle8_sh_encoded_copy(pBlockStart, &pOut, copyCount, &header, lastRleSymbol, &secondMostImportant, &thirdMostImportant, &lastOccuredSymbol);
          }
          else
          {
            rle8_sh_copy(pBlockStart, &pOut, copyCount, &header, lastRleSymbol, &secondMostImportant, &thirdMostImportant, &lastOccuredSymbol);
          }

          pBlockStart += copyCount;
          copyCount = 0;
          rleSymbolCopyCount = 0;
          lastWasSame = true;
          lastWasCopy = false;
          lastSymbol = symbol;
        }
      }
    }
    else
    {
      if (lastWasSame && lastWasCopy)
      {
        lastWasSame = false;
        copyCount += rleCount;
        rleCount = 0;
      }

      if (symbol == lastSymbol)
      {
        rleChangeCount++;
      }
      else
      {
        if (rleChangeCount >= SH_MIN_CHANGE_SYMBOL_BYTES)
        {
          const size_t copyCountWithoutRle = copyCount - rleChangeCount;

          if (rleSymbolCopyCount * 7 > (copyCountWithoutRle - rleSymbolCopyCount) * 2)
          {
            rle8_sh_encoded_copy(pBlockStart, &pOut, copyCountWithoutRle, &header, lastRleSymbol, &secondMostImportant, &thirdMostImportant, &lastOccuredSymbol);
          }
          else
          {
            rle8_sh_copy(pBlockStart, &pOut, copyCountWithoutRle, &header, lastRleSymbol, &secondMostImportant, &thirdMostImportant, &lastOccuredSymbol);
          }

          pBlockStart += copyCount;
          copyCount = 0;
          rleSymbolCopyCount = 0;
          lastRleSymbol = lastSymbol;

          rle8_sh_write_bits(&header, SH_RLE_LARGE_BLOCK_PATTERN, SH_RLE_LARGE_BLOCK_BITS);
          *(uint32_t *)pOut = (uint32_t)(rleChangeCount - SH_MIN_RLE_BLOCK_BYTES);
          rle8_write_to_hist0(rleChangeCount);
          pOut += sizeof(uint32_t);
          *pOut = lastSymbol;
          pOut++;
        }

        rleChangeCount = 1;
        lastSymbol = symbol;
      }

      if (!lastWasCopy)
      {
        if (rleCount > 255 + SH_MIN_RLE_BLOCK_BYTES)
        {
          rle8_sh_write_bits(&header, SH_RLE_LARGE_BLOCK_PATTERN, SH_RLE_LARGE_BLOCK_BITS);
          *(uint32_t *)pOut = (uint32_t)(rleCount - SH_MIN_RLE_BLOCK_BYTES);
          rle8_write_to_hist0(rleCount);
          pOut += sizeof(uint32_t);
          *pOut = lastRleSymbol;
          pOut++;
        }
        else if (rleCount >= SH_MIN_RLE_BLOCK_BYTES)
        {
          rle8_sh_write_bits(&header, SH_RLE_SMALL_BLOCK_PATTERN, SH_RLE_SMALL_BLOCK_BITS);
          *pOut = (uint8_t)(rleCount - SH_MIN_RLE_BLOCK_BYTES);
          rle8_write_to_hist0(rleCount);
          pOut++;
        }
        else
        {
          for (size_t j = 0; j < rleCount; j++)
            rle8_sh_write_bits(&header, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
        }

        pBlockStart = pIn + i;
        copyCount = 1;
        rleCount = 0;
        rleSymbolCopyCount = 0;
        lastWasSame = false;
        lastWasCopy = true;
      }
      else
      {
        copyCount++;
      }
    }
  }

  if (lastWasCopy)
  {
    if (lastWasSame && lastWasCopy)
    {
      lastWasSame = false;
      copyCount += rleCount;
      rleCount = 0;
    }

    rle8_sh_copy(pBlockStart, &pOut, copyCount, &header, lastRleSymbol, &secondMostImportant, &thirdMostImportant, &lastOccuredSymbol);
  }
  else
  {
    if (rleCount > 255 + SH_MIN_RLE_BLOCK_BYTES)
    {
      rle8_sh_write_bits(&header, SH_RLE_LARGE_BLOCK_PATTERN, SH_RLE_LARGE_BLOCK_BITS);
      *(uint32_t *)pOut = (uint32_t)(rleCount - SH_MIN_RLE_BLOCK_BYTES);
      rle8_write_to_hist0(rleCount);
      pOut += sizeof(uint32_t);
      *pOut = lastRleSymbol;
      pOut++;
    }
    else if (rleCount >= SH_MIN_RLE_BLOCK_BYTES)
    {
      rle8_sh_write_bits(&header, SH_RLE_SMALL_BLOCK_PATTERN, SH_RLE_SMALL_BLOCK_BITS);
      *pOut = (uint8_t)(rleCount - SH_MIN_RLE_BLOCK_BYTES);
      rle8_write_to_hist0(rleCount);
      pOut++;
    }
    else
    {
      for (size_t j = 0; j < rleCount; j++)
        rle8_sh_write_bits(&header, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
    }
  }

  rle8_sh_write_bits(&header, SH_COPY_LARGE_BLOCK_PATTERN, SH_COPY_LARGE_BLOCK_BITS);
  *(uint32_t *)pOut = (uint32_t)0;
  rle8_write_to_hist0(0);
  pOut += sizeof(uint32_t);

  const size_t headerOffset = (uint32_t)(pOut - (uint8_t *)pFileHeader);

  if (header.nextBit == 0)
  {
    header.headerSize--;
    header.pHeader++;
  }

  memmove(pOut, header.pHeader, header.headerSize);

  pFileHeader[1] = (uint32_t)(headerOffset + header.headerSize);

  return pFileHeader[1];
}

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_sh_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const uint32_t expectedOutSize = ((uint32_t *)pIn)[0];
  const uint32_t expectedInSize = ((uint32_t *)pIn)[1];

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  rle8_sh_read_header header;
  header.pHeader = pIn + expectedInSize - 1;
  header.value = *header.pHeader;
  header.remainingBits = 8;
  header.pHeader--;

  uint8_t lastRleSymbol = SH_INIT_RLE_SYMBOL;
  uint8_t lastOccuredSymbol = SH_INIT_PREVIOUS_SYMBOL;
  uint8_t secondMostImportant = SH_INIT_SECOND_SYMBOL;
  uint8_t thirdMostImportant = SH_INIT_THIRD_SYMBOL;

  pIn += sizeof(uint32_t) * 2;

  while (true)
  {
    if (!(header.value & 0b1))
    {
      *pOut = lastRleSymbol;
      pOut++;
      rle8_sh_consume_bits(&header, 1);
    }
    else if (!(header.value & 0b10))
    {
      rle8_sh_consume_bits(&header, 2);

      const uint8_t sym = *pIn;

      if (sym == lastOccuredSymbol)
      {
        thirdMostImportant = secondMostImportant;
        secondMostImportant = sym;
      }

      lastOccuredSymbol = sym;

      *pOut = sym;
      pOut++;
      pIn++;
    }
    else if (!(header.value & 0b100))
    {
      lastOccuredSymbol = secondMostImportant;

      *pOut = secondMostImportant;
      pOut++;
      rle8_sh_consume_bits(&header, 3);
    }
    else if (!(header.value & 0b1000))
    {
      lastOccuredSymbol = thirdMostImportant;

      *pOut = thirdMostImportant;
      pOut++;
      rle8_sh_consume_bits(&header, 4);
    }
    else if (!(header.value & 0b10000)) // SH_RLE_SMALL_BLOCK_PATTERN
    {
      rle8_sh_consume_bits(&header, 5);

      const size_t count = *pIn + SH_MIN_RLE_BLOCK_BYTES;
      rle8_validate_from_hist0(count);
      pIn++;

      memset(pOut, lastRleSymbol, count);

      pOut += count;
    }
    else
    {
      const uint8_t value = (header.value >> 5) & 0b11;
      rle8_sh_consume_bits(&header, 7);

      switch (value)
      {
      case 0: // SH_COPY_SMALL_BLOCK_PATTERN (0b0011111)
      {
        const size_t count = *pIn + SH_MIN_COPY_BLOCK_BYTES;
        rle8_validate_from_hist0(count);
        pIn++;

        memcpy(pOut, pIn, count);

        pIn += count;
        pOut += count;

        break;
      }

      case 1: // SH_COPY_LARGE_BLOCK_PATTERN (0b0111111)
      {
        size_t count = *(uint32_t *)pIn;
        pIn += sizeof(uint32_t);

        if (count == 0)
          return expectedOutSize;

        count += SH_MIN_COPY_BLOCK_BYTES;
        rle8_validate_from_hist0(count);

        memcpy(pOut, pIn, count);

        pIn += count;
        pOut += count;

        break;
      }

      case 2: // SH_RLE_LARGE_BLOCK_PATTERN (0b1011111)
      {
        const size_t count = *(uint32_t *)pIn + SH_MIN_RLE_BLOCK_BYTES;
        rle8_validate_from_hist0(count);
        pIn += sizeof(uint32_t);
        lastRleSymbol = *pIn;
        pIn++;

        memset(pOut, lastRleSymbol, count);

        pOut += count;

        break;
      }

      case 3: // SH_ENCODED_COPY_BLOCK_BITS (0b1111111)
      {
        const size_t count = *pIn + SH_MIN_ENCODED_COPY_BLOCK_BYTES;
        rle8_validate_from_hist0(count);
        pIn++;

        for (size_t i = 0; i < count; i++)
        {
          if (!(header.value & 0b1))
          {
            *pOut = lastRleSymbol;
            pOut++;
            rle8_sh_consume_bits(&header, 1);
          }
          else if (!(header.value & 0b10))
          {
            rle8_sh_consume_bits(&header, 2);

            const uint8_t sym = *pIn;

            if (sym == lastOccuredSymbol)
            {
              thirdMostImportant = secondMostImportant;
              secondMostImportant = sym;
            }

            lastOccuredSymbol = sym;

            *pOut = sym;
            pOut++;
            pIn++;
          }
          else if (!(header.value & 0b100))
          {
            lastOccuredSymbol = secondMostImportant;

            *pOut = secondMostImportant;
            pOut++;
            rle8_sh_consume_bits(&header, 3);
          }
          else
          {
            lastOccuredSymbol = thirdMostImportant;

            *pOut = thirdMostImportant;
            pOut++;
            rle8_sh_consume_bits(&header, 3);
          }
        }

        break;
      }
      }
    }
  }
}
