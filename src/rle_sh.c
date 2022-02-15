#include "rle8.h"
#include "rleX_extreme_common.h"

//////////////////////////////////////////////////////////////////////////

typedef struct
{
  uint8_t *pHeaderStart, *pHeader;
  uint8_t nextBit;
  uint32_t headerSize;
} rle8_sh_header_state;

static size_t stats[255] = { 0 };

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

//////////////////////////////////////////////////////////////////////////

#define SH_LAST_RLE_SYMBOL_BITS (1)
#define SH_LAST_RLE_SYMBOL_PATTERN (0b0)
#define SH_COPY_SYMBOL_BITS (4)
#define SH_COPY_SYMBOL_PATTERN (0b0111)
#define SH_SECOND_SYMBOL_BITS (2)
#define SH_SECOND_SYMBOL_PATTERN (0b01)
#define SH_THIRD_SYMBOL_BITS (3)
#define SH_THIRD_SYMBOL_PATTERN (0b011)
#define SH_COPY_SMALL_BLOCK_BITS (6)
#define SH_COPY_SMALL_BLOCK_PATTERN (0b001111)
#define SH_COPY_LARGE_BLOCK_BITS (6)
#define SH_COPY_LARGE_BLOCK_PATTERN (0b011111)
#define SH_RLE_SMALL_BLOCK_BITS (6)
#define SH_RLE_SMALL_BLOCK_PATTERN (0b101111)
#define SH_RLE_LARGE_BLOCK_BITS (6)
#define SH_RLE_LARGE_BLOCK_PATTERN (0b111111)

#define SH_MIN_COPY_BLOCK_BYTES (7)
#define SH_MIN_RLE_BLOCK_BYTES (14)
#define SH_MIN_CHANGE_SYMBOL_BYTES (10)

uint32_t rle8_sh_bounds(const uint32_t size)
{
  return size + sizeof(uint32_t) * 3 + (size * (3 * 4 + 1)) / (8 * 4) + 1 + sizeof(uint32_t); // assuming default rle symbol dotted around every 4th byte, so our copy heuristic will attempt to encode some symbols, resulting in only every fourth byte being turned into one bit whilst 3/4 of symbols will be turned into 1 byte + 4 bit. this should result in 3 * 4 + 1 additional bits / 4 * 8 bits.
}

uint32_t rle8_sh_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle8_extreme_mmtf128_compress_bounds(inSize))
    return 0;

  uint32_t *pFileHeader = (uint32_t *)pOut;
  pFileHeader[0] = inSize;
  // pFileHeader[1] will be the compressed size.
  // pFileHeader[2] will be the offset to the header.

  pOut += sizeof(uint32_t) * 3;

  rle8_sh_header_state header;
  header.pHeader = header.pHeaderStart = pOut + outSize - 1;
  header.nextBit = 0;
  header.headerSize = 1;

  *header.pHeader = 0;

  uint8_t lastRleSymbol = 0;
  uint8_t lastSymbol = 0;
  size_t copyCount = 0;
  size_t rleChangeCount = 0;
  size_t rleCount = 0;
  size_t rleSymbolCopyCount = 0;
  uint8_t lastOccuredSymbol = 0xFF;
  uint8_t secondMostImportant = 0xFF;
  uint8_t thirdMostImportant = 0x01;
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

          if (copyCountWithoutRle > 255 + SH_MIN_COPY_BLOCK_BYTES)
          {
            rle8_sh_write_bits(&header, SH_COPY_LARGE_BLOCK_PATTERN, SH_COPY_LARGE_BLOCK_BITS);
            *(uint32_t *)pOut = (uint32_t)(copyCountWithoutRle - SH_MIN_COPY_BLOCK_BYTES);
            pOut += sizeof(uint32_t);
            memcpy(pOut, pBlockStart, copyCountWithoutRle);
            pOut += copyCountWithoutRle;
          }
          else if (copyCountWithoutRle >= SH_MIN_COPY_BLOCK_BYTES)
          {
            rle8_sh_write_bits(&header, SH_COPY_SMALL_BLOCK_PATTERN, SH_COPY_SMALL_BLOCK_BITS);
            *pOut = (uint8_t)(copyCountWithoutRle - SH_MIN_COPY_BLOCK_BYTES);
            pOut++;
            memcpy(pOut, pBlockStart, copyCountWithoutRle);
            pOut += copyCountWithoutRle;
          }
          else
          {
            for (size_t j = 0; j < copyCountWithoutRle; j++)
            {
              if (pBlockStart[j] == lastRleSymbol)
              {
                rle8_sh_write_bits(&header, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
              }
              else
              {
                rle8_sh_write_bits(&header, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

                *pOut = pBlockStart[j];
                pOut++;
              }
            }
          }

          pBlockStart += copyCount;
          copyCount = 0;
          lastRleSymbol = lastSymbol;

          rle8_sh_write_bits(&header, SH_RLE_LARGE_BLOCK_PATTERN, SH_RLE_LARGE_BLOCK_BITS);
          *(uint32_t *)pOut = (uint32_t)(rleChangeCount - SH_MIN_RLE_BLOCK_BYTES);
          pOut += sizeof(uint32_t);
          *pOut = lastSymbol;
          pOut++;
        }

        rleCount = 1;
        rleSymbolCopyCount++;
        rleChangeCount = 0;
        lastWasSame = true;
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
            for (size_t j = 0; j < copyCount; j++)
            {
              if (pBlockStart[j] == lastRleSymbol)
              {
                rle8_sh_write_bits(&header, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
              }
              else
              {
                const uint8_t sym = pBlockStart[j];

                if (sym == secondMostImportant)
                {
                  rle8_sh_write_bits(&header, SH_SECOND_SYMBOL_PATTERN, SH_SECOND_SYMBOL_BITS);

                  lastOccuredSymbol = sym;
                }
                else if (sym == thirdMostImportant)
                {
                  rle8_sh_write_bits(&header, SH_THIRD_SYMBOL_PATTERN, SH_THIRD_SYMBOL_BITS);

                  lastOccuredSymbol = sym;
                }
                else
                {
                  rle8_sh_write_bits(&header, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

                  if (sym == lastOccuredSymbol)
                  {
                    thirdMostImportant = secondMostImportant;
                    secondMostImportant = sym;
                  }

                  lastOccuredSymbol = sym;

                  *pOut = sym;
                  pOut++;
                }
              }
            }
          }
          else
          {
            if (copyCount > 255 + SH_MIN_COPY_BLOCK_BYTES)
            {
              rle8_sh_write_bits(&header, SH_COPY_LARGE_BLOCK_PATTERN, SH_COPY_LARGE_BLOCK_BITS);
              *(uint32_t *)pOut = (uint32_t)(copyCount - SH_MIN_COPY_BLOCK_BYTES);
              pOut += sizeof(uint32_t);
              memcpy(pOut, pBlockStart, copyCount);
              pOut += copyCount;
            }
            else if (copyCount >= SH_MIN_COPY_BLOCK_BYTES)
            {
              rle8_sh_write_bits(&header, SH_COPY_SMALL_BLOCK_PATTERN, SH_COPY_SMALL_BLOCK_BITS);
              *pOut = (uint8_t)(copyCount - SH_MIN_COPY_BLOCK_BYTES);
              pOut++;
              memcpy(pOut, pBlockStart, copyCount);
              pOut += copyCount;
            }
            else
            {
              for (size_t j = 0; j < copyCount; j++)
              {
                if (pBlockStart[j] == lastRleSymbol)
                {
                  rle8_sh_write_bits(&header, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
                }
                else
                {
                  const uint8_t sym = pBlockStart[j];

                  if (sym == secondMostImportant)
                  {
                    rle8_sh_write_bits(&header, SH_SECOND_SYMBOL_PATTERN, SH_SECOND_SYMBOL_BITS);

                    lastOccuredSymbol = sym;
                  }
                  else if (sym == thirdMostImportant)
                  {
                    rle8_sh_write_bits(&header, SH_THIRD_SYMBOL_PATTERN, SH_THIRD_SYMBOL_BITS);

                    lastOccuredSymbol = sym;
                  }
                  else
                  {
                    rle8_sh_write_bits(&header, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

                    if (sym == lastOccuredSymbol)
                    {
                      thirdMostImportant = secondMostImportant;
                      secondMostImportant = sym;
                    }

                    lastOccuredSymbol = sym;

                    *pOut = sym;
                    pOut++;
                  }
                }
              }
            }
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
            for (size_t j = 0; j < copyCountWithoutRle; j++)
            {
              if (pBlockStart[j] == lastRleSymbol)
              {
                rle8_sh_write_bits(&header, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
              }
              else
              {
                const uint8_t sym = pBlockStart[j];

                if (sym == secondMostImportant)
                {
                  rle8_sh_write_bits(&header, SH_SECOND_SYMBOL_PATTERN, SH_SECOND_SYMBOL_BITS);

                  lastOccuredSymbol = sym;
                }
                else if (sym == thirdMostImportant)
                {
                  rle8_sh_write_bits(&header, SH_THIRD_SYMBOL_PATTERN, SH_THIRD_SYMBOL_BITS);

                  lastOccuredSymbol = sym;
                }
                else
                {
                  rle8_sh_write_bits(&header, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

                  if (sym == lastOccuredSymbol)
                  {
                    thirdMostImportant = secondMostImportant;
                    secondMostImportant = sym;
                  }

                  lastOccuredSymbol = sym;

                  *pOut = sym;
                  pOut++;
                }
              }
            }
          }
          else
          {
            if (copyCountWithoutRle > 255 + SH_MIN_COPY_BLOCK_BYTES)
            {
              rle8_sh_write_bits(&header, SH_COPY_LARGE_BLOCK_PATTERN, SH_COPY_LARGE_BLOCK_BITS);
              *(uint32_t *)pOut = (uint32_t)(copyCountWithoutRle - SH_MIN_COPY_BLOCK_BYTES);
              pOut += sizeof(uint32_t);
              memcpy(pOut, pBlockStart, copyCountWithoutRle);
              pOut += copyCountWithoutRle;
            }
            else if (copyCountWithoutRle >= SH_MIN_COPY_BLOCK_BYTES)
            {
              rle8_sh_write_bits(&header, SH_COPY_SMALL_BLOCK_PATTERN, SH_COPY_SMALL_BLOCK_BITS);
              *pOut = (uint8_t)(copyCountWithoutRle - SH_MIN_COPY_BLOCK_BYTES);
              pOut++;
              memcpy(pOut, pBlockStart, copyCountWithoutRle);
              pOut += copyCountWithoutRle;
            }
            else
            {
              for (size_t j = 0; j < copyCountWithoutRle; j++)
              {
                if (pBlockStart[j] == lastRleSymbol)
                {
                  rle8_sh_write_bits(&header, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
                }
                else
                {
                  const uint8_t sym = pBlockStart[j];

                  if (sym == secondMostImportant)
                  {
                    rle8_sh_write_bits(&header, SH_SECOND_SYMBOL_PATTERN, SH_SECOND_SYMBOL_BITS);

                    lastOccuredSymbol = sym;
                  }
                  else if (sym == thirdMostImportant)
                  {
                    rle8_sh_write_bits(&header, SH_THIRD_SYMBOL_PATTERN, SH_THIRD_SYMBOL_BITS);

                    lastOccuredSymbol = sym;
                  }
                  else
                  {
                    rle8_sh_write_bits(&header, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

                    if (sym == lastOccuredSymbol)
                    {
                      thirdMostImportant = secondMostImportant;
                      secondMostImportant = sym;
                    }

                    lastOccuredSymbol = sym;

                    *pOut = sym;
                    pOut++;
                  }
                }
              }
            }
          }

          pBlockStart += copyCount;
          copyCount = 0;
          rleSymbolCopyCount = 0;
          lastRleSymbol = lastSymbol;

          rle8_sh_write_bits(&header, SH_RLE_LARGE_BLOCK_PATTERN, SH_RLE_LARGE_BLOCK_BITS);
          *(uint32_t *)pOut = (uint32_t)(rleChangeCount - SH_MIN_RLE_BLOCK_BYTES);
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
          pOut += sizeof(uint32_t);
          *pOut = lastRleSymbol;
          pOut++;
        }
        else if (rleCount >= SH_MIN_RLE_BLOCK_BYTES)
        {
          rle8_sh_write_bits(&header, SH_RLE_SMALL_BLOCK_PATTERN, SH_RLE_SMALL_BLOCK_BITS);
          *pOut = (uint8_t)(rleCount - SH_MIN_RLE_BLOCK_BYTES);
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

    if (copyCount > 255 + SH_MIN_COPY_BLOCK_BYTES)
    {
      rle8_sh_write_bits(&header, SH_COPY_LARGE_BLOCK_PATTERN, SH_COPY_LARGE_BLOCK_BITS);
      *(uint32_t *)pOut = (uint32_t)(copyCount - SH_MIN_COPY_BLOCK_BYTES);
      pOut += sizeof(uint32_t);
      memcpy(pOut, pBlockStart, copyCount);
      pOut += copyCount;
    }
    else if (copyCount >= SH_MIN_COPY_BLOCK_BYTES)
    {
      rle8_sh_write_bits(&header, SH_COPY_SMALL_BLOCK_PATTERN, SH_COPY_SMALL_BLOCK_BITS);
      *pOut = (uint8_t)(copyCount - SH_MIN_COPY_BLOCK_BYTES);
      pOut++;
      memcpy(pOut, pBlockStart, copyCount);
      pOut += copyCount;
    }
    else
    {
      for (size_t j = 0; j < copyCount; j++)
      {
        if (pBlockStart[j] == lastRleSymbol)
        {
          rle8_sh_write_bits(&header, SH_LAST_RLE_SYMBOL_PATTERN, SH_LAST_RLE_SYMBOL_BITS);
        }
        else
        {
          const uint8_t sym = pBlockStart[j];

          if (sym == secondMostImportant)
          {
            rle8_sh_write_bits(&header, SH_SECOND_SYMBOL_PATTERN, SH_SECOND_SYMBOL_BITS);

            lastOccuredSymbol = sym;
          }
          else if (sym == thirdMostImportant)
          {
            rle8_sh_write_bits(&header, SH_THIRD_SYMBOL_PATTERN, SH_THIRD_SYMBOL_BITS);

            lastOccuredSymbol = sym;
          }
          else
          {
            rle8_sh_write_bits(&header, SH_COPY_SYMBOL_PATTERN, SH_COPY_SYMBOL_BITS);

            if (sym == lastOccuredSymbol)
            {
              thirdMostImportant = secondMostImportant;
              secondMostImportant = sym;
            }

            lastOccuredSymbol = sym;

            *pOut = sym;
            pOut++;
          }
        }
      }

      stats[lastOccuredSymbol]++;
    }
  }
  else
  {
    if (rleCount > 255 + SH_MIN_RLE_BLOCK_BYTES)
    {
      rle8_sh_write_bits(&header, SH_RLE_LARGE_BLOCK_PATTERN, SH_RLE_LARGE_BLOCK_BITS);
      *(uint32_t *)pOut = (uint32_t)(rleCount - SH_MIN_RLE_BLOCK_BYTES);
      pOut += sizeof(uint32_t);
      *pOut = lastRleSymbol;
      pOut++;
    }
    else if (rleCount >= SH_MIN_RLE_BLOCK_BYTES)
    {
      rle8_sh_write_bits(&header, SH_RLE_SMALL_BLOCK_PATTERN, SH_RLE_SMALL_BLOCK_BITS);
      *pOut = (uint8_t)(rleCount - SH_MIN_RLE_BLOCK_BYTES);
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

  pFileHeader[2] = (uint32_t)(pOut - (uint8_t *)pFileHeader);

  memmove(pOut, header.pHeader, header.headerSize);

  pFileHeader[1] = pFileHeader[2] + header.headerSize;

  return pFileHeader[1];
}

uint32_t rle8_sh_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  (void)pIn;
  (void)inSize;
  (void)pOut;
  (void)outSize;

  return 0;
}