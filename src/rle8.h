#ifndef rle8_h__
#define rle8_h__

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdio.h>
#include <memory.h>

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

uint32_t rle8_compress_bounds(const uint32_t inSize);
uint32_t rle8_decompressed_size(IN const uint8_t *pIn, const uint32_t inSize);
uint32_t rle8_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

typedef struct rle8_compress_info_t
{
  bool rle[256];
  uint8_t symbolsByProb[256];
  uint8_t symbolCount;
} rle8_compress_info_t;

bool rle8_get_compress_info(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_compress_info_t *pCompressInfo);
uint32_t rle8_write_compress_info(IN rle8_compress_info_t *pCompressInfo, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_compress_with_info(IN const uint8_t *pIn, const uint32_t inSize, IN const rle8_compress_info_t *pCompressInfo, OUT uint8_t *pOut, const uint32_t outSize);

typedef struct rle8_decompress_info_t
{
  bool rle[256];
  uint8_t symbolToCount[256];
} rle8_decompress_info_t;

uint32_t rle8_read_decompress_info(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_decompress_info_t *pDecompressInfo);
uint32_t rle8_decompress_with_info(IN const uint8_t *pIn, IN const uint8_t *pEnd, IN const rle8_decompress_info_t *pDecompressInfo, OUT uint8_t *pOut, const uint32_t expectedOutSize);

#endif // rle8_h__
