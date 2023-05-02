#ifndef rle8_h__
#define rle8_h__

#include <stdint.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdio.h>
#include <memory.h>

#ifndef __cplusplus
  #include <stdbool.h>
#endif

#ifndef IN
  #define IN
#endif

#ifndef OUT
  #define OUT
#endif

#ifndef OPTIONAL
  #define OPTIONAL
#endif

#ifndef min
  #define min(a, b) ((a < b) ? (a) : (b))
#endif

#ifndef max
  #define max(a, b) ((a > b) ? (a) : (b))
#endif

#ifdef _MSC_VER
  #define ALIGN(a) __declspec(align(a))
#else
  #define ALIGN(a) __attribute__((aligned(a)))
  
  #ifndef _STATIC_ASSERT
    #define _STATIC_ASSERT(expr) typedef char __static_assert_t[(expr) != 0]
  #endif
#endif

#ifndef ARRAYSIZE
  #define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t rle8_low_entropy_compress_bounds(const uint32_t inSize);
uint32_t rle8_low_entropy_decompressed_size(IN const uint8_t *pIn, const uint32_t inSize);
uint32_t rle8_low_entropy_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_low_entropy_compress_only_max_frequency(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_low_entropy_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle8m_compress_bounds(const uint32_t subSections, const uint32_t inSize);
uint32_t rle8m_compress(const uint32_t subSections, IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8m_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

typedef struct rle8_low_entropy_compress_info_t
{
  bool rle[256];
  uint8_t symbolsByProb[256];
  uint8_t symbolCount;
} rle8_low_entropy_compress_info_t;

bool rle8_low_entropy_get_compress_info(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_low_entropy_compress_info_t *pCompressInfo);
bool rle8_low_entropy_get_compress_info_only_max_frequency(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_low_entropy_compress_info_t *pCompressInfo);
uint32_t rle8_low_entropy_write_compress_info(IN rle8_low_entropy_compress_info_t *pCompressInfo, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_low_entropy_compress_with_info(IN const uint8_t *pIn, const uint32_t inSize, IN const rle8_low_entropy_compress_info_t *pCompressInfo, OUT uint8_t *pOut, const uint32_t outSize);

typedef struct rle8_low_entropy_decompress_info_t
{
  bool rle[256];
  uint8_t symbolToCount[256];
} rle8_low_entropy_decompress_info_t;

uint32_t rle8_low_entropy_read_decompress_info(IN const uint8_t *pIn, const uint32_t inSize, OUT rle8_low_entropy_decompress_info_t *pDecompressInfo);
uint32_t rle8_low_entropy_decompress_with_info(IN const uint8_t *pIn, IN const uint8_t *pEnd, IN const rle8_low_entropy_decompress_info_t *pDecompressInfo, OUT uint8_t *pOut, const uint32_t expectedOutSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_low_entropy_short_compress_bounds(const uint32_t inSize);
uint32_t rle8_low_entropy_short_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_low_entropy_short_compress_only_max_frequency(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_low_entropy_short_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle8_low_entropy_short_compress_with_info(IN const uint8_t *pIn, const uint32_t inSize, IN const rle8_low_entropy_compress_info_t *pCompressInfo, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_low_entropy_short_decompress_with_info(IN const uint8_t *pIn, IN const uint8_t *pEnd, IN const rle8_low_entropy_decompress_info_t *pDecompressInfo, OUT uint8_t *pOut, const uint32_t expectedOutSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_compress_bounds(const uint32_t inSize);
uint32_t rle8_multi_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_single_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle8_decompress_additional_size();

uint32_t rle16_sym_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle16_sym_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle32_sym_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle32_sym_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle64_sym_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle64_sym_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle16_byte_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle16_byte_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle32_byte_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle32_byte_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle64_byte_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle64_byte_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_packed_multi_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_packed_single_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle16_byte_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle16_byte_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle32_byte_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle32_byte_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle64_byte_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle64_byte_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle24_sym_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle24_sym_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle48_sym_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle48_sym_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle128_sym_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle128_sym_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle24_byte_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle24_byte_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle48_byte_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle48_byte_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle128_byte_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle128_byte_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle24_byte_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle24_byte_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle48_byte_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle48_byte_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle128_byte_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle128_byte_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle16_sym_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle16_sym_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle32_sym_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle32_sym_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

uint32_t rle64_sym_packed_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle64_sym_packed_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle_mmtf_bounds(const uint32_t inSize);

// If SSE2 is not available, both of these functions will fail and return 0.
uint32_t rle_mmtf128_encode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle_mmtf128_decode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

// If SSE2 is not available, both of these functions will fail and return 0.
uint32_t rle_mmtf256_encode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle_mmtf256_decode(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_mmtf128_compress_bounds(const uint32_t inSize);
uint32_t rle8_mmtf256_compress_bounds(const uint32_t inSize);

// If SSE2 is not available, both of these functions will fail and return 0.
uint32_t rle8_mmtf128_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_mmtf128_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

// If SSE2 is not available, both of these functions will fail and return 0.
uint32_t rle8_mmtf256_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_mmtf256_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

uint32_t rle8_sh_bounds(const uint32_t size);

uint32_t rle8_sh_compress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);
uint32_t rle8_sh_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

//////////////////////////////////////////////////////////////////////////

#ifdef BUILD_WITH_OPENCL

bool rle8m_opencl_init(const size_t inputDataSize, const size_t outputDataSize, const size_t maxSubsectionCount);
void rle8m_opencl_destroy();
uint32_t rle8m_opencl_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize);

#endif

#ifdef __cplusplus
}
#endif

#endif // rle8_h__
