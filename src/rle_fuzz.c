#include "rle.h"
#include "codec_funcs.h"
#include "simd_platform.h"

#ifndef _MSC_VER
#define __debugbreak() __builtin_trap()
#endif

// From other files:
uint64_t GetCurrentTimeTicks();
uint64_t TicksToNs(const uint64_t ticks);

typedef enum
{
  fst_random_data,
  fst_repeating_symbol,

  _fst_count,
} fuzz_section_type;

typedef enum
{
  flt_small,
  flt_medium,
  flt_16_bit_limit,

  _flt_count,
} fuzz_length_type;

enum
{
  flt_small_min_value = 1,
  flt_small_max_value = 256 + 24,

  flt_medium_min_value = 768,
  flt_medium_max_value = 8192,

  flt_16_bit_limit_min_value = (1 << 16) - 8,
  flt_16_bit_limit_max_value = (1 << 16) + 24,

  fuzz_max_symbol_length = 16, // must be divisible by 4.

  fuzz_symbol_count = 16,
};

typedef struct
{
  fuzz_section_type type;
  fuzz_length_type lengthType;
  size_t currentLength;
  size_t symbolIndex;
} fuzz_sub_state_t;

typedef struct
{
  uint8_t symbol[fuzz_symbol_count][fuzz_max_symbol_length];
  size_t symbolLength;
  bool symbolBound;
  bool iterativeMode;
  size_t relevantStateCount; // if `!iteativeMode`.
  size_t lastIteratedStateIndex; // if `iteativeMode`.
  fuzz_section_type startSectionType;
  size_t stateCount;

#ifdef _MSC_VER
  #pragma warning (push)
  #pragma warning (disable: 4200)
#endif
  fuzz_sub_state_t states[]; // trailing array.
#ifdef _MSC_VER
  #pragma warning (pop)
#endif

} fuzz_state_t;

uint32_t fuzz_read_rand_predictable()
{
  static uint64_t last = 0xF824558383B00C0CULL;

  const uint64_t oldState = last;
  last = oldState * 6364136223846793005ULL + 0xE04B41702DB21F17ULL;

  const uint32_t xorshifted = (uint32_t)(((oldState >> 18) ^ oldState) >> 27);
  const uint32_t rot = (uint32_t)(oldState >> 59);

  const uint32_t hi = (xorshifted >> rot) | (xorshifted << (uint32_t)((-(int32_t)rot) & 31));
  
  return hi;
}

#ifndef _MSC_VER
__attribute__((target("aes")))
#endif
void fuzz_read_rand(__m128i *pOut)
{
  // This function assumes that AES-NI is supported.
  
  static __m128i last[2];
  static bool initialized = false;

  if (!initialized)
  {
    last[0] = _mm_set_epi64x(GetCurrentTimeTicks(), __rdtsc());
    last[1] = _mm_set_epi64x(~__rdtsc(), ~GetCurrentTimeTicks());
  }

  const __m128i r = _mm_aesdec_si128(last[0], last[1]);

  last[0] = last[1];
  last[1] = r;

  _mm_storeu_si128(pOut, last[0]);
}

uint64_t fuzz_rand_between(const uint64_t min, const uint64_t max)
{
  uint64_t buffer[2];
  fuzz_read_rand((__m128i *)&buffer);

  return (buffer[0] % (max - min)) + min;
}

void fuzz_sub_state_reset(fuzz_sub_state_t *pSubState)
{
  pSubState->lengthType = flt_small;
  pSubState->currentLength = flt_small_min_value;
  pSubState->symbolIndex = 0;
}

void fuzz_sub_state_init(fuzz_sub_state_t *pSubState, const fuzz_section_type sectionType)
{
  pSubState->type = sectionType;

  fuzz_sub_state_reset(pSubState);
}

void fuzz_sub_state_set_random_with_same_type(fuzz_sub_state_t *pSubState)
{
  pSubState->symbolIndex = fuzz_rand_between(0, fuzz_symbol_count);
  const uint64_t discriminator = fuzz_rand_between(0, 0x100);

  if (discriminator < 0xD0)
  {
    pSubState->lengthType = flt_small;
    pSubState->currentLength = fuzz_rand_between(flt_small_min_value, flt_small_max_value);
  }
  else if (discriminator < 0xF8)
  {
    pSubState->lengthType = flt_medium;
    pSubState->currentLength = fuzz_rand_between(flt_medium_min_value, flt_medium_max_value);
  }
  else
  {
    pSubState->lengthType = flt_16_bit_limit;
    pSubState->currentLength = fuzz_rand_between(flt_16_bit_limit_min_value, flt_16_bit_limit_max_value);
  }
}

bool fuzz_sub_state_try_increment(fuzz_sub_state_t *pSubState)
{
  switch (pSubState->lengthType)
  {
  case flt_small:
  {
    pSubState->currentLength++;

    if (pSubState->currentLength > flt_small_max_value)
    {
      pSubState->lengthType = flt_medium;
      pSubState->currentLength = flt_medium_min_value;
    }

    break;
  }

  case flt_medium:
  {
    pSubState->currentLength = pSubState->currentLength * 17 / 16;

    if (pSubState->currentLength > flt_medium_max_value)
    {
      pSubState->lengthType = flt_16_bit_limit;
      pSubState->currentLength = flt_16_bit_limit_min_value;
    }

    break;
  }

  case flt_16_bit_limit:
  {
    pSubState->currentLength++;

    if (pSubState->currentLength > flt_16_bit_limit_max_value)
    {
      pSubState->lengthType++;
      pSubState->currentLength = 0;
    }

    break;
  }
  }

  return pSubState->lengthType < _flt_count;
}

bool fuzz_state_increment_sub_states(fuzz_state_t *pState)
{
  const int64_t lastIteratedState = (int64_t)min(pState->stateCount - 1, pState->lastIteratedStateIndex);

  for (int64_t i = lastIteratedState; i >= 0; i--)
  {
    if (fuzz_sub_state_try_increment(&pState->states[i]))
      return true;
    
    fuzz_sub_state_reset(&pState->states[i]);
  }

  return false;
}

bool fuzz_state_increment_sub_state_order(fuzz_state_t *pState)
{
  pState->startSectionType++;

  if (pState->startSectionType >= _fst_count)
    return false;

  for (size_t i = 0; i < pState->stateCount; i++)
    fuzz_sub_state_init(&pState->states[i], (i + pState->startSectionType) % _fst_count);

  return true;
}

bool fuzz_state_increment_symbol_alignment(fuzz_state_t *pState)
{
  if (pState->symbolBound || pState->symbolLength == 1)
    return false;

  pState->symbolBound = true;
  pState->startSectionType = fst_random_data;

  for (size_t i = 0; i < pState->stateCount; i++)
    fuzz_sub_state_init(&pState->states[i], (i + pState->startSectionType) % _fst_count);

  return true;
}

bool fuzz_state_increment_symbol_length(fuzz_state_t *pState)
{
  pState->symbolLength++;

  if (pState->symbolLength > fuzz_max_symbol_length)
    return false;

  pState->symbolBound = false;
  pState->startSectionType = fst_random_data;

  for (size_t i = 0; i < pState->stateCount; i++)
    fuzz_sub_state_init(&pState->states[i], (i + pState->startSectionType) % _fst_count);

  return true;
}

bool fuzz_state_increment(fuzz_state_t *pState)
{
  // yeah, this could be a big or, but that's way messier to read.

  if (fuzz_state_increment_sub_states(pState))
    return true;

  if (fuzz_state_increment_sub_state_order(pState))
    return true;

  if (fuzz_state_increment_symbol_alignment(pState))
    return true;

  if (fuzz_state_increment_symbol_length(pState))
    return true;

  return false;
}

bool fuzz_state_advance(fuzz_state_t *pState)
{
  if (pState->iterativeMode)
    return fuzz_state_increment(pState);

  const uint64_t discriminator = fuzz_rand_between(0, 0x10);
  const uint64_t reasonableSymbolLengths[] = { 1, 2, 3, 4, 6, 8, 16 };

  if (discriminator < 0xD)
    pState->symbolLength = reasonableSymbolLengths[fuzz_rand_between(1, sizeof(reasonableSymbolLengths) / sizeof(reasonableSymbolLengths[0]))];
  else
    pState->symbolLength = fuzz_rand_between(1, fuzz_max_symbol_length + 1);

  pState->symbolBound = !!fuzz_rand_between(0, 2);
  pState->startSectionType = fuzz_rand_between(0, _fst_count);
  pState->relevantStateCount = fuzz_rand_between(1, pState->stateCount);

  for (size_t i = 0; i < pState->relevantStateCount; i++)
  {
    fuzz_sub_state_init(&pState->states[i], (i + pState->startSectionType) % _fst_count);
    fuzz_sub_state_set_random_with_same_type(&pState->states[i]);
  }

  return true;
}

size_t fuzz_sub_state_get_required_buffer_capacity(fuzz_state_t *pState)
{
  return pState->stateCount * flt_16_bit_limit_max_value * fuzz_max_symbol_length;
}

#ifdef _MSC_VER
__declspec(noinline)
#endif
bool fuzz_create(OUT fuzz_state_t **ppFuzzState, const size_t internalStates, const bool iterative)
{
  if (ppFuzzState == NULL)
    return false;

  *ppFuzzState = malloc(sizeof(fuzz_state_t) + internalStates * sizeof(fuzz_sub_state_t));

  if (*ppFuzzState == NULL)
    return false;

  (*ppFuzzState)->relevantStateCount = (*ppFuzzState)->stateCount = internalStates;
  (*ppFuzzState)->lastIteratedStateIndex = (*ppFuzzState)->relevantStateCount - 1;
  (*ppFuzzState)->symbolLength = 1;
  (*ppFuzzState)->symbolBound = false;
  (*ppFuzzState)->startSectionType = fst_random_data;
  (*ppFuzzState)->iterativeMode = iterative;

  for (size_t i = 0; i < fuzz_symbol_count; i++)
  {
    for (size_t j = 0; j < fuzz_max_symbol_length; j += sizeof(uint32_t))
    {
      const uint32_t value = fuzz_read_rand_predictable();
      memcpy(&(*ppFuzzState)->symbol[i][j], &value, sizeof(value));
    }
  }

  for (size_t i = 0; i < (*ppFuzzState)->stateCount; i++)
    fuzz_sub_state_init(&(*ppFuzzState)->states[i], (i + (*ppFuzzState)->startSectionType) % _fst_count);

  return true;
}

void fuzz_destroy(fuzz_state_t **ppFuzzState)
{
  if (ppFuzzState == NULL || *ppFuzzState == NULL)
    return;

  free(*ppFuzzState);
  *ppFuzzState = NULL;
}

size_t fuzz_fill_buffer(fuzz_state_t *pState, uint8_t *pBuffer)
{
  uint8_t *pBufferStart = pBuffer;

  for (size_t i = 0; i < pState->relevantStateCount; i++)
  {
    switch (pState->states[i].type)
    {
    case fst_random_data:
    {
      if (pState->iterativeMode)
      {
        const int64_t length32 = pState->states[i].currentLength - sizeof(uint32_t);
        int64_t j = 0;

        for (; j <= length32; j += sizeof(uint32_t))
        {
          const uint32_t value = fuzz_read_rand_predictable();
          memcpy(pBuffer + j, &value, sizeof(value));
        }

        for (; j < (int64_t)pState->states[i].currentLength; j++)
          pBuffer[j] = (uint8_t)fuzz_read_rand_predictable();

        pBuffer += pState->states[i].currentLength;
      }
      else
      {
        __m128i value;
        const int64_t length128 = pState->states[i].currentLength - sizeof(__m128i);
        int64_t j = 0;

        for (; j <= length128; j += sizeof(uint32_t))
        {
          fuzz_read_rand(&value);
          _mm_storeu_si128((__m128i *)(pBuffer + j), value);
        }

        fuzz_read_rand(&value);
        uint8_t values8[sizeof(value)];
        _mm_storeu_si128((__m128i *)values8, value);
        size_t index = 0;

        for (; j < (int64_t)pState->states[i].currentLength; j++)
          pBuffer[j] = values8[index++];

        pBuffer += pState->states[i].currentLength;
      }

      break;
    }

    case fst_repeating_symbol:
    {
      int64_t length = pState->states[i].currentLength;

      if (pState->symbolBound)
        length *= pState->symbolLength;

      const int64_t lengthInSymbol = length - pState->symbolLength;

      int64_t j = 0;

      for (; j <= lengthInSymbol; j += pState->symbolLength)
        memcpy(pBuffer + j, pState->symbol[pState->states[i].symbolIndex], pState->symbolLength);

      if (j < length)
        memcpy(pBuffer + j, pState->symbol[pState->states[i].symbolIndex], length - j);

      pBuffer += length;

      break;
    }
    }
  }

  return pBuffer - pBufferStart;
}

#ifdef _MSC_VER
inline
#else
__attribute__((target("avx2")))
#endif
bool MemoryEquals(const uint8_t *pBufferA, const uint8_t *pBufferB, const size_t length)
{
  // This function assumes that AVX2 is supported.
  // Yes, we're _intentionally_ not early-exiting, because that's only beneficial in error cases which can be as slow as they want.

  size_t offset = 0;
  const size_t endInSimd = (size_t)max(0LL, (int64_t)length - (int64_t)sizeof(__m256));

  if (endInSimd)
  {
    const size_t endInSimd4 = (size_t)max(0LL, (int64_t)length - (int64_t)sizeof(__m256) * 4);

    __m256i cmp[4];

    cmp[0] = _mm256_setzero_si256();

    if (((size_t)pBufferA & (sizeof(__m256) - 1)) == 0 && ((size_t)pBufferB & (sizeof(__m256) - 1)) == 0) // if aligned.
    {
      if (endInSimd4)
      {
        for (size_t i = 1; i < 4; i++)
          cmp[i] = _mm256_setzero_si256();

        for (; offset < endInSimd4; offset += sizeof(__m256i) * 4)
        {
          cmp[0] = _mm256_or_si256(cmp[0], _mm256_xor_si256(_mm256_stream_load_si256((__m256i *)(pBufferA + offset)), _mm256_stream_load_si256((__m256i *)(pBufferB + offset))));
          cmp[1] = _mm256_or_si256(cmp[1], _mm256_xor_si256(_mm256_stream_load_si256((__m256i *)(pBufferA + offset + sizeof(__m256i) * 1)), _mm256_stream_load_si256((__m256i *)(pBufferB + offset + sizeof(__m256i) * 1))));
          cmp[2] = _mm256_or_si256(cmp[2], _mm256_xor_si256(_mm256_stream_load_si256((__m256i *)(pBufferA + offset + sizeof(__m256i) * 2)), _mm256_stream_load_si256((__m256i *)(pBufferB + offset + sizeof(__m256i) * 2))));
          cmp[3] = _mm256_or_si256(cmp[3], _mm256_xor_si256(_mm256_stream_load_si256((__m256i *)(pBufferA + offset + sizeof(__m256i) * 3)), _mm256_stream_load_si256((__m256i *)(pBufferB + offset + sizeof(__m256i) * 3))));
        }

        cmp[0] = _mm256_or_si256(cmp[0], cmp[1]);
        cmp[2] = _mm256_or_si256(cmp[2], cmp[3]);
        cmp[0] = _mm256_or_si256(cmp[0], cmp[2]);
      }

      for (; offset < endInSimd; offset += sizeof(__m256i))
        cmp[0] = _mm256_or_si256(cmp[0], _mm256_xor_si256(_mm256_load_si256((__m256i *)(pBufferA + offset)), _mm256_load_si256((__m256i *)(pBufferB + offset))));

      if (_mm256_movemask_epi8(cmp[0]) != 0)
        return false;
    }
    else
    {
      if (endInSimd4)
      {
        for (size_t i = 1; i < 4; i++)
          cmp[i] = _mm256_setzero_si256();

        for (; offset < endInSimd4; offset += sizeof(__m256i) * 4)
        {
          cmp[0] = _mm256_or_si256(cmp[0], _mm256_xor_si256(_mm256_loadu_si256((__m256i *)(pBufferA + offset)), _mm256_loadu_si256((__m256i *)(pBufferB + offset))));
          cmp[1] = _mm256_or_si256(cmp[1], _mm256_xor_si256(_mm256_loadu_si256((__m256i *)(pBufferA + offset + sizeof(__m256i) * 1)), _mm256_loadu_si256((__m256i *)(pBufferB + offset + sizeof(__m256i) * 1))));
          cmp[2] = _mm256_or_si256(cmp[2], _mm256_xor_si256(_mm256_loadu_si256((__m256i *)(pBufferA + offset + sizeof(__m256i) * 2)), _mm256_loadu_si256((__m256i *)(pBufferB + offset + sizeof(__m256i) * 2))));
          cmp[3] = _mm256_or_si256(cmp[3], _mm256_xor_si256(_mm256_loadu_si256((__m256i *)(pBufferA + offset + sizeof(__m256i) * 3)), _mm256_loadu_si256((__m256i *)(pBufferB + offset + sizeof(__m256i) * 3))));
        }

        cmp[0] = _mm256_or_si256(cmp[0], cmp[1]);
        cmp[2] = _mm256_or_si256(cmp[2], cmp[3]);
        cmp[0] = _mm256_or_si256(cmp[0], cmp[2]);
      }

      for (; offset < endInSimd; offset += sizeof(__m256i))
        cmp[0] = _mm256_or_si256(cmp[0], _mm256_xor_si256(_mm256_loadu_si256((__m256i *)(pBufferA + offset)), _mm256_loadu_si256((__m256i *)(pBufferB + offset))));

      if (_mm256_movemask_epi8(cmp[0]) != 0)
        return false;
    }
  }

  const size_t endIn64 = (size_t)max(0LL, (int64_t)length - (int64_t)sizeof(uint64_t));

  for (; offset < endIn64; offset += sizeof(uint64_t))
    if (*(uint64_t *)(pBufferA + offset) != *(uint64_t *)(pBufferB + offset))
      return false;

  for (; offset < length; offset++)
    if (pBufferA[offset] != pBufferB[offset])
      return false;

  return true;
}

// To be more accessible in a debugger.
static FILE *_pFuzzInputBufferFile = NULL;
static size_t _inputBufferSize = 0;
static uint8_t *_pInputBuffer = NULL;

bool fuzz(const size_t sectionCount, const bool iterative)
{
  bool result = false;
  fuzz_state_t *pState = NULL;
  uint8_t *pInputBuffer = NULL;
  uint8_t *pInputBufferCopy = NULL;
  uint8_t *pCompressed = NULL;
  uint8_t *pDecompressed = NULL;
  
  if (!fuzz_create(&pState, sectionCount, iterative))
    goto epilogue;

  const size_t inputBufferCapacity = fuzz_sub_state_get_required_buffer_capacity(pState);

  pInputBuffer = malloc(inputBufferCapacity);
  _pInputBuffer = pInputBuffer;
  
  if (pInputBuffer == NULL)
    goto epilogue;
  
  pInputBufferCopy = malloc(inputBufferCapacity);
  
  if (pInputBufferCopy == NULL)
    goto epilogue;

  pDecompressed = (uint8_t *)malloc(inputBufferCapacity + rle_decompress_additional_size());

  if (pDecompressed == NULL)
    goto epilogue;

  size_t compressedBufferSize = rle_compress_bounds((uint32_t)inputBufferCapacity);
  compressedBufferSize = max(compressedBufferSize, mmtf_bounds((uint32_t)inputBufferCapacity));
  compressedBufferSize = max(compressedBufferSize, rle8_sh_bounds((uint32_t)inputBufferCapacity));
  compressedBufferSize = max(compressedBufferSize, rle8_mmtf128_compress_bounds((uint32_t)inputBufferCapacity));
  compressedBufferSize = max(compressedBufferSize, rle8_mmtf256_compress_bounds((uint32_t)inputBufferCapacity));
  compressedBufferSize = max(compressedBufferSize, rle8_low_entropy_compress_bounds((uint32_t)inputBufferCapacity));
  compressedBufferSize = max(compressedBufferSize, rle8_low_entropy_short_compress_bounds((uint32_t)inputBufferCapacity));

  pCompressed = (uint8_t *)malloc(compressedBufferSize);

  if (pCompressed == NULL)
    goto epilogue;

  const uint64_t startTicks = GetCurrentTimeTicks();
  size_t iteration = 0;
  double fuzzedGB = 0;
  size_t fuzzedBytesSinceLastStep = 0;

  // Let's have the file already open, in case we crash and may be able to use it in the debugger.
  FILE *pFile = fopen("fuzz-failure.bin", "wb");
  _pFuzzInputBufferFile = pFile;
  
  do
  {
    if ((iteration & 255) == 0 && iteration > 0)
    {
      fuzzedGB += (fuzzedBytesSinceLastStep * MemCopy) / (double)(1024 * 1024 * 1024);
      fuzzedBytesSinceLastStep = 0;

      const uint64_t elapsedNs = TicksToNs(GetCurrentTimeTicks() - startTicks);
      printf("\33[2K\rInput %" PRIu64 ": ~%1.0fk codecs fuzzed/s, %4.2f GiB/s (%4.2f TiB total), (%" PRIu64 " byte symbols (%saligned))", iteration, (iteration * MemCopy * 1e-3) / (elapsedNs * 1e-9), fuzzedGB / (elapsedNs * 1e-9), fuzzedGB / 1024.0, pState->symbolLength, pState->symbolBound ? "" : "un");

      for (size_t i = 0; i < pState->relevantStateCount; i++)
        printf(" [%c: T%" PRIu64 "/%" PRIu64 "]", pState->states[i].type == fst_random_data ? '?' : 'X', (uint64_t)pState->states[i].lengthType, pState->states[i].currentLength);
    }

    iteration++;

    const size_t inputSize = fuzz_fill_buffer(pState, pInputBuffer);
    fuzzedBytesSinceLastStep += inputSize;
    _inputBufferSize = inputSize;

#ifdef INPUT_BUFFER_VALIDATE
    memcpy(pInputBufferCopy, pInputBuffer, inputSize);
#endif

    for (codec_t codec = 0; codec < MemCopy; codec++)
    {
      const size_t compressedSize = codecCallbacks[codec].compress_func(pInputBuffer, (uint32_t)inputSize, pCompressed, (uint32_t)compressedBufferSize);

      if (compressedSize == 0)
      {
        puts("Failed to compress!");
        __debugbreak();
      }

#ifdef INPUT_BUFFER_VALIDATE
      if (!MemoryEquals(pInputBuffer, pInputBufferCopy, inputSize))
      {
        puts("Input Buffer Corrupted!");
        __debugbreak();
      }
#endif

      // Scramble End to ensure we actually _fit_ into the size we claimed to inhabit.
      {
        const size_t end = min(compressedBufferSize, inputSize + 64);

        for (size_t i = compressedSize; i < end; i++)
          pCompressed[i] = ~pCompressed[i];
      }

      memset(pDecompressed, 0, inputSize);

      const size_t decompressedSize = codecCallbacks[codec].decompress_func(pCompressed, (uint32_t)compressedSize, pDecompressed, (uint32_t)inputSize);

      if (decompressedSize != inputSize)
      {
        puts("Decompressed to incorrect size!");
        __debugbreak();
      }

      if (!MemoryEquals(pInputBuffer, pDecompressed, (size_t)inputSize))
      {
        printf("Validation Failed for codec '%s':\n", codecNames[codec]);

        printf("Fuzzer State:\nsymbol length %" PRIu64 " bytes (%saliugned), %" PRIu64 " sub states starting with %scompressible\n", pState->symbolLength, pState->symbolBound ? "" : "un", pState->stateCount, pState->startSectionType == fst_random_data ? "un" : "");

        for (size_t i = 0; i < pState->stateCount; i++)
          printf("  %" PRIu64 ": %scompressible, length mode: %" PRIu64 ", length: %" PRIu64 " unites\n", i + 1, pState->states[i].type == fst_random_data ? "un" : "", (uint64_t)pState->states[i].lengthType, pState->states[i].currentLength);

        // Try to write input buffer to file.
        {
          puts("\nAttempting to write input buffer to `fuzz-failure.bin`...\n");

          if (pFile != NULL)
          {
            fwrite(pInputBuffer, 1, inputSize, pFile);
            fclose(pFile);
          }
        }

        for (size_t i = 0; i < inputSize; i++)
        {
          if (pInputBuffer[i] != pDecompressed[i])
          {
            printf("First invalid char at %" PRIu64 " [0x%" PRIX64 "] (0x%" PRIX8 " != 0x%" PRIX8 ").\n", i, i, pInputBuffer[i], pDecompressed[i]);

            const int64_t start = max(0, (int64_t)i - 64);
            const int64_t end = min((int64_t)inputSize, (int64_t)(i + 64));

            printf("\nContext: (%" PRIi64 " to %" PRIi64 ")\n\n   Expected:                                        |  Actual Output:\n\n", start, end);

            for (int64_t context = start; context < end; context += 16)
            {
              const int64_t context_end = min(end, context + 16);

              bool different = false;

              for (int64_t j = context; j < context_end; j++)
              {
                if (pInputBuffer[j] != pDecompressed[j])
                {
                  different = true;
                  break;
                }
              }

              if (different)
                fputs("!! ", stdout);
              else
                fputs("   ", stdout);

              for (int64_t j = context; j < context_end; j++)
                printf("%02" PRIX8 " ", pInputBuffer[j]);

              for (int64_t j = context_end; j < context + 16; j++)
                fputs("   ", stdout);

              fputs(" |  ", stdout);

              for (int64_t j = context; j < context_end; j++)
                printf("%02" PRIX8 " ", pDecompressed[j]);

              puts("");

              if (different)
              {
                fputs("   ", stdout);

                for (int64_t j = context; j < context_end; j++)
                {
                  if (pInputBuffer[j] != pDecompressed[j])
                    fputs("~~ ", stdout);
                  else
                    fputs("   ", stdout);
                }

                for (int64_t j = context_end; j < context + 16; j++)
                  fputs("   ", stdout);

                fputs("    ", stdout);

                for (int64_t j = context; j < context_end; j++)
                {
                  if (pInputBuffer[j] != pDecompressed[j])
                    fputs("~~ ", stdout);
                  else
                    fputs("   ", stdout);
                }
              }

              puts("");
            }

            break;
          }
        }

        __debugbreak();
      }
    }

  } while (fuzz_state_advance(pState));

  result = true;

epilogue:
  free(pInputBuffer);
  free(pCompressed);
  free(pDecompressed);
  fuzz_destroy(&pState);

  return result;
}
