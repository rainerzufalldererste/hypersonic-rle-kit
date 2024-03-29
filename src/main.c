#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(_MSC_VER)
#define ALIGNED_ALLOC(b, a) _aligned_malloc(a, b)
#define ALIGNED_FREE(a) _aligned_free(a)
#elif !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#define ALIGNED_ALLOC(b, a) malloc(a)
#define ALIGNED_FREE(a) free(a)
#else
#define ALIGNED_ALLOC(b, a) aligned_alloc(b, a)
#define ALIGNED_FREE(a) free(a)
#endif

#include "rle.h"
#include "simd_platform.h"
#include "codec_funcs.h"

// From fuzzer:
bool fuzz(const size_t sectionCount, const bool iterative);

const char ArgumentRuns[] = "--runs";
const char ArgumentNormal[] = "--low-entropy";
const char ArgumentSingle[] = "--single";
const char ArgumentMulti[] = "--multi";
const char ArgumentShort[] = "--short";
const char ArgumentNotShort[] = "--not-short";
const char ArgumentUltra[] = "--low-entropy-short";
const char ArgumentExtreme[] = "--extreme";
const char ArgumentExtremeSize[] = "--x-size";
const char ArgumentExtremeByteGran[] = "--byte-aligned";
const char ArgumentExtremeSymbolGran[] = "--symbol-aligned";
const char ArgumentExtremePacked[] = "--packed";
const char ArgumentExtremeNotPacked[] = "--not-packed";
const char ArgumentExtremeLutSize[] = "--lut-size";
const char ArgumentExtremeGreedy[] = "--greedy";
const char ArgumentExtremeNotGreedy[] = "--not-greedy";
const char ArgumentMinimumTime[] = "--min-time";
const char ArgumentExtremeMMTF[] = "--rle-mmtf";
const char ArgumentMMTF[] = "--mmtf";
const char ArgumentSH[] = "--sh";
const char ArgumentAnalyze[] = "--analyze";
const char ArgumentMaxSimd[] = "--max-simd";
const char ArgumentMaxSimdAVX512F[] = "avx512f";
const char ArgumentMaxSimdAVX2[] = "avx2";
const char ArgumentMaxSimdAVX[] = "avx";
const char ArgumentMaxSimdSSE42[] = "sse4.2";
const char ArgumentMaxSimdSSE41[] = "sse4.1";
const char ArgumentMaxSimdSSSE3[] = "ssse3";
const char ArgumentMaxSimdSSE3[] = "sse3";
const char ArgumentMaxSimdSSE2[] = "sse2";
const char ArgumentMaxSimdNone[] = "none";
const char ArgumentTest[] = "--test";
const char ArgumentStdDev[] = "--sd";
const char ArgumentFuzzIterative[] = "--fuzz-iterative";
const char ArgumentFuzzRandom[] = "--fuzz-random";

#ifdef _WIN32
const char ArgumentCpuCore[] = "--cpu-core";
#endif

struct
{
  bool hasMode, isModeExtreme, isModeLowEntropy, isModeSH, isModeRleMMTF, isModeMMTF;
  bool hasShortMode, isShortMode;
  bool hasSingleMode, isSingleMode;
  bool hasAlignment, isAlignmentByte;
  bool hasPackedMode, isPacked;
  bool hasGreedyMode, isGreedy;
  bool hasLutSize;
  size_t lutSize;
  bool hasBitCount;
  size_t bitCount;
} _Args;

uint64_t GetCurrentTimeTicks();
uint64_t TicksToNs(const uint64_t ticks);
void SleepNs(const uint64_t sleepNs);
bool Validate(const uint8_t *pUncompressedData, const uint8_t *pDecompressedData, const size_t fileSize);
double GetInformationRatio(const uint8_t *pData, const size_t length);
void AnalyzeData(const uint8_t *pData, const size_t size);
bool CodecMatchesArgs(const codec_t codec);

//////////////////////////////////////////////////////////////////////////

int main(int argc, char **pArgv)
{
  if (argc <= 1)
  {
    printf("Usage: hsrlekit <InputFileName>\n\n");
    printf("\t[%s <Run Count>]\n\n\t[%s <Minimum Benchmark Time in Seconds>]\n\n", ArgumentRuns, ArgumentMinimumTime);
    printf("\t(to restrict to a subset of codecs to benchmark:)\n");
    printf("\t  [%s / %s / %s / %s / %s]\n", ArgumentExtreme, ArgumentExtremeMMTF, ArgumentMMTF, ArgumentNormal, ArgumentSH);
    printf("\t  [%s / %s]\n", ArgumentExtremePacked, ArgumentExtremeNotPacked);
    printf("\t  [%s / %s]\n", ArgumentExtremeByteGran, ArgumentExtremeSymbolGran);
    printf("\t  [%s / %s]\n", ArgumentExtremeGreedy, ArgumentExtremeNotGreedy);
    printf("\t  [%s / %s]\n", ArgumentMulti, ArgumentSingle);
    printf("\t  [%s / %s]\n", ArgumentShort, ArgumentNotShort);
    printf("\t[%s 0 / 1 / 3 / 7]\n", ArgumentExtremeLutSize);
    printf("\n\t[%s %s / %s / %s / %s / %s / %s / %s / %s]\n", ArgumentMaxSimd, ArgumentMaxSimdAVX512F, ArgumentMaxSimdAVX2, ArgumentMaxSimdAVX, ArgumentMaxSimdSSE42, ArgumentMaxSimdSSE41, ArgumentMaxSimdSSSE3, ArgumentMaxSimdSSE3, ArgumentMaxSimdSSE2);
    printf("\t[%s (show std deviation)]\n", ArgumentStdDev);
    printf("\n\t[%s (fail on simgle compression/decompression/validation failure)]\n", ArgumentTest);

#ifdef _WIN32
    printf("\n\t[%s <CPU Core Index>]\n", ArgumentCpuCore);
#endif

    printf("\n\n  OR: (for debugging purposes only)\n\n");
    printf("\t%s / %s (input file name will be ignored)\n", ArgumentFuzzIterative, ArgumentFuzzRandom);
    
    return 1;
  }

  memset(&_Args, 0, sizeof(_Args));

  int32_t runs = 8;
  int32_t minSeconds = 2;
  bool analyzeFileContents = false;
  bool noDelays = false;
  bool isTestRun = false;
  bool fuzzing = false;
  bool fuzzingIterative = false;
  bool showStdDev = false;

#ifdef _WIN32
  size_t cpuCoreIndex = 0;
#endif

  // Parse Parameters.
  if (argc > 2)
  {
    size_t argIndex = 2;
    size_t argsRemaining = (size_t)argc - 2;

    while (argsRemaining)
    {
      if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentTest, sizeof(ArgumentTest)) == 0)
      {
        isTestRun = true;
        noDelays = true;
        argIndex++;
        argsRemaining--;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentStdDev, sizeof(ArgumentStdDev)) == 0)
      {
        showStdDev = true;
        argIndex++;
        argsRemaining--;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentFuzzIterative, sizeof(ArgumentFuzzIterative)) == 0)
      {
        fuzzing = true;
        fuzzingIterative = true;
        argIndex++;
        argsRemaining--;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentFuzzRandom, sizeof(ArgumentFuzzRandom)) == 0)
      {
        fuzzing = true;
        fuzzingIterative = false;
        argIndex++;
        argsRemaining--;
      }
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentMaxSimd, sizeof(ArgumentMaxSimd)) == 0)
      {
        _DetectCPUFeatures();

        do
        {
          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdAVX512F, sizeof(ArgumentMaxSimdAVX512F)) == 0)
          {
            if (!avx512FSupported)
            {
              puts("AVX512F is not supported by this platform. Aborting.");
              return 1;
            }

            // In future versions with other simd flavours better than avx512 supported, disable them here.

            break;
          }

          avx512FSupported = false;
          avx512PFSupported = false;
          avx512ERSupported = false;
          avx512CDSupported = false;
          avx512BWSupported = false;
          avx512DQSupported = false;
          avx512VLSupported = false;
          avx512IFMASupported = false;
          avx512VBMISupported = false;
          avx512VNNISupported = false;
          avx512VBMI2Supported = false;
          avx512POPCNTDQSupported = false;
          avx512BITALGSupported = false;
          avx5124VNNIWSupported = false;
          avx5124FMAPSSupported = false;

          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdAVX2, sizeof(ArgumentMaxSimdAVX2)) == 0)
          {
            if (!avx2Supported)
            {
              puts("AVX2 is not supported by this platform. Aborting.");
              return 1;
            }

            break;
          }

          avx2Supported = false;
          fma3Supported = false;

          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdAVX, sizeof(ArgumentMaxSimdAVX)) == 0)
          {
            if (!avxSupported)
            {
              puts("AVX is not supported by this platform. Aborting.");
              return 1;
            }

            break;
          }

          avxSupported = false;

          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdSSE42, sizeof(ArgumentMaxSimdSSE42)) == 0)
          {
            if (!sse42Supported)
            {
              puts("SSE4.2 is not supported by this platform. Aborting.");
              return 1;
            }

            break;
          }

          sse42Supported = false;

          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdSSE41, sizeof(ArgumentMaxSimdSSE41)) == 0)
          {
            if (!sse41Supported)
            {
              puts("SSE4.1 is not supported by this platform. Aborting.");
              return 1;
            }

            break;
          }

          sse41Supported = false;

          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdSSSE3, sizeof(ArgumentMaxSimdSSSE3)) == 0)
          {
            if (!ssse3Supported)
            {
              puts("SSSE3 is not supported by this platform. Aborting.");
              return 1;
            }

            break;
          }

          ssse3Supported = false;

          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdSSE3, sizeof(ArgumentMaxSimdSSE3)) == 0)
          {
            if (!sse3Supported)
            {
              puts("SSE3 is not supported by this platform. Aborting.");
              return 1;
            }

            break;
          }

          sse3Supported = false;

          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdSSE2, sizeof(ArgumentMaxSimdSSE2)) == 0)
          {
            if (!sse2Supported)
            {
              puts("SSE2 is not supported by this platform. Aborting.");
              return 1;
            }

            break;
          }

          sse2Supported = false;

          if (strncmp(pArgv[argIndex + 1], ArgumentMaxSimdNone, sizeof(ArgumentMaxSimdNone)) == 0)
          {
            printf("%s %s is only intended for testing purposes and will only restrict some codecs to no SIMD\n", ArgumentMaxSimd, ArgumentMaxSimdNone);

            break;
          }

          printf("Invalid SIMD Variant '%s' specified.", pArgv[argIndex + 1]);
          return 1;

        } while (false);

        argIndex += 2;
        argsRemaining -= 2;
      }
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentRuns, sizeof(ArgumentRuns)) == 0)
      {
        runs = atoi(pArgv[argIndex + 1]);

        if (runs < 0)
        {
          puts("Invalid Parameter.");
          return 1;
        }
        else if (runs == 0)
        {
          runs = 1;
          noDelays = true;
        }

        argIndex += 2;
        argsRemaining -= 2;
      }
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentMinimumTime, sizeof(ArgumentMinimumTime)) == 0)
      {
        minSeconds = atoi(pArgv[argIndex + 1]);

        if (minSeconds < 0)
        {
          puts("Invalid Parameter.");
          return 1;
        }

        argIndex += 2;
        argsRemaining -= 2;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentNormal, sizeof(ArgumentNormal)) == 0)
      {
        _Args.hasMode = true;
        _Args.isModeLowEntropy = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentSingle, sizeof(ArgumentSingle)) == 0)
      {
        if (_Args.hasSingleMode)
        {
          puts("Single mode has already been specified.");
          return 1;
        }

        _Args.hasSingleMode = true;
        _Args.isSingleMode = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentMulti, sizeof(ArgumentMulti)) == 0)
      {
        if (_Args.hasSingleMode)
        {
          puts("Single mode has already been specified.");
          return 1;
        }

        _Args.hasSingleMode = true;
        _Args.isSingleMode = false;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentShort, sizeof(ArgumentShort)) == 0)
      {
        if (_Args.hasShortMode)
        {
          puts("Short mode has already been specified.");
          return 1;
        }

        _Args.hasShortMode = true;
        _Args.isShortMode = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentNotShort, sizeof(ArgumentNotShort)) == 0)
      {
        if (_Args.hasShortMode)
        {
          puts("Short mode has already been specified.");
          return 1;
        }

        _Args.hasShortMode = true;
        _Args.isShortMode = false;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremeGreedy, sizeof(ArgumentExtremeGreedy)) == 0)
      {
        if (_Args.hasGreedyMode)
        {
          puts("Greedy mode has already been specified.");
          return 1;
        }

        _Args.hasGreedyMode = true;
        _Args.isGreedy = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremeNotGreedy, sizeof(ArgumentExtremeNotGreedy)) == 0)
      {
        if (_Args.hasGreedyMode)
        {
          puts("Greedy mode has already been specified.");
          return 1;
        }

        _Args.hasGreedyMode = true;
        _Args.isGreedy = false;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentUltra, sizeof(ArgumentUltra)) == 0)
      {
        _Args.hasMode = true;
        _Args.isModeLowEntropy = true;
        _Args.hasShortMode = true;
        _Args.isShortMode = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtreme, sizeof(ArgumentExtreme)) == 0)
      {
        _Args.hasMode = true;
        _Args.isModeExtreme = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentExtremeSize, sizeof(ArgumentExtremeSize)) == 0)
      {
        if (_Args.hasBitCount)
        {
          puts("Bit count has already been specified.");
          return 1;
        }

        _Args.hasBitCount = true;
        
        switch (pArgv[argIndex + 1][0])
        {
        case '8':
          _Args.bitCount = 8;
          break;

        case '1':
          switch (pArgv[argIndex + 1][1])
          {
          case '6':
            _Args.bitCount = 16;
            break;

          case '2':
            _Args.bitCount = 128;
            break;

          default:
            puts("Invalid Parameter.");
            return 1;
          }
          break;

        case '2':
          switch (pArgv[argIndex + 1][1])
          {
          case '4':
            _Args.bitCount = 24;
            break;

          case '5':
            _Args.bitCount = 256;
            break;

          default:
            puts("Invalid Parameter.");
            return 1;
          }
          break;

        case '3':
          _Args.bitCount = 32;
          break;

        case '4':
          _Args.bitCount = 48;
          break;

        case '6':
          _Args.bitCount = 64;
          break;

        default:
          puts("Invalid Parameter.");
          return 1;
        }

        argIndex += 2;
        argsRemaining -= 2;
      }
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentExtremeLutSize, sizeof(ArgumentExtremeLutSize)) == 0)
      {
        if (_Args.hasLutSize)
        {
          puts("Lut size has already been specified.");
          return 1;
        }

        _Args.hasLutSize = true;
        
        switch (pArgv[argIndex + 1][0])
        {
        case '0':
          _Args.lutSize = 0;
          break;
          
        case '1':
          _Args.lutSize = 1;
          break;
          
        case '3':
          _Args.lutSize = 3;
          break;
          
        case '7':
          _Args.lutSize = 7;
          break;

        default:
          puts("Invalid Parameter.");
          return 1;
        }

        argIndex += 2;
        argsRemaining -= 2;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremeByteGran, sizeof(ArgumentExtremeByteGran)) == 0)
      {
        if (_Args.hasAlignment)
        {
          puts("Alignment has already been specified.");
          return 1;
        }

        _Args.hasAlignment = true;
        _Args.isAlignmentByte = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremeSymbolGran, sizeof(ArgumentExtremeSymbolGran)) == 0)
      {
        if (_Args.hasAlignment)
        {
          puts("Alignment has already been specified.");
          return 1;
        }

        _Args.hasAlignment = true;
        _Args.isAlignmentByte = false;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremePacked, sizeof(ArgumentExtremePacked)) == 0)
      {
        if (_Args.hasPackedMode)
        {
          puts("Packed mode has already been specified.");
          return 1;
        }

        _Args.hasPackedMode = true;
        _Args.isPacked = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremeNotPacked, sizeof(ArgumentExtremeNotPacked)) == 0)
      {
        if (_Args.hasPackedMode)
        {
          puts("Packed mode has already been specified.");
          return 1;
        }

        _Args.hasPackedMode = true;
        _Args.isPacked = false;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentMMTF, sizeof(ArgumentMMTF)) == 0)
      {
        _Args.hasMode = true;
        _Args.isModeMMTF = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentSH, sizeof(ArgumentSH)) == 0)
      {
        _Args.hasMode = true;
        _Args.isModeSH = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremeMMTF, sizeof(ArgumentExtremeMMTF)) == 0)
      {
        _Args.hasMode = true;
        _Args.isModeRleMMTF = true;

        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentAnalyze, sizeof(ArgumentAnalyze)) == 0)
      {
        analyzeFileContents = true;
        argIndex += 1;
        argsRemaining -= 1;
      }
#ifdef _WIN32
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentCpuCore, sizeof(ArgumentCpuCore)) == 0)
      {
        cpuCoreIndex = strtoull(pArgv[argIndex + 1], NULL, 10);
        argIndex += 2;
        argsRemaining -= 2;
      }
#endif
      else
      {
        printf("Invalid Parameter '%s'.", pArgv[argIndex]);
        return 1;
      }
    }
  }

  const bool matchBenchmarks = _Args.hasAlignment || _Args.hasBitCount || _Args.hasGreedyMode || _Args.hasLutSize || _Args.hasMode || _Args.hasPackedMode || _Args.hasShortMode || _Args.hasSingleMode;

#ifdef _WIN32
  // For more consistent benchmarking results.
  HANDLE thread = GetCurrentThread();
  SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
  SetThreadAffinityMask(thread, (uint64_t)1 << cpuCoreIndex);
#endif

  // Validate Parameters.
  {
    if ((_Args.hasMode && _Args.isModeExtreme) && (_Args.hasSingleMode && _Args.isSingleMode) && (_Args.hasBitCount && _Args.bitCount != 8))
    {
      puts("Single Symbol in Extreme Mode is only supported for symbol size 8.");
      return 1;
    }

    if ((_Args.hasMode && _Args.isModeExtreme) && (_Args.hasBitCount && _Args.bitCount == 256))
    {
      puts("Extreme Mode doesn't support symbol size 256.");
      return 1;
    }

    if ((_Args.hasMode && (_Args.isModeMMTF || _Args.isModeRleMMTF)) && (_Args.hasBitCount && _Args.bitCount != 8 && _Args.bitCount != 16 && _Args.bitCount != 256 && _Args.bitCount != 128))
    {
      puts("MMTF Modes only supports mtf width of 8, 16, 128 or 256.");
      return 1;
    }
  }

  size_t fileSize = 0;
  uint32_t compressedBufferSize = 0;
  uint8_t *pUncompressedData = NULL;
  uint8_t *pDecompressedData = NULL;
  uint8_t *pCompressedData = NULL;

  FILE *pFile = NULL;

  if (!fuzzing)
  {
    pFile = fopen(pArgv[1], "rb");

    if (!pFile)
    {
      puts("Failed to read file.");
      goto epilogue;
    }

    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);

    if (fileSize <= 0)
    {
      puts("Invalid File size / failed to read file.");
      goto epilogue;
    }

    fseek(pFile, 0, SEEK_SET);

    compressedBufferSize = rle_compress_bounds((uint32_t)fileSize);

    compressedBufferSize = max(compressedBufferSize, mmtf_bounds((uint32_t)fileSize));
    compressedBufferSize = max(compressedBufferSize, rle8_sh_bounds((uint32_t)fileSize));
    compressedBufferSize = max(compressedBufferSize, rle8_mmtf128_compress_bounds((uint32_t)fileSize));
    compressedBufferSize = max(compressedBufferSize, rle8_mmtf256_compress_bounds((uint32_t)fileSize));
    compressedBufferSize = max(compressedBufferSize, rle8_low_entropy_compress_bounds((uint32_t)fileSize));
    compressedBufferSize = max(compressedBufferSize, rle8_low_entropy_short_compress_bounds((uint32_t)fileSize));

    pUncompressedData = (uint8_t *)ALIGNED_ALLOC(32, fileSize);
    pDecompressedData = (uint8_t *)ALIGNED_ALLOC(32, fileSize + rle_decompress_additional_size());
    pCompressedData = (uint8_t *)ALIGNED_ALLOC(32, compressedBufferSize);

    if (!pUncompressedData || !pDecompressedData || !pCompressedData)
    {
      puts("Failed to allocate memory.");
      goto epilogue;
    }

    if (fileSize != fread(pUncompressedData, 1, (size_t)fileSize, pFile))
    {
      puts("Failed to read file.");
      goto epilogue;
    }

    //////////////////////////////////////////////////////////////////////////

    if (analyzeFileContents)
      AnalyzeData(pUncompressedData, fileSize);
  }

  //////////////////////////////////////////////////////////////////////////

  if (fuzzing)
  {
    bool success;
    
    if (fuzzingIterative)
      success = fuzz(6, true);
    else
      success = fuzz(8, false);

    if (success)
      puts("Fuzzer Completed!");
    else
      puts("Fuzzer Failed!");

    return success ? 0 : 1;
  }
  else
  {
    codec_t currentCodec = 0;

    uint32_t fileSize32 = (uint32_t)fileSize;

    size_t individualRunsCount = 0;
    size_t individualRunsCapacity = 0;
    size_t *pIndividualRuns = NULL;

    printf("\nBenchmarking File '%s' (%" PRIu64 " Bytes)\n\n", pArgv[1], fileSize);

    if (!showStdDev)
    {
      puts("Codec                           Ratio      Encoder Throughput (Maximum)    Decoder Throughput (Maximum)    R*H/log2(|S|)\n"
           "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    }
    else
    {
      puts("Codec                           Ratio      Encoder Throughput (Maximum)    -StdDev ~ +StdDev     Decoder Throughput (Maximum)    -StdDev ~ +StdDev     R*H/log2(|S|)\n"
           "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

      individualRunsCapacity = 1024;
      pIndividualRuns = (size_t *)malloc(sizeof(size_t) * individualRunsCapacity);

      if (pIndividualRuns == NULL)
      {
        puts("Memory Allocation Failure");
        return 1;
      }
    }

    for (; currentCodec < CodecCount; currentCodec++)
    {
      if (matchBenchmarks && !CodecMatchesArgs(currentCodec))
        continue;

      if (!noDelays && currentCodec > 0)
        SleepNs(500 * 1000 * 1000);

      printf("%s|          | (dry run)", codecNames[currentCodec]);

      uint64_t compressionTime = 0;
      uint64_t fastestCompresionTime = UINT64_MAX;
      int64_t compressionRuns = -1;
      uint32_t compressedSize = 0;
      double encodeMinusStdDevMiBs = 0;
      double encodePlusStdDevMiBs = 0;

      individualRunsCount = 0;

      if (noDelays)
        compressionRuns = 0; // Skip dry run.

      const uint64_t compressStartTicks = GetCurrentTimeTicks();
      uint64_t lastSleepTicks = compressStartTicks;

      while (compressionRuns < (int64_t)runs || TicksToNs(GetCurrentTimeTicks() - compressStartTicks) < 1000000000 * (uint64_t)minSeconds)
      {
        uint64_t runTime = GetCurrentTimeTicks();

        switch (currentCodec)
        {
        default:
          compressedSize = codecCallbacks[currentCodec].compress_func(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case MemCopy:
          compressedSize = fileSize32;
          memcpy(pCompressedData, pUncompressedData, fileSize);
          break;
        }

        runTime = TicksToNs(GetCurrentTimeTicks() - runTime);

        if (compressedSize == 0)
          break;

        if (compressionRuns >= 0)
          compressionTime += runTime;

        if (runTime < fastestCompresionTime)
          fastestCompresionTime = runTime;

        if (showStdDev && compressionRuns > 0)
        {
          if (individualRunsCapacity <= individualRunsCount)
          {
            individualRunsCapacity *= 2;
            pIndividualRuns = (size_t *)realloc(pIndividualRuns, sizeof(size_t) * individualRunsCapacity);

            if (pIndividualRuns == NULL)
            {
              puts("Memory Allocation Failure");
              return 1;
            }
          }

          pIndividualRuns[individualRunsCount++] = runTime;
        }

        compressionRuns++;

        if (compressionRuns > 0)
          printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s)", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9));

        if (!noDelays)
        {
          const uint64_t sinceSleepNs = TicksToNs(GetCurrentTimeTicks() - lastSleepTicks);

          if (sinceSleepNs > 250 * 1000 * 1000) // Prevent thermal saturation.
          {
            SleepNs(min(sinceSleepNs / 2, 2 * 1000 * 1000 * 1000));
            lastSleepTicks = GetCurrentTimeTicks();
          }
        }
      }

      if (showStdDev)
      {
        const double meanNs = compressionTime / (double)compressionRuns;
        double stdDevNs = 0;

        for (size_t i = 0; i < individualRunsCount; i++)
        {
          const double diff = pIndividualRuns[i] - meanNs;
          stdDevNs += diff * diff;
        }

        stdDevNs = sqrt(stdDevNs / (double)(individualRunsCount - 1));

        encodePlusStdDevMiBs = (fileSize / (double)(1024 * 1024)) / ((meanNs + stdDevNs) / 1e9);
        encodeMinusStdDevMiBs = (fileSize / (double)(1024 * 1024)) / ((meanNs - stdDevNs) / 1e9);

        printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %5.0f ~ %5.0f MiB/s", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9), encodePlusStdDevMiBs, encodeMinusStdDevMiBs);
      }

      if (compressedSize == 0)
      {
        printf("\r%s| <FAILED TO COMRPESS>\n", codecNames[currentCodec]);


        if (!avxSupported && currentCodec == MultiMTF256)
        {
          printf("\r%s| <AVX NOT SUPPORTED BY PLATFORM>\n", codecNames[currentCodec]);

          continue;
        }

        if (!sse2Supported && (currentCodec == MultiMTF128 || currentCodec == MultiMTF256))
        {
          printf("\r%s| <SSE2 NOT SUPPORTED BY PLATFORM>\n", codecNames[currentCodec]);

          continue;
        }

        if (isTestRun)
          return -1;

        continue;
      }

      int64_t decompressionRuns = -1;
      uint64_t decompressionTime = 0;
      uint64_t fastestDecompresionTime = UINT64_MAX;
      uint32_t decompressedSize = 0;
      double decodeMinusStdDevMiBs = 0;
      double decodePlusStdDevMiBs = 0;

      individualRunsCount = 0;

      if (isTestRun || (!isTestRun & !noDelays))
      {
        for (size_t i = compressedSize; i < compressedBufferSize; i++)
          pCompressedData[i] = ~pCompressedData[i];

        memset(pDecompressedData, 0, decompressedSize);
      }

      if (noDelays)
        decompressionRuns = 0; // Skip dry run.

      if (showStdDev)
        printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %5.0f ~ %5.0f MiB/s | (dry run)", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9), encodePlusStdDevMiBs, encodeMinusStdDevMiBs);
      else
        printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | (dry run)", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9));

      if (!noDelays)
        SleepNs(500 * 1000 * 1000);

      const uint64_t decompressStartTicks = GetCurrentTimeTicks();

      while (decompressionRuns < (int64_t)runs || TicksToNs(GetCurrentTimeTicks() - decompressStartTicks) < 1000000000 * (uint64_t)minSeconds)
      {
        uint64_t runTime = GetCurrentTimeTicks();

        switch (currentCodec)
        {
        default:
          decompressedSize = codecCallbacks[currentCodec].decompress_func(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case MemCopy:
          decompressedSize = fileSize32;
          memcpy(pDecompressedData, pCompressedData, fileSize);
          break;
        }

        runTime = TicksToNs(GetCurrentTimeTicks() - runTime);

        if (decompressedSize == 0)
          break;

        if (decompressionRuns >= 0)
          decompressionTime += runTime;

        if (runTime < fastestDecompresionTime)
          fastestDecompresionTime = runTime;

        if (showStdDev && decompressionRuns > 0)
        {
          if (individualRunsCapacity <= individualRunsCount)
          {
            individualRunsCapacity *= 2;
            pIndividualRuns = (size_t *)realloc(pIndividualRuns, sizeof(size_t) * individualRunsCapacity);

            if (pIndividualRuns == NULL)
            {
              puts("Memory Allocation Failure");
              return 1;
            }
          }

          pIndividualRuns[individualRunsCount++] = runTime;
        }

        decompressionRuns++;

        if (decompressionRuns > 0)
        {
          if (showStdDev)
            printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %5.0f ~ %5.0f MiB/s | %7.1f MiB/s (%7.1f MiB/s)", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9), encodePlusStdDevMiBs, encodeMinusStdDevMiBs, (fileSize * (double)decompressionRuns / (double)(1024 * 1024)) / (decompressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestDecompresionTime / 1e9));
          else
            printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %7.1f MiB/s (%7.1f MiB/s)", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9), (fileSize * (double)decompressionRuns / (double)(1024 * 1024)) / (decompressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestDecompresionTime / 1e9));
        }

        if (!noDelays)
        {
          const uint64_t sinceSleepNs = TicksToNs(GetCurrentTimeTicks() - lastSleepTicks);

          if (sinceSleepNs > 250 * 1000 * 1000) // Prevent thermal saturation.
          {
            SleepNs(min(sinceSleepNs / 2, 2 * 1000 * 1000 * 1000));
            lastSleepTicks = GetCurrentTimeTicks();
          }
        }
      }

      if (decompressedSize == 0)
      {
        if (showStdDev)
          printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %5.0f ~ %5.0f MiB/s | <FAILED TO DECOMRPESS>", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9), encodePlusStdDevMiBs, encodeMinusStdDevMiBs);
        else
          printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | <FAILED TO DECOMRPESS>", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9));

        if (isTestRun)
        {
          if (!sse2Supported && (currentCodec == MultiMTF128 || currentCodec == MultiMTF256))
            puts("<SSE2 NOT SUPPORTED BY PLATFORM>");
          else
            return -1;
        }

        continue;
      }

      if (showStdDev)
      {
        const double meanNs = decompressionTime / (double)decompressionRuns;
        double stdDevNs = 0;

        for (size_t i = 0; i < individualRunsCount; i++)
        {
          const double diff = pIndividualRuns[i] - meanNs;
          stdDevNs += diff * diff;
        }

        stdDevNs = sqrt(stdDevNs / (double)(individualRunsCount - 1));

        decodePlusStdDevMiBs = (fileSize / (double)(1024 * 1024)) / ((meanNs + stdDevNs) / 1e9);
        decodeMinusStdDevMiBs = (fileSize / (double)(1024 * 1024)) / ((meanNs - stdDevNs) / 1e9);
      }

      if (showStdDev)
        printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %5.0f ~ %5.0f MiB/s | %7.1f MiB/s (%7.1f MiB/s) | %5.0f ~ %5.0f MiB/s | %11.7f %%", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9), encodePlusStdDevMiBs, encodeMinusStdDevMiBs, (fileSize * (double)decompressionRuns / (double)(1024 * 1024)) / (decompressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestDecompresionTime / 1e9), decodePlusStdDevMiBs, decodeMinusStdDevMiBs, ((compressedSize / (double)fileSize) * (GetInformationRatio(pCompressedData, compressedSize))) * 100.0f);
      else
        printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %7.1f MiB/s (%7.1f MiB/s) | %11.7f %%", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1e9), (fileSize * (double)decompressionRuns / (double)(1024 * 1024)) / (decompressionTime / 1e9), (fileSize / (double)(1024 * 1024)) / (fastestDecompresionTime / 1e9), ((compressedSize / (double)fileSize) * (GetInformationRatio(pCompressedData, compressedSize))) * 100.0f);

      puts("");

      if (!Validate(pUncompressedData, pDecompressedData, fileSize))
      {
        if (isTestRun)
          return -1;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////

  goto epilogue;

epilogue:
  if (pFile)
    fclose(pFile);
  
  ALIGNED_FREE(pUncompressedData);
  ALIGNED_FREE(pDecompressedData);
  ALIGNED_FREE(pCompressedData);

  return 0;
}

//////////////////////////////////////////////////////////////////////////

uint64_t GetCurrentTimeTicks()
{
#ifdef WIN32
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);

  return now.QuadPart;
#else
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);

  return (uint64_t)time.tv_sec * 1000000000 + (uint64_t)time.tv_nsec;
#endif
}

uint64_t TicksToNs(const uint64_t ticks)
{
#ifdef WIN32
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);

  return (ticks * 1000 * 1000 * 1000) / freq.QuadPart;
#else
  return ticks;
#endif
}

void SleepNs(const uint64_t sleepNs)
{
#ifdef _WIN32
  Sleep((DWORD)((sleepNs + 500 * 1000) / (1000 * 1000)));
#else
  usleep((uint32_t)((sleepNs + 500) / (1000)));
#endif
}

bool Validate(const uint8_t *pUncompressedData, const uint8_t *pDecompressedData, const size_t size)
{
  if (memcmp(pUncompressedData, pDecompressedData, (size_t)size) != 0)
  {
    puts("Validation Failed.");

    for (size_t i = 0; i < size; i++)
    {
      if (pUncompressedData[i] != pDecompressedData[i])
      {
        printf("First invalid char at %" PRIu64 " [0x%" PRIX64 "] (0x%" PRIX8 " != 0x%" PRIX8 ").\n", i, i, pUncompressedData[i], pDecompressedData[i]);

        const int64_t start = max(0, (int64_t)i - 64);
        const int64_t end = min((int64_t)size, (int64_t)(i + 64));

        printf("\nContext: (%" PRIi64 " to %" PRIi64 ")\n\n   Expected:                                        |  Actual Output:\n\n", start, end);

        for (int64_t context = start; context < end; context += 16)
        {
          const int64_t context_end = min(end, context + 16);

          bool different = false;

          for (int64_t j = context; j < context_end; j++)
          {
            if (pUncompressedData[j] != pDecompressedData[j])
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
            printf("%02" PRIX8 " ", pUncompressedData[j]);

          for (int64_t j = context_end; j < context + 16; j++)
            fputs("   ", stdout);

          fputs(" |  ", stdout);

          for (int64_t j = context; j < context_end; j++)
            printf("%02" PRIX8 " ", pDecompressedData[j]);

          puts("");

          if (different)
          {
            fputs("   ", stdout);

            for (int64_t j = context; j < context_end; j++)
            {
              if (pUncompressedData[j] != pDecompressedData[j])
                fputs("~~ ", stdout);
              else
                fputs("   ", stdout);
            }

            for (int64_t j = context_end; j < context + 16; j++)
              fputs("   ", stdout);

            fputs("    ", stdout);

            for (int64_t j = context; j < context_end; j++)
            {
              if (pUncompressedData[j] != pDecompressedData[j])
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

    return false;
  }

  return true;
}

double GetInformationRatio(const uint8_t *pData, const size_t length)
{
  uint64_t hist[256];
  memset(hist, 0, sizeof(hist));

  for (size_t i = 0; i < length; i++)
    hist[pData[i]]++;

  const double lengthD = (double)length;
  double ret = 0;
  uint64_t histSymbolCount = 0;

  for (size_t i = 0; i < 256; i++)
  {
    if (hist[i] != 0)
    {
      const double freq = hist[i] / lengthD;
      ret -= freq * log2(freq);
      histSymbolCount++;
    }
  }

  return ret / log2((double)histSymbolCount);
}

void AnalyzeData(const uint8_t *pData, const size_t size)
{
  size_t hist[256];
  size_t rle8hist[256];

  typedef struct
  {
    size_t rleLengthByBits[64];
    size_t rleLengthExact[64];
    size_t emptyLengthByBits[64];
    size_t emptyLengthExact[64];
    size_t alignedRleLengthByBits[64];
    size_t alignedRleLengthExact[64];
    size_t alignedEmptyLengthByBits[64];
    size_t alignedEmptyLengthExact[64];
    uint8_t lastSymbol[16][16];
    size_t recurringLastSymbol[16];
    uint8_t alignedLastSymbol[16][16];
    size_t alignedRecurringLastSymbol[16];
    size_t totalRleCount, totalRleLength, totalEmptyLength, currentLength, currentNonLength, lastLength, lastNonLength;
    int64_t lastNonLengthDiff;
    size_t alignedTotalRleCount, alignedTotalRleLength, alignedTotalEmptyLength, alignedCurrentLength, alignedCurrentNonLength, alignedLastLength, alignedLastNonLength;
    int64_t alignedLastNonLengthDiff;
    size_t copyBitsVsRleLengthBits[16 * 16];
    size_t alignedCopyBitsVsRleLengthBits[16 * 16];
    size_t copyDiffVsCountDiff[32 * 32];
    size_t alignedCopyDiffVsCountDiff[32 * 32];
  } rle_data_t;
  
  static rle_data_t byRLE[16];

  memset(hist, 0, sizeof(hist));
  memset(rle8hist, 0, sizeof(rle8hist));
  memset(byRLE, 0, sizeof(byRLE));

  printf("\rAnalyzing Data...");

  const size_t percentStep = size / 100;
  size_t nextPercent = percentStep;
  
  for (size_t i = 0; i < size; i++)
  {
    hist[pData[i]]++;
    
    for (size_t j = 1; j <= 16; j++)
    {
      if (i < j)
        continue;

      const bool match = memcmp(pData + i, pData + i - j, j) == 0;

      rle_data_t *pRLE = &byRLE[j - 1];

      if (match)
      {
        if (pRLE->currentLength == 0)
        {
          size_t recurringIndex = 0;

          for (; recurringIndex < 16; recurringIndex++)
          {
            if (memcmp(pRLE->lastSymbol[recurringIndex], pData + i, j) == 0)
            {
              pRLE->recurringLastSymbol[recurringIndex]++;
              break;
            }
          }

          if (recurringIndex != 0)
            for (size_t k = min(recurringIndex, 16 - 1); k >= 1; k--)
              memcpy(pRLE->lastSymbol[k], pRLE->lastSymbol[k - 1], j);

          memcpy(pRLE->lastSymbol[0], pData + i, j);

          if (j == 1)
            rle8hist[pData[i]]++;

          if (pRLE->currentNonLength)
          {
#ifdef _MSC_VER
            unsigned long index;
            _BitScanReverse64(&index, pRLE->currentNonLength);
#else
            const uint32_t index = 63 - __builtin_clz(pRLE->currentNonLength);
#endif

            pRLE->emptyLengthByBits[index]++;

            if (pRLE->currentNonLength <= 64)
              pRLE->emptyLengthExact[pRLE->currentNonLength - 1]++;

            pRLE->totalEmptyLength += pRLE->currentNonLength;

            pRLE->lastNonLengthDiff = pRLE->currentNonLength - pRLE->lastLength;
            pRLE->lastNonLength = pRLE->currentNonLength;
            pRLE->currentNonLength = 0;
          }

          pRLE->currentLength = j;
        }
        else
        {
          pRLE->currentLength++;
        }
      }
      else
      {
        if (pRLE->currentLength)
        {
#ifdef _MSC_VER
          unsigned long index;
          _BitScanReverse64(&index, pRLE->currentLength);
#else
          const uint32_t index = 63 - __builtin_clz(pRLE->currentLength);
#endif

          pRLE->rleLengthByBits[index]++;

          if (pRLE->currentLength <= 64)
            pRLE->rleLengthExact[pRLE->currentLength - 1]++;

          pRLE->totalRleLength += pRLE->currentLength;
          pRLE->totalRleCount++;

#ifdef _MSC_VER
          unsigned long lastNonLengthBits;
          _BitScanReverse64(&lastNonLengthBits, pRLE->lastNonLength);
#else
          const uint32_t lastNonLengthBits = 63 - __builtin_clz(pRLE->lastNonLength);
#endif

          pRLE->copyBitsVsRleLengthBits[(max((int64_t)0, min((int64_t)lastNonLengthBits - 1, (int64_t)15))) * 16 + (max((int64_t)0, min((int64_t)index - 1, (int64_t)15)))]++;

          const int64_t copyLengthDiff = pRLE->lastNonLengthDiff;
          const int64_t lengthDiff = pRLE->currentLength - pRLE->lastLength;

          pRLE->copyDiffVsCountDiff[(max((int64_t)0, min((int64_t)31, (int64_t)copyLengthDiff + 15))) * 32 + (max((int64_t)0, min((int64_t)31, (int64_t)lengthDiff + 15)))]++;

          pRLE->lastLength = pRLE->currentLength;
          pRLE->currentLength = 0;
        }

        pRLE->currentNonLength++;
      }

      if (i % j == 0)
      {
        if (match)
        {
          if (pRLE->alignedCurrentLength == 0)
          {
            size_t recurringIndex = 0;

            for (; recurringIndex < 16; recurringIndex++)
            {
              if (memcmp(pRLE->alignedLastSymbol[recurringIndex], pData + i, j) == 0)
              {
                pRLE->alignedRecurringLastSymbol[recurringIndex]++;
                break;
              }
            }

            if (recurringIndex != 0)
              for (size_t k = min(recurringIndex, 16 - 1); k >= 1; k--)
                memcpy(pRLE->alignedLastSymbol[k], pRLE->alignedLastSymbol[k - 1], j);

            memcpy(pRLE->alignedLastSymbol[0], pData + i, j);

            if (pRLE->alignedCurrentNonLength)
            {
#ifdef _MSC_VER
              unsigned long index;
              _BitScanReverse64(&index, pRLE->alignedCurrentNonLength);
#else
              const uint32_t index = 63 - __builtin_clz(pRLE->alignedCurrentNonLength);
#endif

              pRLE->alignedEmptyLengthByBits[index]++;

              if (pRLE->alignedCurrentNonLength <= 64)
                pRLE->alignedEmptyLengthExact[pRLE->alignedCurrentNonLength - 1]++;

              pRLE->alignedTotalEmptyLength += pRLE->alignedCurrentNonLength;

              pRLE->alignedLastNonLengthDiff = pRLE->alignedCurrentNonLength - pRLE->alignedLastLength;
              pRLE->alignedLastNonLength = pRLE->alignedCurrentNonLength;
              pRLE->alignedCurrentNonLength = 0;
            }

            pRLE->alignedCurrentLength = 1;
          }
          else
          {
            pRLE->alignedCurrentLength++;
          }
        }
        else
        {
          if (pRLE->alignedCurrentLength)
          {
#ifdef _MSC_VER
            unsigned long index;
            _BitScanReverse64(&index, pRLE->alignedCurrentLength);
#else
            const uint32_t index = 63 - __builtin_clz(pRLE->alignedCurrentLength);
#endif

            pRLE->alignedRleLengthByBits[index]++;

            if (pRLE->alignedCurrentLength <= 64)
              pRLE->alignedRleLengthExact[pRLE->alignedCurrentLength - 1]++;

            pRLE->alignedTotalRleLength += pRLE->alignedCurrentLength;
            pRLE->alignedTotalRleCount++;

#ifdef _MSC_VER
            unsigned long lastNonLengthBits;
            _BitScanReverse64(&lastNonLengthBits, pRLE->alignedLastNonLength);
#else
            const uint32_t lastNonLengthBits = 63 - __builtin_clz(pRLE->alignedLastNonLength);
#endif

            pRLE->alignedCopyBitsVsRleLengthBits[(max((int64_t)0, min((int64_t)lastNonLengthBits - 1, (int64_t)15))) * 16 + (max((int64_t)0, min((int64_t)index - 1, (int64_t)15)))]++;

            const int64_t copyLengthDiff = pRLE->alignedLastNonLengthDiff;
            const int64_t lengthDiff = pRLE->alignedCurrentLength - pRLE->alignedLastLength;

            pRLE->alignedCopyDiffVsCountDiff[(max((int64_t)0, min((int64_t)31, (int64_t)copyLengthDiff + 15))) * 32 + (max((int64_t)0, min((int64_t)31, (int64_t)lengthDiff + 15)))]++;

            pRLE->alignedLastLength = pRLE->alignedCurrentLength;
            pRLE->alignedCurrentLength = 0;
          }

          pRLE->alignedCurrentNonLength++;
        }
      }
    }

    if (i > nextPercent)
    {
      nextPercent += percentStep;

      printf("\rAnalyzing Data... (%" PRIu64 " %%)", i * 100 / size);
    }
  }

  printf("\rAnalysis Complete. %10" PRIu64 " Bytes Total.\n\n", size);

  for (size_t i = 0; i < 16; i++)
  {
    rle_data_t *pRLE = &byRLE[i];

    if (pRLE->currentNonLength)
    {
#ifdef _MSC_VER
      unsigned long index;
      _BitScanReverse64(&index, pRLE->currentNonLength);
#else
      const uint32_t index = 63 - __builtin_clz(pRLE->currentNonLength);
#endif

      pRLE->emptyLengthByBits[index]++;

      if (pRLE->currentNonLength <= 64)
        pRLE->emptyLengthExact[pRLE->currentNonLength - 1]++;

      pRLE->totalEmptyLength += pRLE->currentNonLength;
    }
    else if (pRLE->currentLength)
    {
#ifdef _MSC_VER
      unsigned long index;
      _BitScanReverse64(&index, pRLE->currentLength);
#else
      const uint32_t index = 63 - __builtin_clz(pRLE->currentLength);
#endif

      pRLE->rleLengthByBits[index]++;

      if (pRLE->currentLength <= 64)
        pRLE->rleLengthExact[pRLE->currentLength - 1]++;

      pRLE->totalRleLength += pRLE->currentLength;
      pRLE->totalRleCount++;
    }

    if (pRLE->alignedCurrentNonLength)
    {
#ifdef _MSC_VER
      unsigned long index;
      _BitScanReverse64(&index, pRLE->alignedCurrentNonLength);
#else
      const uint32_t index = 63 - __builtin_clz(pRLE->alignedCurrentNonLength);
#endif

      pRLE->alignedEmptyLengthByBits[index]++;

      if (pRLE->alignedCurrentNonLength <= 64)
        pRLE->alignedEmptyLengthExact[pRLE->alignedCurrentNonLength - 1]++;

      pRLE->alignedTotalEmptyLength += pRLE->alignedCurrentNonLength;
    }
    else if (pRLE->alignedCurrentLength)
    {
#ifdef _MSC_VER
      unsigned long index;
      _BitScanReverse64(&index, pRLE->alignedCurrentLength);
#else
      const uint32_t index = 63 - __builtin_clz(pRLE->alignedCurrentLength);
#endif

      pRLE->alignedRleLengthByBits[index]++;

      if (pRLE->alignedCurrentLength <= 64)
        pRLE->alignedRleLengthExact[pRLE->alignedCurrentLength - 1]++;

      pRLE->alignedTotalRleLength += pRLE->alignedCurrentLength;
      pRLE->alignedTotalRleCount++;
    }

    printf("RLE Length %2" PRIu64 "\n=============\n", i + 1);
    printf("Non-Aligned: Repeating: %10" PRIu64 " (%15" PRIu64 " Bytes, %5.2f %%, %15" PRIu64 " Bytes Distinct)\n", pRLE->totalRleCount, pRLE->totalRleLength, pRLE->totalRleLength * 100.0 / size, pRLE->totalEmptyLength);
    printf("Aligned:     Repeating: %10" PRIu64 " (%15" PRIu64 " Bytes, %5.2f %%, %15" PRIu64 " Bytes Distinct)\n", pRLE->alignedTotalRleCount, pRLE->alignedTotalRleLength, pRLE->alignedTotalRleLength * 100.0 / size, pRLE->alignedTotalEmptyLength);
    puts("");
    
    printf("AprxLen | Repeating    | Distinct     || ExactLen  | Repeating    | Distinct     || AprxCnt | AlgnRepeat   | AlgnDistinct || Exact Count | AlgnRepeat   | AlgnDistinct\n");
    printf("----------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    
    for (size_t j = 0; j < 64; j++)
      printf("%2" PRIu64 " Bit: | %12" PRIu64 " | %12" PRIu64 " || %2" PRIu64 " Bytes: | %12" PRIu64 " | %12" PRIu64 " || %2" PRIu64 " Bit: | %12" PRIu64 " | %12" PRIu64 " || %2" PRIu64 " Symbols: | %12" PRIu64 " | %12" PRIu64 "\n", j + 1, pRLE->rleLengthByBits[j], pRLE->emptyLengthByBits[j], j + 1, pRLE->rleLengthExact[j], pRLE->emptyLengthExact[j], j + 1, pRLE->alignedRleLengthByBits[j], pRLE->alignedEmptyLengthByBits[j], j + 1, pRLE->alignedRleLengthExact[j], pRLE->alignedEmptyLengthExact[j]);
    
    puts("");

    printf("#  | References   (Of Total ) | Aligned Ref. (Of Total )\n");
    printf("--------------------------------------------------------\n");

    for (size_t j = 0; j < 16; j++)
      printf("%2" PRIu64 " | %12" PRIu64 " (%7.3f %%) | %12" PRIu64 " (%7.3f %%)\n", j + 1, pRLE->recurringLastSymbol[j], pRLE->recurringLastSymbol[j] / (double)pRLE->totalRleCount * 100.0, pRLE->alignedRecurringLastSymbol[j], pRLE->alignedRecurringLastSymbol[j] / (double)pRLE->alignedTotalRleCount * 100.0);

    puts("");
    
    printf("   | Non-Aligned Copy Length vs RLE Length Bits                                      | Aligned Copy Length vs RLE Length Bits\n");
    printf("%%  | 1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   >    | 1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   > \n");
    printf("---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (size_t j = 0; j < 16; j++)
    {
      if (j < 15)
        printf("%2" PRIu8 " | ", (uint8_t)(j + 1));
      else
        fputs(">  | ", stdout);

      for (size_t k = 0; k < 16; k++)
        printf("%4.1f ", pRLE->copyBitsVsRleLengthBits[j * 16 + k] / (double)pRLE->totalRleCount * 100.0);

      fputs("| ", stdout);

      for (size_t k = 0; k < 16; k++)
        printf("%4.1f ", pRLE->alignedCopyBitsVsRleLengthBits[j * 16 + k] / (double)pRLE->alignedTotalRleCount * 100.0);

      puts("");
    }

    puts("");

    printf("Non-Aligned Copy Length vs RLE Length:\n");
    printf("%%   | <    -14  -13  -12  -11  -10   -9   -8   -7   -6   -5   -4   -3   -2   -1    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   < \n");
    printf("---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (size_t j = 0; j < 32; j++)
    {
      if (j == 0)
        fputs(" <  | ", stdout);
      else if (j == 31)
        fputs(" >  | ", stdout);
      else
        printf("%3" PRIi8 " | ", (int8_t)j - 15);

      for (size_t k = 0; k < 32; k++)
        printf("%4.1f ", pRLE->copyDiffVsCountDiff[j * 32 + k] / (double)pRLE->totalRleCount * 100.0);

      puts("");
    }

    puts("");

    printf("Aligned Copy Length vs RLE Length:\n");
    printf("%%   | <    -14  -13  -12  -11  -10   -9   -8   -7   -6   -5   -4   -3   -2   -1    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   < \n");
    printf("---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (size_t j = 0; j < 32; j++)
    {
      if (j == 0)
        fputs(" <  | ", stdout);
      else if (j == 31)
        fputs(" >  | ", stdout);
      else
        printf("%3" PRIi8 " | ", (int8_t)j - 15);

      for (size_t k = 0; k < 32; k++)
        printf("%4.1f ", pRLE->alignedCopyDiffVsCountDiff[j * 32 + k] / (double)pRLE->totalRleCount * 100.0);

      puts("");
    }

    puts("");

    if (i == 0)
    {
      puts("RLE-Symbol Histogram:");
      puts("% | 0     | 1     | 2     | 3     | 4     | 5     | 6     | 7     | 8     | 9     | A     | B     | C     | D     | E     | F     |");
      puts("-----------------------------------------------------------------------------------------------------------------------------------");

      for (uint8_t j = 0; j <= 0xF; j++)
      {
        printf("%" PRIX8 " | ", j);

        for (uint8_t k = 0; k <= 0xF; k++)
          printf("%5.2f | ", rle8hist[(k << 4) | j] * 100.0 / (double)pRLE->totalRleCount);

        puts("");
      }

      puts("\n");
    }
  }

  puts("Histogram:");
  puts("% | 0     | 1     | 2     | 3     | 4     | 5     | 6     | 7     | 8     | 9     | A     | B     | C     | D     | E     | F     |");
  puts("-----------------------------------------------------------------------------------------------------------------------------------");

  for (uint8_t i = 0; i <= 0xF; i++)
  {
    printf("%" PRIX8 " | ", i);

    for (uint8_t j = 0; j <= 0xF; j++)
      printf("%5.2f | ", hist[(j << 4) | i] * 100.0 / (double)size);

    puts("");
  }

  puts("\n\n");
}

bool CodecMatchesArgs(const codec_t codec)
{
  if (_Args.hasMode)
  {
    if (_Args.isModeExtreme && (codec > Extreme128BytePacked || codec < Extreme8))
      return false;

    if (_Args.isModeLowEntropy && (codec < LowEntropy || codec > LowEntropyShortSingle))
      return false;

    if (_Args.isModeMMTF && codec != MultiMTF128 && codec != MultiMTF256 && codec != BitMultiMTF8 && codec != BitMultiMTF16)
      return false;

    if (_Args.isModeSH && codec != Rle8SH)
      return false;

    if (_Args.isModeRleMMTF && codec != Extreme8MultiMTF128)
      return false;
  }

  if (_Args.hasAlignment)
  {
    switch (codec)
    {
      case Extreme16Sym:
      case Extreme16SymShort:
      case Extreme16SymPacked:
      case Extreme16Sym_1SLShort:
      case Extreme16Sym_3SL:
      case Extreme16Sym_3SLShort:
      case Extreme16Sym_7SL:
      case Extreme16Sym_7SLShort:
      case Extreme24Sym:
      case Extreme24SymShort:
      case Extreme24SymPacked:
      case Extreme24Sym_1SLShort:
      case Extreme24Sym_3SL:
      case Extreme24Sym_3SLShort:
      case Extreme24Sym_7SL:
      case Extreme24Sym_7SLShort:
      case Extreme32Sym:
      case Extreme32SymShort:
      case Extreme32SymPacked:
      case Extreme32Sym_1SLShort:
      case Extreme32Sym_3SL:
      case Extreme32Sym_3SLShort:
      case Extreme32Sym_7SL:
      case Extreme32Sym_7SLShort:
      case Extreme48Sym:
      case Extreme48SymShort:
      case Extreme48SymPacked:
      case Extreme48Sym_1SLShort:
      case Extreme48Sym_3SL:
      case Extreme48Sym_3SLShort:
      case Extreme48Sym_7SL:
      case Extreme48Sym_7SLShort:
      case Extreme64Sym:
      case Extreme64SymShort:
      case Extreme64SymPacked:
      case Extreme64Sym_1SLShort:
      case Extreme64Sym_3SL:
      case Extreme64Sym_3SLShort:
      case Extreme64Sym_7SL:
      case Extreme64Sym_7SLShort:
      case Extreme128Sym:
      case Extreme128SymPacked:
        if (_Args.isAlignmentByte)
          return false;
        break;

      case Extreme16Byte:
      case Extreme16ByteShort:
      case Extreme16BytePacked:
      case Extreme16Byte_1SLShort:
      case Extreme16Byte_1SLShortGreedy:
      case Extreme16Byte_3SL:
      case Extreme16Byte_3SLShort:
      case Extreme16Byte_3SLShortGreedy:
      case Extreme16Byte_7SL:
      case Extreme16Byte_7SLShort:
      case Extreme16Byte_7SLShortGreedy:
      case Extreme24Byte:
      case Extreme24ByteShort:
      case Extreme24BytePacked:
      case Extreme24Byte_1SLShort:
      case Extreme24Byte_1SLShortGreedy:
      case Extreme24Byte_3SL:
      case Extreme24Byte_3SLShort:
      case Extreme24Byte_3SLShortGreedy:
      case Extreme24Byte_7SL:
      case Extreme24Byte_7SLShort:
      case Extreme24Byte_7SLShortGreedy:
      case Extreme32Byte:
      case Extreme32ByteShort:
      case Extreme32BytePacked:
      case Extreme32Byte_1SLShort:
      case Extreme32Byte_1SLShortGreedy:
      case Extreme32Byte_3SL:
      case Extreme32Byte_3SLShort:
      case Extreme32Byte_3SLShortGreedy:
      case Extreme32Byte_7SL:
      case Extreme32Byte_7SLShort:
      case Extreme32Byte_7SLShortGreedy:
      case Extreme48Byte:
      case Extreme48ByteShort:
      case Extreme48BytePacked:
      case Extreme48Byte_1SLShort:
      case Extreme48Byte_1SLShortGreedy:
      case Extreme48Byte_3SL:
      case Extreme48Byte_3SLShort:
      case Extreme48Byte_3SLShortGreedy:
      case Extreme48Byte_7SL:
      case Extreme48Byte_7SLShort:
      case Extreme48Byte_7SLShortGreedy:
      case Extreme64Byte:
      case Extreme64ByteShort:
      case Extreme64BytePacked:
      case Extreme64Byte_1SLShort:
      case Extreme64Byte_1SLShortGreedy:
      case Extreme64Byte_3SL:
      case Extreme64Byte_3SLShort:
      case Extreme64Byte_3SLShortGreedy:
      case Extreme64Byte_7SL:
      case Extreme64Byte_7SLShort:
      case Extreme64Byte_7SLShortGreedy:
      case Extreme128Byte:
      case Extreme128BytePacked:
        if (!_Args.isAlignmentByte)
          return false;
        break;

      default:
        return false;
    }
  }

  if (_Args.hasSingleMode)
  {
    switch (codec)
    {
    case Extreme8Single:
    case Extreme8SingleShort:
    case Extreme8PackedSingle:
    case LowEntropySingle:
    case LowEntropyShortSingle:
      if (!_Args.isSingleMode)
        return false;
      break;

    case Rle8SH:
    case Extreme8MultiMTF128:
    case MultiMTF128:
    case MultiMTF256:
      return false; // the entire concept of single/multi makes no sense here.

    default:
      if (_Args.isSingleMode)
        return false;
      break;
    }
  }

  if (_Args.hasPackedMode)
  {
    switch (codec)
    {
    case Extreme8Packed:
    case Extreme8PackedSingle:
    case Extreme16SymPacked:
    case Extreme16BytePacked:
    case Extreme24SymPacked:
    case Extreme24BytePacked:
    case Extreme32SymPacked:
    case Extreme32BytePacked:
    case Extreme48SymPacked:
    case Extreme48BytePacked:
    case Extreme64SymPacked:
    case Extreme64BytePacked:
    case Extreme128SymPacked:
    case Extreme128BytePacked:
      if (!_Args.isPacked)
        return false;
      break;

    default:
      if (_Args.isPacked)
        return false;
      break;
    }
  }

  if (_Args.hasShortMode)
  {
    switch (codec)
    {
    case Extreme8Short:
    case Extreme8_1SLShort:
    case Extreme8_3SLShort:
    case Extreme8_7SLShort:
    case Extreme8SingleShort:
    case Extreme16SymShort:
    case Extreme16Sym_1SLShort:
    case Extreme16Sym_3SLShort:
    case Extreme16Sym_7SLShort:
    case Extreme16ByteShort:
    case Extreme16Byte_1SLShort:
    case Extreme16Byte_1SLShortGreedy:
    case Extreme16Byte_3SLShort:
    case Extreme16Byte_3SLShortGreedy:
    case Extreme16Byte_7SLShort:
    case Extreme16Byte_7SLShortGreedy:
    case Extreme24SymShort:
    case Extreme24Sym_1SLShort:
    case Extreme24Sym_3SLShort:
    case Extreme24Sym_7SLShort:
    case Extreme24ByteShort:
    case Extreme24Byte_1SLShort:
    case Extreme24Byte_1SLShortGreedy:
    case Extreme24Byte_3SLShort:
    case Extreme24Byte_3SLShortGreedy:
    case Extreme24Byte_7SLShort:
    case Extreme24Byte_7SLShortGreedy:
    case Extreme32SymShort:
    case Extreme32Sym_1SLShort:
    case Extreme32Sym_3SLShort:
    case Extreme32Sym_7SLShort:
    case Extreme32ByteShort:
    case Extreme32Byte_1SLShort:
    case Extreme32Byte_1SLShortGreedy:
    case Extreme32Byte_3SLShort:
    case Extreme32Byte_3SLShortGreedy:
    case Extreme32Byte_7SLShort:
    case Extreme32Byte_7SLShortGreedy:
    case Extreme48SymShort:
    case Extreme48Sym_1SLShort:
    case Extreme48Sym_3SLShort:
    case Extreme48Sym_7SLShort:
    case Extreme48ByteShort:
    case Extreme48Byte_1SLShort:
    case Extreme48Byte_1SLShortGreedy:
    case Extreme48Byte_3SLShort:
    case Extreme48Byte_3SLShortGreedy:
    case Extreme48Byte_7SLShort:
    case Extreme48Byte_7SLShortGreedy:
    case Extreme64SymShort:
    case Extreme64Sym_1SLShort:
    case Extreme64Sym_3SLShort:
    case Extreme64Sym_7SLShort:
    case Extreme64ByteShort:
    case Extreme64Byte_1SLShort:
    case Extreme64Byte_1SLShortGreedy:
    case Extreme64Byte_3SLShort:
    case Extreme64Byte_3SLShortGreedy:
    case Extreme64Byte_7SLShort:
    case Extreme64Byte_7SLShortGreedy:
    case LowEntropyShort:
    case LowEntropyShortSingle:
      if (!_Args.isShortMode)
        return false;
      break;

    default:
      if (_Args.isShortMode)
        return false;
      break;
    }
  }

  if (_Args.hasGreedyMode)
  {
    switch (codec)
    {
    default:
      if (_Args.isGreedy)
        return false;
      break;

    case Extreme16Byte_1SLShortGreedy:
    case Extreme16Byte_3SLShortGreedy:
    case Extreme16Byte_7SLShortGreedy:
    case Extreme24Byte_1SLShortGreedy:
    case Extreme24Byte_3SLShortGreedy:
    case Extreme24Byte_7SLShortGreedy:
    case Extreme32Byte_1SLShortGreedy:
    case Extreme32Byte_3SLShortGreedy:
    case Extreme32Byte_7SLShortGreedy:
    case Extreme48Byte_1SLShortGreedy:
    case Extreme48Byte_3SLShortGreedy:
    case Extreme48Byte_7SLShortGreedy:
    case Extreme64Byte_1SLShortGreedy:
    case Extreme64Byte_3SLShortGreedy:
    case Extreme64Byte_7SLShortGreedy:
      if (!_Args.isGreedy)
        return false;
      break;
    }
  }

  if (_Args.hasLutSize)
  {
    switch (codec)
    {
    case Extreme8Packed:
    case Extreme8PackedSingle:
    case Extreme8_1SLShort:
    case Extreme16SymPacked:
    case Extreme16Sym_1SLShort:
    case Extreme16BytePacked:
    case Extreme16Byte_1SLShort:
    case Extreme16Byte_1SLShortGreedy:
    case Extreme24SymPacked:
    case Extreme24Sym_1SLShort:
    case Extreme24BytePacked:
    case Extreme24Byte_1SLShort:
    case Extreme24Byte_1SLShortGreedy:
    case Extreme32SymPacked:
    case Extreme32Sym_1SLShort:
    case Extreme32BytePacked:
    case Extreme32Byte_1SLShort:
    case Extreme32Byte_1SLShortGreedy:
    case Extreme48SymPacked:
    case Extreme48Sym_1SLShort:
    case Extreme48BytePacked:
    case Extreme48Byte_1SLShort:
    case Extreme48Byte_1SLShortGreedy:
    case Extreme64SymPacked:
    case Extreme64Sym_1SLShort:
    case Extreme64BytePacked:
    case Extreme64Byte_1SLShort:
    case Extreme64Byte_1SLShortGreedy:
    case Extreme128SymPacked:
    case Extreme128BytePacked:
      if (_Args.lutSize != 1)
        return false;
      break;

    case Extreme8_3SL:
    case Extreme8_3SLShort:
    case Extreme16Sym_3SL:
    case Extreme16Sym_3SLShort:
    case Extreme16Byte_3SL:
    case Extreme16Byte_3SLShort:
    case Extreme16Byte_3SLShortGreedy:
    case Extreme24Sym_3SL:
    case Extreme24Sym_3SLShort:
    case Extreme24Byte_3SL:
    case Extreme24Byte_3SLShort:
    case Extreme24Byte_3SLShortGreedy:
    case Extreme32Sym_3SL:
    case Extreme32Sym_3SLShort:
    case Extreme32Byte_3SL:
    case Extreme32Byte_3SLShort:
    case Extreme32Byte_3SLShortGreedy:
    case Extreme48Sym_3SL:
    case Extreme48Sym_3SLShort:
    case Extreme48Byte_3SL:
    case Extreme48Byte_3SLShort:
    case Extreme48Byte_3SLShortGreedy:
    case Extreme64Sym_3SL:
    case Extreme64Sym_3SLShort:
    case Extreme64Byte_3SL:
    case Extreme64Byte_3SLShort:
    case Extreme64Byte_3SLShortGreedy:
      if (_Args.lutSize != 3)
        return false;
      break;
    
    case Extreme8_7SL:
    case Extreme8_7SLShort:
    case Extreme16Sym_7SL:
    case Extreme16Sym_7SLShort:
    case Extreme16Byte_7SL:
    case Extreme16Byte_7SLShort:
    case Extreme16Byte_7SLShortGreedy:
    case Extreme24Sym_7SL:
    case Extreme24Sym_7SLShort:
    case Extreme24Byte_7SL:
    case Extreme24Byte_7SLShort:
    case Extreme24Byte_7SLShortGreedy:
    case Extreme32Sym_7SL:
    case Extreme32Sym_7SLShort:
    case Extreme32Byte_7SL:
    case Extreme32Byte_7SLShort:
    case Extreme32Byte_7SLShortGreedy:
    case Extreme48Sym_7SL:
    case Extreme48Sym_7SLShort:
    case Extreme48Byte_7SL:
    case Extreme48Byte_7SLShort:
    case Extreme48Byte_7SLShortGreedy:
    case Extreme64Sym_7SL:
    case Extreme64Sym_7SLShort:
    case Extreme64Byte_7SL:
    case Extreme64Byte_7SLShort:
    case Extreme64Byte_7SLShortGreedy:
      if (_Args.lutSize != 7)
        return false;
      break;

    default:
      if (_Args.lutSize != 0)
        return false;
      break;
    }
  }

  if (_Args.hasBitCount)
  {
    switch (codec)
    {
    case Extreme8:
    case Extreme8Short:
    case Extreme8Packed:
    case Extreme8_1SLShort:
    case Extreme8_3SL:
    case Extreme8_3SLShort:
    case Extreme8_7SL:
    case Extreme8_7SLShort:
    case Extreme8Single:
    case Extreme8SingleShort:
    case Extreme8PackedSingle:
    case Rle8SH:
    case LowEntropy:
    case LowEntropySingle:
    case LowEntropyShort:
    case LowEntropyShortSingle:
    case BitMultiMTF8:
      if (_Args.bitCount != 8)
        return false;
      break;

    case Extreme16Sym:
    case Extreme16SymShort:
    case Extreme16SymPacked:
    case Extreme16Sym_1SLShort:
    case Extreme16Sym_3SL:
    case Extreme16Sym_3SLShort:
    case Extreme16Sym_7SL:
    case Extreme16Sym_7SLShort:
    case Extreme16Byte:
    case Extreme16ByteShort:
    case Extreme16BytePacked:
    case Extreme16Byte_1SLShort:
    case Extreme16Byte_1SLShortGreedy:
    case Extreme16Byte_3SL:
    case Extreme16Byte_3SLShort:
    case Extreme16Byte_3SLShortGreedy:
    case Extreme16Byte_7SL:
    case Extreme16Byte_7SLShort:
    case Extreme16Byte_7SLShortGreedy:
    case BitMultiMTF16:
      if (_Args.bitCount != 16)
        return false;
      break;

    case Extreme24Sym:
    case Extreme24SymShort:
    case Extreme24SymPacked:
    case Extreme24Sym_1SLShort:
    case Extreme24Sym_3SL:
    case Extreme24Sym_3SLShort:
    case Extreme24Sym_7SL:
    case Extreme24Sym_7SLShort:
    case Extreme24Byte:
    case Extreme24ByteShort:
    case Extreme24BytePacked:
    case Extreme24Byte_1SLShort:
    case Extreme24Byte_1SLShortGreedy:
    case Extreme24Byte_3SL:
    case Extreme24Byte_3SLShort:
    case Extreme24Byte_3SLShortGreedy:
    case Extreme24Byte_7SL:
    case Extreme24Byte_7SLShort:
    case Extreme24Byte_7SLShortGreedy:
      if (_Args.bitCount != 24)
        return false;
      break;

    case Extreme32Sym:
    case Extreme32SymShort:
    case Extreme32SymPacked:
    case Extreme32Sym_1SLShort:
    case Extreme32Sym_3SL:
    case Extreme32Sym_3SLShort:
    case Extreme32Sym_7SL:
    case Extreme32Sym_7SLShort:
    case Extreme32Byte:
    case Extreme32ByteShort:
    case Extreme32BytePacked:
    case Extreme32Byte_1SLShort:
    case Extreme32Byte_1SLShortGreedy:
    case Extreme32Byte_3SL:
    case Extreme32Byte_3SLShort:
    case Extreme32Byte_3SLShortGreedy:
    case Extreme32Byte_7SL:
    case Extreme32Byte_7SLShort:
    case Extreme32Byte_7SLShortGreedy:
      if (_Args.bitCount != 32)
        return false;
      break;

    case Extreme48Sym:
    case Extreme48SymShort:
    case Extreme48SymPacked:
    case Extreme48Sym_1SLShort:
    case Extreme48Sym_3SL:
    case Extreme48Sym_3SLShort:
    case Extreme48Sym_7SL:
    case Extreme48Sym_7SLShort:
    case Extreme48Byte:
    case Extreme48ByteShort:
    case Extreme48BytePacked:
    case Extreme48Byte_1SLShort:
    case Extreme48Byte_1SLShortGreedy:
    case Extreme48Byte_3SL:
    case Extreme48Byte_3SLShort:
    case Extreme48Byte_3SLShortGreedy:
    case Extreme48Byte_7SL:
    case Extreme48Byte_7SLShort:
    case Extreme48Byte_7SLShortGreedy:
      if (_Args.bitCount != 48)
        return false;
      break;

    case Extreme64Sym:
    case Extreme64SymShort:
    case Extreme64SymPacked:
    case Extreme64Sym_1SLShort:
    case Extreme64Sym_3SL:
    case Extreme64Sym_3SLShort:
    case Extreme64Sym_7SL:
    case Extreme64Sym_7SLShort:
    case Extreme64Byte:
    case Extreme64ByteShort:
    case Extreme64BytePacked:
    case Extreme64Byte_1SLShort:
    case Extreme64Byte_1SLShortGreedy:
    case Extreme64Byte_3SL:
    case Extreme64Byte_3SLShort:
    case Extreme64Byte_3SLShortGreedy:
    case Extreme64Byte_7SL:
    case Extreme64Byte_7SLShort:
    case Extreme64Byte_7SLShortGreedy:
      if (_Args.bitCount != 64)
        return false;
      break;

    case Extreme128Sym:
    case Extreme128SymPacked:
    case Extreme128Byte:
    case Extreme128BytePacked:
    case Extreme8MultiMTF128:
    case MultiMTF128:
      if (_Args.bitCount != 128)
        return false;
      break;

    case MultiMTF256:
      if (_Args.bitCount != 256)
        return false;
      break;

    default:
      return false;
    }
  }
  
  return true;
}
