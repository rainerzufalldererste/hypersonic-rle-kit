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

uint32_t rle8_extreme_decompress_additional_size()
{
  return 128; // just to be on the safe side.
}

#include "rle8_extreme_cpu.h"

#define PACKED

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
  #define PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle8_extreme_cpu.h"