#include "rle.h"
#include "codec_funcs.h"
#include "simd_platform.h"

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
};

typedef struct
{
  fuzz_section_type type;
  fuzz_length_type lengthType;
  size_t currentLength;
} fuzz_sub_state_t;

typedef struct
{
  uint8_t symbol[fuzz_max_symbol_length];
  size_t symbolLength;
  bool symbolBound;
  fuzz_section_type startSectionType;
  size_t stateCount;
  fuzz_sub_state_t states[];
} fuzz_state_t;

inline uint32_t fuzz_read_rand_predictable()
{
  static uint64_t last = 0xF824558383B00C0CULL;

  const uint64_t oldState = last;
  last = oldState * 6364136223846793005ULL + 0xE04B41702DB21F17ULL;

  const uint32_t xorshifted = (uint32_t)(((oldState >> 18) ^ oldState) >> 27);
  const uint32_t rot = (uint32_t)(oldState >> 59);

  const uint32_t hi = (xorshifted >> rot) | (xorshifted << (uint32_t)((-(int32_t)rot) & 31));
  
  return hi;
}

void fuzz_sub_state_reset(fuzz_sub_state_t *pSubState)
{
  pSubState->lengthType = flt_small;
  pSubState->currentLength = flt_small_min_value;
}

void fuzz_sub_state_init(fuzz_sub_state_t *pSubState, const fuzz_section_type sectionType)
{
  pSubState->type = sectionType;

  fuzz_sub_state_reset(pSubState);
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
  for (int64_t i = (int64_t)pState->stateCount - 1; i >= 0; i--)
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

  if (pState->symbolLength >= fuzz_max_symbol_length)
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

size_t fuzz_sub_state_get_required_buffer_capacity(fuzz_state_t *pState)
{
  return pState->stateCount * flt_16_bit_limit_max_value * fuzz_max_symbol_length;
}

bool fuzz_create(OUT fuzz_state_t **ppFuzzState, const size_t internalStates)
{
  if (ppFuzzState == NULL)
    return false;

  *ppFuzzState = malloc(sizeof(fuzz_state_t) + internalStates * sizeof(fuzz_sub_state_t));

  if (*ppFuzzState == NULL)
    return false;

  (*ppFuzzState)->stateCount = internalStates;
  (*ppFuzzState)->symbolLength = 1;
  (*ppFuzzState)->symbolBound = false;
  (*ppFuzzState)->startSectionType = fst_random_data;

  for (size_t i = 0; i < sizeof((*ppFuzzState)->symbol); i += sizeof(uint32_t))
  {
    const uint32_t value = fuzz_read_rand_predictable();
    memcpy((*ppFuzzState)->symbol + i, &value, sizeof(value));
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

  for (size_t i = 0; i < pState->stateCount; i++)
  {
    switch (pState->states[i].type)
    {
    case fst_random_data:
    {
      const int64_t length32 = pState->states[i].currentLength - sizeof(uint32_t);
      int64_t j = 0;

      for (; j <= length32; j += sizeof(uint32_t))
      {
        const uint32_t value = fuzz_read_rand_predictable();
        memcpy(pBuffer + j, &value, sizeof(value));
      }

      for (; j < pState->states[i].currentLength; j++)
        pBuffer[j] = (uint8_t)fuzz_read_rand_predictable();

      pBuffer += pState->states[i].currentLength;

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
        memcpy(pBuffer + j, pState->symbol, pState->symbolLength);

      if (j < length)
        memcpy(pBuffer + j, pState->symbol, length - j);

      pBuffer += length;

      break;
    }
    }
  }

  return pBuffer - pBufferStart;
}

uint64_t GetCurrentTimeTicks();
uint64_t TicksToNs(const uint64_t ticks);

#ifdef _MSC_VER
inline 
#endif
bool MemoryEquals(const uint8_t *pBufferA, const uint8_t *pBufferB, const size_t length)
{
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
          cmp[0] = _mm256_or_si256(cmp[0], _mm256_xor_si256(_mm256_stream_load_si256(pBufferA + offset), _mm256_stream_load_si256(pBufferB + offset)));
          cmp[1] = _mm256_or_si256(cmp[1], _mm256_xor_si256(_mm256_stream_load_si256(pBufferA + offset + sizeof(__m256i) * 1), _mm256_stream_load_si256(pBufferB + offset + sizeof(__m256i) * 1)));
          cmp[2] = _mm256_or_si256(cmp[2], _mm256_xor_si256(_mm256_stream_load_si256(pBufferA + offset + sizeof(__m256i) * 2), _mm256_stream_load_si256(pBufferB + offset + sizeof(__m256i) * 2)));
          cmp[3] = _mm256_or_si256(cmp[3], _mm256_xor_si256(_mm256_stream_load_si256(pBufferA + offset + sizeof(__m256i) * 3), _mm256_stream_load_si256(pBufferB + offset + sizeof(__m256i) * 3)));
        }

        cmp[0] = _mm256_or_si256(cmp[0], cmp[1]);
        cmp[2] = _mm256_or_si256(cmp[2], cmp[3]);
        cmp[0] = _mm256_or_si256(cmp[0], cmp[2]);
      }

      for (; offset < endInSimd; offset += sizeof(__m256i))
        cmp[0] = _mm256_or_si256(cmp[0], _mm256_xor_si256(_mm256_load_si256(pBufferA + offset), _mm256_load_si256(pBufferB + offset)));

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
          cmp[0] = _mm256_or_si256(cmp[0], _mm256_xor_si256(_mm256_loadu_si256(pBufferA + offset), _mm256_loadu_si256(pBufferB + offset)));
          cmp[1] = _mm256_or_si256(cmp[1], _mm256_xor_si256(_mm256_loadu_si256(pBufferA + offset + sizeof(__m256i) * 1), _mm256_loadu_si256(pBufferB + offset + sizeof(__m256i) * 1)));
          cmp[2] = _mm256_or_si256(cmp[2], _mm256_xor_si256(_mm256_loadu_si256(pBufferA + offset + sizeof(__m256i) * 2), _mm256_loadu_si256(pBufferB + offset + sizeof(__m256i) * 2)));
          cmp[3] = _mm256_or_si256(cmp[3], _mm256_xor_si256(_mm256_loadu_si256(pBufferA + offset + sizeof(__m256i) * 3), _mm256_loadu_si256(pBufferB + offset + sizeof(__m256i) * 3)));
        }

        cmp[0] = _mm256_or_si256(cmp[0], cmp[1]);
        cmp[2] = _mm256_or_si256(cmp[2], cmp[3]);
        cmp[0] = _mm256_or_si256(cmp[0], cmp[2]);
      }

      for (; offset < endInSimd; offset += sizeof(__m256i))
        cmp[0] = _mm256_or_si256(cmp[0], _mm256_xor_si256(_mm256_loadu_si256(pBufferA + offset), _mm256_loadu_si256(pBufferB + offset)));

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

bool fuzz(const size_t sectionCount)
{
  bool result = false;
  fuzz_state_t *pState = NULL;
  uint8_t *pInputBuffer = NULL;
  uint8_t *pInputBufferCopy = NULL;
  uint8_t *pCompressed = NULL;
  uint8_t *pDecompressed = NULL;
  
  if (!fuzz_create(&pState, sectionCount))
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

  // Let's have the file already open, in case we crash and may be able to use it in the debugger.
  FILE *pFile = fopen("fuzz-failure.bin", "wb");
  _pFuzzInputBufferFile = pFile;
  
  do
  {
    if ((iteration & 255) == 0 && iteration > 0)
    {
      const uint64_t elapsedNs = TicksToNs(GetCurrentTimeTicks() - startTicks);
      printf("\rInput %" PRIu64 ": ~%3.0fk codecs fuzzed/s, (%" PRIu64 " byte symbols (%saligned))", iteration, (iteration * MemCopy * 1e-3) / (elapsedNs * 1e-9), pState->symbolLength, pState->symbolBound ? "" : "un");

      for (size_t i = 0; i < pState->stateCount; i++)
        printf(" [%c: T%" PRIu64 "/%" PRIu64 "]", pState->states[i].type == fst_random_data ? '?' : 'X', (uint64_t)pState->states[i].lengthType, pState->states[i].currentLength);
    }

    iteration++;

    const size_t inputSize = fuzz_fill_buffer(pState, pInputBuffer);
    _inputBufferSize = inputSize;

#ifdef INPUT_BUFFER_VALIDATE
    memcpy(pInputBufferCopy, pInputBuffer, inputSize);
#endif

    for (codec_t codec = 0; codec < MemCopy; codec++)
    {
      const size_t compressedSize = codecCallbacks[codec].compress_func(pInputBuffer, inputSize, pCompressed, compressedBufferSize);

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

      // Scramble End to ensure we actually _fit_.
      {
        const size_t end = min(compressedBufferSize, inputSize + 64);

        for (size_t i = compressedSize; i < end; i++)
          pCompressed[i] = ~pCompressed[i];
      }

      memset(pDecompressed, 0, inputSize);

      const size_t decompressedSize = codecCallbacks[codec].decompress_func(pCompressed, compressedSize, pDecompressed, inputSize);

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

  } while (fuzz_state_increment(pState));

  result = true;

epilogue:
  free(pInputBuffer);
  free(pCompressed);
  free(pDecompressed);
  fuzz_destroy(&pState);

  return result;
}
