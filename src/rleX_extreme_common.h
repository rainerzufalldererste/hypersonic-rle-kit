#ifndef rleX_extreme_common_h__
#define rleX_extreme_common_h__

#include "simd_platform.h"

//////////////////////////////////////////////////////////////////////////

#define _CONCAT_LITERALS3(a, b, c) a ## b ## c
#define CONCAT_LITERALS3(a, b, c) _CONCAT_LITERALS3(a, b, c)
#define CONCAT3(a, b, c) CONCAT_LITERALS3(a, b, c)

#define _CONCAT_LITERALS2(a, b) a ## b
#define CONCAT_LITERALS2(a, b) _CONCAT_LITERALS2(a, b)
#define CONCAT2(a, b) CONCAT_LITERALS2(a, b)

//////////////////////////////////////////////////////////////////////////

//#define PREFER_UNALIGNED
#define PREFER_STREAM

#define SSE_PREFETCH_BYTES 128
#define AVX_PREFETCH_BYTES 256
#define AVX512_PREFETCH_BYTES 512

#define PREFETCH_TYPE _MM_HINT_T0

#define MULTI(a) {a}
#define MULTI_LARGE(a) {a}

#ifdef PREFER_STREAM

#define MEMCPY_SSE \
if (offset <= sizeof(__m128i)) \
{ _mm_storeu_si128((__m128i *)pOut, _mm_loadu_si128((__m128i *)pInStart)); \
  pOut += offset; \
  pInStart += offset; \
} \
else \
{ size_t unaligned = ((size_t)pInStart & (sizeof(__m128i) - 1)); \
  const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm_storeu_si128((__m128i *)pCOut, _mm_loadu_si128((__m128i *)pCIn)); \
    pCIn = (uint8_t *)((size_t)pCIn & ~(size_t)(sizeof(__m128i) - 1)) + sizeof(__m128i); \
    pCOut += (pCIn - pInStart); \
  } \
\
  pOut += offset; \
  pInStart += offset; \
\
  while (pCOut < pOut) \
  { MULTI(_mm_storeu_si128((__m128i *)pCOut, _mm_load_si128((__m128i *)pCIn)); \
    pCIn += sizeof(__m128i); \
    pCOut += sizeof(__m128i);) \
    _mm_prefetch((const char *)pCIn + SSE_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
}

#define MEMCPY_SSE41 \
if (offset <= sizeof(__m128i)) \
{ _mm_storeu_si128((__m128i *)pOut, _mm_loadu_si128((__m128i *)pInStart)); \
  pOut += offset; \
  pInStart += offset; \
} \
else \
{ size_t unaligned = ((size_t)pInStart & (sizeof(__m128i) - 1)); \
  const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm_storeu_si128((__m128i *)pCOut, _mm_loadu_si128((__m128i *)pCIn)); \
    pCIn = (uint8_t *)((size_t)pCIn & ~(size_t)(sizeof(__m128i) - 1)) + sizeof(__m128i); \
    pCOut += (pCIn - pInStart); \
  } \
\
  pOut += offset; \
  pInStart += offset; \
\
  while (pCOut < pOut) \
  { MULTI(_mm_storeu_si128((__m128i *)pCOut, _mm_stream_load_si128((__m128i *)pCIn)); \
    pCIn += sizeof(__m128i); \
    pCOut += sizeof(__m128i);) \
    _mm_prefetch((const char *)pCIn + SSE_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
}

#define MEMCPY_AVX \
if (offset <= sizeof(__m256i)) \
{ _mm256_storeu_si256((__m256i *)pOut, _mm256_loadu_si256((__m256i *)pInStart)); \
  pOut += offset; \
  pInStart += offset; \
} \
else \
{ size_t unaligned = ((size_t)pInStart & (sizeof(__m256i) - 1)); \
  const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm256_storeu_si256((__m256i *)pCOut, _mm256_loadu_si256((__m256i *)pCIn)); \
    pCIn = (uint8_t *)((size_t)pCIn & ~(size_t)(sizeof(__m256i) - 1)) + sizeof(__m256i); \
    pCOut += (pCIn - pInStart); \
  } \
\
  pOut += offset; \
  pInStart += offset; \
\
  while (pCOut < pOut) \
  { MULTI(_mm256_storeu_si256((__m256i *)pCOut, _mm256_load_si256((__m256i *)pCIn)); \
    pCIn += sizeof(__m256i); \
    pCOut += sizeof(__m256i);) \
    _mm_prefetch((const char *)pCIn + AVX_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
}

#define MEMCPY_AVX2 \
if (offset <= sizeof(__m256i)) \
{ _mm256_storeu_si256((__m256i *)pOut, _mm256_loadu_si256((__m256i *)pInStart)); \
  pOut += offset; \
  pInStart += offset; \
} \
else \
{ size_t unaligned = ((size_t)pInStart & (sizeof(__m256i) - 1)); \
  const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm256_storeu_si256((__m256i *)pCOut, _mm256_loadu_si256((__m256i *)pCIn)); \
    pCIn = (uint8_t *)((size_t)pCIn & ~(size_t)(sizeof(__m256i) - 1)) + sizeof(__m256i); \
    pCOut += (pCIn - pInStart); \
  } \
\
  pOut += offset; \
  pInStart += offset; \
\
  while (pCOut < pOut) \
  { MULTI(_mm256_storeu_si256((__m256i *)pCOut, _mm256_stream_load_si256((__m256i *)pCIn)); \
    pCIn += sizeof(__m256i); \
    pCOut += sizeof(__m256i);) \
    _mm_prefetch((const char *)pCIn + AVX_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
}

#define MEMCPY_AVX512 \
if (offset <= sizeof(__m512i)) \
{ _mm512_storeu_si512((__m512i *)pOut, _mm512_loadu_si512((__m512i *)pInStart)); \
  pOut += offset; \
  pInStart += offset; \
} \
else \
{ size_t unaligned = ((size_t)pInStart & (sizeof(__m512i) - 1)); \
  const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm512_storeu_si512((__m512i *)pCOut, _mm512_loadu_si512((__m512i *)pCIn)); \
    pCIn = (uint8_t *)((size_t)pCIn & ~(size_t)(sizeof(__m512i) - 1)) + sizeof(__m512i); \
    pCOut += (pCIn - pInStart); \
  } \
\
  pOut += offset; \
  pInStart += offset; \
\
  while (pCOut < pOut) \
  { MULTI(_mm512_storeu_si512((__m512i *)pCOut, _mm512_stream_load_si512((__m512i *)pCIn)); \
    pCIn += sizeof(__m512i); \
    pCOut += sizeof(__m512i);) \
    _mm_prefetch((const char *)pCIn + AVX512_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
}

#define MEMSET_SSE \
if (symbolCount <= sizeof(__m128i)) \
{ _mm_storeu_si128((__m128i *)pOut, symbol); \
  pOut += symbolCount; \
} \
else \
{ \
  size_t unaligned = ((size_t)pOut & (sizeof(__m128i) - 1)); \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm_storeu_si128((__m128i *)pCOut, symbol); \
    pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m128i) - 1)) + sizeof(__m128i); \
  } \
\
  pOut += symbolCount; \
\
  while (pCOut < pOut) \
  { MULTI(_mm_store_si128((__m128i *)pCOut, symbol); \
    pCOut += sizeof(__m128i);) \
  } \
}

#define MEMSET_AVX \
if (symbolCount <= sizeof(__m256i)) \
{ _mm256_storeu_si256((__m256i *)pOut, symbol); \
  pOut += symbolCount; \
} \
else \
{ size_t unaligned = ((size_t)pOut & (sizeof(__m256i) - 1)); \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm256_storeu_si256((__m256i *)pCOut, symbol); \
    pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m256i) - 1)) + sizeof(__m256i); \
  } \
\
  pOut += symbolCount; \
\
  while (pCOut < pOut) \
  { MULTI(_mm256_store_si256((__m256i *)pCOut, symbol); \
    pCOut += sizeof(__m256i);) \
  } \
}

#define MEMSET_AVX2 MEMSET_AVX

/*#define MEMSET_AVX2 \
if (symbolCount <= sizeof(__m256i)) \
{ _mm256_storeu_si256((__m256i *)pOut, symbol); \
  pOut += symbolCount; \
} \
else \
{ size_t unaligned = ((size_t)pOut & (sizeof(__m256i) - 1)); \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm256_storeu_si256((__m256i *)pCOut, symbol); \
    pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m256i) - 1)) + sizeof(__m256i); \
  } \
\
  pOut += symbolCount; \
\
  if (symbolCount >= 64) \
  { while (pCOut < pOut) \
    { MULTI(_mm256_stream_si256((__m256i *)pCOut, symbol); \
      pCOut += sizeof(__m256i);) \
    } \
  } \
  else \
  { while (pCOut < pOut) \
    { MULTI(_mm256_store_si256((__m256i *)pCOut, symbol); \
      pCOut += sizeof(__m256i);) \
    } \
  } \
}*/

#define MEMSET_AVX512 \
if (symbolCount <= sizeof(__m512i)) \
{ _mm512_storeu_si512((__m512i *)pOut, symbol); \
  pOut += symbolCount; \
} \
else \
{ size_t unaligned = ((size_t)pOut & (sizeof(__m512i) - 1)); \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm512_storeu_si512((__m512i *)pCOut, symbol); \
    pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m512i) - 1)) + sizeof(__m512i); \
  } \
\
  pOut += symbolCount; \
\
  while (pCOut < pOut) \
  { MULTI(_mm512_store_si512((__m512i *)pCOut, symbol); \
    pCOut += sizeof(__m512i);) \
  } \
}

#define MEMSET_SSE_MULTI \
{ \
  uint8_t *pCOut = pOut; \
  uint8_t *pCOutEnd = pOut + symbolCount; \
\
  while (pCOut < pCOutEnd) \
  { MULTI(_mm_storeu_si128((__m128i *)pCOut, symbol); \
    pCOut += sizeof(symbol);) \
  } \
\
  pOut = pCOutEnd; \
}

#define MEMSET_AVX_MULTI \
{ uint8_t *pCOut = pOut; \
  uint8_t *pCOutEnd = pOut + symbolCount; \
\
  while (pCOut < pCOutEnd) \
  { MULTI(_mm256_storeu_si256((__m256i *)pCOut, symbol); \
    pCOut += sizeof(symbol);) \
  } \
\
  pOut = pCOutEnd; \
}

#define MEMSET_AVX512_MULTI \
{ uint8_t *pCOut = pOut; \
  uint8_t *pCOutEnd = pOut + symbolCount; \
\
  while (pCOut < pCOutEnd) \
  { MULTI(_mm512_storeu_si512((__m512i *)pCOut, symbol); \
    pCOut += sizeof(symbol);) \
  } \
\
  pOut = pCOutEnd; \
}

#define MEMCPY_SSE_MULTI MEMCPY_SSE
#define MEMCPY_SSE41_MULTI MEMCPY_SSE41
#define MEMCPY_AVX_MULTI MEMCPY_AVX
#define MEMCPY_AVX2_MULTI MEMCPY_AVX2
#define MEMCPY_AVX512_MULTI MEMCPY_AVX // <- I believe this is because AVX-512 memcpy was actually slower than AVX.

#elif !defined(PREFER_UNALIGNED)
#define MEMCPY_SSE \
if (offset <= sizeof(__m128i)) \
{ _mm_storeu_si128((__m128i *)pOut, _mm_loadu_si128((__m128i *)pInStart)); \
  pOut += offset; \
  pInStart += offset; \
} \
else \
{ size_t unaligned = ((size_t)pInStart & (sizeof(__m128i) - 1)); \
  const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm_storeu_si128((__m128i *)pCOut, _mm_loadu_si128((__m128i *)pCIn)); \
    pCIn = (uint8_t *)((size_t)pCIn & ~(size_t)(sizeof(__m128i) - 1)) + sizeof(__m128i); \
    pCOut += (pCIn - pInStart); \
  } \
\
  pOut += offset; \
  pInStart += offset; \
\
  while (pCOut < pOut) \
  { MULTI(_mm_storeu_si128((__m128i *)pCOut, _mm_load_si128((__m128i *)pCIn)); \
    pCIn += sizeof(__m128i); \
    pCOut += sizeof(__m128i);) \
    _mm_prefetch((const char *)pCIn + SSE_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
}

#define MEMSET_SSE \
if (symbolCount <= sizeof(__m128i)) \
{ _mm_storeu_si128((__m128i *)pOut, symbol); \
  pOut += symbolCount; \
} \
else \
{ \
  size_t unaligned = ((size_t)pOut & (sizeof(__m128i) - 1)); \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm_storeu_si128((__m128i *)pCOut, symbol); \
    pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m128i) - 1)) + sizeof(__m128i); \
  } \
\
  pOut += symbolCount; \
\
  while (pCOut < pOut) \
  { MULTI(_mm_store_si128((__m128i *)pCOut, symbol); \
    pCOut += sizeof(__m128i);) \
  } \
}

#define MEMCPY_AVX \
if (offset <= sizeof(__m256i)) \
{ _mm256_storeu_si256((__m256i *)pOut, _mm256_loadu_si256((__m256i *)pInStart)); \
  pOut += offset; \
  pInStart += offset; \
} \
else \
{ size_t unaligned = ((size_t)pInStart & (sizeof(__m256i) - 1)); \
  const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm256_storeu_si256((__m256i *)pCOut, _mm256_loadu_si256((__m256i *)pCIn)); \
    pCIn = (uint8_t *)((size_t)pCIn & ~(size_t)(sizeof(__m256i) - 1)) + sizeof(__m256i); \
    pCOut += (pCIn - pInStart); \
  } \
\
  pOut += offset; \
  pInStart += offset; \
\
  while (pCOut < pOut) \
  { MULTI(_mm256_storeu_si256((__m256i *)pCOut, _mm256_load_si256((__m256i *)pCIn)); \
    pCIn += sizeof(__m256i); \
    pCOut += sizeof(__m256i);) \
    _mm_prefetch((const char *)pCIn + AVX_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
}

#define MEMCPY_AVX512 \
if (offset <= sizeof(__m512i)) \
{ _mm512_storeu_si512((__m512i *)pOut, _mm512_loadu_si512((__m512i *)pInStart)); \
  pOut += offset; \
  pInStart += offset; \
} \
else \
{ size_t unaligned = ((size_t)pInStart & (sizeof(__m512i) - 1)); \
  const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm512_storeu_si512((__m512i *)pCOut, _mm512_loadu_si512((__m512i *)pCIn)); \
    pCIn = (uint8_t *)((size_t)pCIn & ~(size_t)(sizeof(__m512i) - 1)) + sizeof(__m512i); \
    pCOut += (pCIn - pInStart); \
  } \
\
  pOut += offset; \
  pInStart += offset; \
\
  while (pCOut < pOut) \
  { MULTI(_mm512_storeu_si512((__m512i *)pCOut, _mm512_load_si512((__m512i *)pCIn)); \
    pCIn += sizeof(__m512i); \
    pCOut += sizeof(__m512i);) \
    _mm_prefetch((const char *)pCIn + AVX512_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
}

#define MEMSET_AVX \
if (symbolCount <= sizeof(__m256i)) \
{ _mm256_storeu_si256((__m256i *)pOut, symbol); \
  pOut += symbolCount; \
} \
else \
{ size_t unaligned = ((size_t)pOut & (sizeof(__m256i) - 1)); \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm256_storeu_si256((__m256i *)pCOut, symbol); \
    pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m256i) - 1)) + sizeof(__m256i); \
  } \
\
  pOut += symbolCount; \
\
  while (pCOut < pOut) \
  { MULTI(_mm256_store_si256((__m256i *)pCOut, symbol); \
    pCOut += sizeof(__m256i);) \
  } \
}

#define MEMSET_AVX512 \
if (symbolCount <= sizeof(__m512i)) \
{ _mm512_storeu_si512((__m512i *)pOut, symbol); \
  pOut += symbolCount; \
} \
else \
{ size_t unaligned = ((size_t)pOut & (sizeof(__m512i) - 1)); \
  uint8_t *pCOut = pOut; \
\
  if (unaligned != 0) \
  { _mm512_storeu_si512((__m512i *)pCOut, symbol); \
    pCOut = (uint8_t *)((size_t)pCOut & ~(size_t)(sizeof(__m512i) - 1)) + sizeof(__m512i); \
  } \
\
  pOut += symbolCount; \
\
  while (pCOut < pOut) \
  { MULTI(_mm512_store_si512((__m512i *)pCOut, symbol); \
    pCOut += sizeof(__m512i);) \
  } \
}

#define MEMSET_SSE_MULTI \
{ \
  uint8_t *pCOut = pOut; \
  uint8_t *pCOutEnd = pOut + symbolCount; \
\
  while (pCOut < pCOutEnd) \
  { MULTI(_mm_storeu_si128((__m128i *)pCOut, symbol); \
    pCOut += sizeof(symbol);) \
  } \
\
  pOut = pCOutEnd; \
}

#define MEMSET_AVX_MULTI \
{ uint8_t *pCOut = pOut; \
  uint8_t *pCOutEnd = pOut + symbolCount; \
\
  while (pCOut < pCOutEnd) \
  { MULTI(_mm256_storeu_si256((__m256i *)pCOut, symbol); \
    pCOut += sizeof(symbol);) \
  } \
\
  pOut = pCOutEnd; \
}

#define MEMSET_AVX512_MULTI \
{ uint8_t *pCOut = pOut; \
  uint8_t *pCOutEnd = pOut + symbolCount; \
\
  while (pCOut < pCOutEnd) \
  { MULTI(_mm512_storeu_si512((__m512i *)pCOut, symbol); \
    pCOut += sizeof(symbol);) \
  } \
\
  pOut = pCOutEnd; \
}

#define MEMCPY_SSE_MULTI MEMCPY_SSE
#define MEMCPY_AVX_MULTI MEMCPY_AVX
#define MEMCPY_AVX512_MULTI MEMCPY_AVX // <- I believe this is because AVX-512 memcpy was actually slower than AVX.

#else
#define MEMCPY_SSE \
{ const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
  const uint8_t *pCInEnd = pInStart + offset; \
\
  while (pCIn < pCInEnd) \
  { MULTI(_mm_storeu_si128((__m128i *)pCOut, _mm_loadu_si128((__m128i *)pCIn)); \
    pCIn += sizeof(__m128i); \
    pCOut += sizeof(__m128i);) \
    _mm_prefetch((const char *)pCIn + SSE_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
\
  pOut += offset; \
  pInStart = pCInEnd; \
}

#define MEMSET_SSE \
{ \
  uint8_t *pCOut = pOut; \
  uint8_t *pCOutEnd = pOut + symbolCount; \
\
  while (pCOut < pCOutEnd) \
  { MULTI(_mm_storeu_si128((__m128i *)pCOut, symbol); \
    pCOut += sizeof(__m128i);) \
  } \
\
  pOut = pCOutEnd; \
}

#define MEMCPY_AVX \
{ const uint8_t *pCIn = pInStart; \
  uint8_t *pCOut = pOut; \
  const uint8_t *pCInEnd = pInStart + offset; \
\
  while (pCIn < pCInEnd) \
  { MULTI(_mm256_storeu_si256((__m256i *)pCOut, _mm256_loadu_si256((__m256i *)pCIn)); \
    pCIn += sizeof(__m256i); \
    pCOut += sizeof(__m256i);) \
    _mm_prefetch((const char *)pCIn + AVX_PREFETCH_BYTES, PREFETCH_TYPE); \
  } \
\
  pOut += offset; \
  pInStart = pCInEnd; \
}

#define MEMSET_AVX \
{ uint8_t *pCOut = pOut; \
  uint8_t *pCOutEnd = pOut + symbolCount; \
\
  while (pCOut < pCOutEnd) \
  { MULTI(_mm256_storeu_si256((__m256i *)pCOut, symbol); \
    pCOut += sizeof(__m256i);) \
  } \
\
  pOut = pCOutEnd; \
}

#define MEMSET_SSE_MULTI MEMSET_SSE
#define MEMSET_AVX_MULTI MEMSET_AVX
#define MEMCPY_SSE_MULTI MEMCPY_SSE
#define MEMCPY_AVX_MULTI MEMCPY_AVX

#endif

#endif // rleX_extreme_common_h__
