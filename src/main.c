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

const char ArgumentTo[] = "--to";
const char ArgumentSubSections[] = "--sub-sections";
const char ArgumentRuns[] = "--runs";
const char ArgumentNormal[] = "--low-entropy";
const char ArgumentSingle[] = "--single";
const char ArgumentUltra[] = "--low-entropy-short";
const char ArgumentExtreme[] = "--extreme";
const char ArgumentExtremeSize[] = "--x-size";
const char ArgumentExtremeByteGran[] = "--byte-granularity";
const char ArgumentExtremePacked[] = "--packed";
const char ArgumentMinimumTime[] = "--min-time";
const char ArgumentExtremeMMTF[] = "--rle-mmtf";
const char ArgumentMMTF[] = "--mmtf";
const char ArgumentSH[] = "--sh";
const char ArgumentAnalyze[] = "--analyze";

#ifdef _WIN32
const char ArgumentCpuCore[] = "--cpu-core";
#endif

uint64_t GetCurrentTimeTicks();
uint64_t TicksToNs(const uint64_t ticks);
void SleepNs(const uint64_t sleepNs);
bool Validate(const uint8_t *pUncompressedData, const uint8_t *pDecompressedData, const size_t fileSize);
double GetInformationRatio(const uint8_t *pData, const size_t length);
void AnalyzeData(const uint8_t *pData, const size_t size);

//////////////////////////////////////////////////////////////////////////

int main(int argc, char **pArgv)
{
  if (argc <= 1)
  {
    printf("Usage: hsrlekit <InputFileName>\n\n");
    printf("\t[%s <Run Count>]\n\t[%s <Minimum Benchmark Time in Seconds>]\n\n\t", ArgumentRuns, ArgumentMinimumTime);
    printf("OR:\n\n");
    printf("\t[%s <Output File Name>]\n\n", ArgumentTo);
    printf("\t[%s]\n\t\tif '%s': [%s (8 | 16 | 24 | 32 | 48 | 64 | 128)] (symbol size)\n\t\tif '%s': [%s] (include unaligned repeats, capacity vs. accuracy tradeoff)\n\t\tif '%s': [%s] (preferable if many rle-symbol-repeats)\n\n", ArgumentExtreme, ArgumentExtreme, ArgumentExtremeSize, ArgumentExtreme, ArgumentExtremeByteGran, ArgumentExtreme, ArgumentExtremePacked);
    printf("\t[%s (try to preserve symbol frequencies)]\n\t\tif '%s': [%s <Sub Section Count>] \n\n", ArgumentNormal, ArgumentNormal, ArgumentSubSections);
    printf("\t[%s ('%s' optimized for fewer repititions)]\n\n", ArgumentUltra, ArgumentNormal);
    printf("\t[%s]\n\t\tif '%s': [%s (128 | 256)] (mtf width)\n\n", ArgumentExtremeMMTF, ArgumentExtremeMMTF, ArgumentExtremeSize);
    printf("\t[%s (only transform, no compression)]\n\t\tif '%s': [%s(128 | 256)] (mtf width)\n\n", ArgumentMMTF, ArgumentMMTF, ArgumentExtremeSize);
    printf("\t[%s (separate bit packed header, doesn't support '%s')]\n\n", ArgumentSH, ArgumentSingle);
    printf("\t[%s (only rle most frequent symbol, only available for 8 bit modes)]\n\n\t[%s <Run Count>]\n", ArgumentSingle, ArgumentRuns);
    printf("\t[%s (analyze file contents for compressability)]n", ArgumentAnalyze);

#ifdef _WIN32
    printf("\n\t[%s <CPU Core Index>]\n", ArgumentCpuCore);
#endif
    
    return 1;
  }

  const char *outputFileName = NULL;
  int32_t subSections = 0;
  int32_t runs = 8;
  int32_t minSeconds = 2;
  bool lowEntropyMode = false;
  bool singleSymbol = false;
  bool lowEntropyShortMode = false;
  bool extremeMode = false;
  bool mmtfMode = false;
  bool extremeMmtfMode = false;
  bool shMode = false;
  bool benchmarkAll = false;
  bool analyzeFileContents = false;
  bool byteGranular = false;
  bool packed = false;
  uint64_t extremeSize = 8;

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
      if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentTo, sizeof(ArgumentTo)) == 0)
      {
        outputFileName = pArgv[argIndex + 1];
        argIndex += 2;
        argsRemaining -= 2;
      }
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentSubSections, sizeof(ArgumentSubSections)) == 0)
      {
        subSections = atoi(pArgv[argIndex + 1]);

        if (subSections <= 0)
        {
          puts("Invalid Parameter.");
          return 1;
        }

        argIndex += 2;
        argsRemaining -= 2;
      }
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentRuns, sizeof(ArgumentRuns)) == 0)
      {
        runs = atoi(pArgv[argIndex + 1]);

        if (runs <= 0)
        {
          puts("Invalid Parameter.");
          return 1;
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
        lowEntropyMode = true;
        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentSingle, sizeof(ArgumentSingle)) == 0)
      {
        singleSymbol = true;
        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentUltra, sizeof(ArgumentUltra)) == 0)
      {
        lowEntropyShortMode = true;
        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtreme, sizeof(ArgumentExtreme)) == 0)
      {
        extremeMode = true;
        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 2 && strncmp(pArgv[argIndex], ArgumentExtremeSize, sizeof(ArgumentExtremeSize)) == 0)
      {
        if (!extremeMode && !mmtfMode && !extremeMmtfMode)
        {
          printf("Invalid Parameter. Expected '%s', '%s' or '%s' with '%s'.", ArgumentExtreme, ArgumentMMTF, ArgumentExtremeMMTF, ArgumentExtremeSize);
          return 1;
        }
        
        switch (pArgv[argIndex + 1][0])
        {
        case '8':
          extremeSize = 8;
          break;

        case '1':
          switch (pArgv[argIndex + 1][1])
          {
          case '6':
            extremeSize = 16;
            break;

          case '2':
            extremeSize = 128;
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
            extremeSize = 24;
            break;

          case '5':
            extremeSize = 256;
            break;

          default:
            puts("Invalid Parameter.");
            return 1;
          }
          break;

        case '3':
          extremeSize = 32;
          break;

        case '4':
          extremeSize = 48;
          break;

        case '6':
          extremeSize = 64;
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
        if (!extremeMode)
        {
          printf("Invalid Parameter. Expected '%s' with '%s'.", ArgumentExtreme, ArgumentExtremeByteGran);
          return 1;
        }

        if (extremeSize != 16 && extremeSize != 24 && extremeSize != 32 && extremeSize != 48 && extremeSize != 64 && extremeSize != 128)
        {
          printf("Not Supported. Expected '%s %s [16 / 24 / 32 / 48 / 64 / 128]' with '%s'.", ArgumentExtreme, ArgumentExtremeSize, ArgumentExtremeByteGran);
          return 1;
        }

        byteGranular = true;
        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremePacked, sizeof(ArgumentExtremePacked)) == 0)
      {
        if (!extremeMode)
        {
          printf("Invalid Parameter. Expected '%s' with '%s'.", ArgumentExtreme, ArgumentExtremePacked);
          return 1;
        }

        if (extremeSize != 8 && extremeSize != 16 && extremeSize != 24 && extremeSize != 32 && extremeSize != 48 && extremeSize != 64 && extremeSize != 128)
        {
          printf("Not Supported. Expected '%s %s [8 / 16 / 24 / 32 / 48 / 64 / 128]' with '%s'.", ArgumentExtreme, ArgumentExtremeSize, ArgumentExtremePacked);
          return 1;
        }

        packed = true;
        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentMMTF, sizeof(ArgumentMMTF)) == 0)
      {
        mmtfMode = true;
        extremeSize = 128;
        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentSH, sizeof(ArgumentSH)) == 0)
      {
        shMode = true;
        argIndex += 1;
        argsRemaining -= 1;
      }
      else if (argsRemaining >= 1 && strncmp(pArgv[argIndex], ArgumentExtremeMMTF, sizeof(ArgumentExtremeMMTF)) == 0)
      {
        extremeMmtfMode = true;
        extremeSize = 128;
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
        puts("Invalid Parameter.");
        return 1;
      }
    }
  }

#ifdef _WIN32
  // For more consistent benchmarking results.
  HANDLE thread = GetCurrentThread();
  SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
  SetThreadAffinityMask(thread, (uint64_t)1 << cpuCoreIndex);
#endif

  // Validate Parameters.
  {
    if (singleSymbol && subSections != 0)
    {
      puts("Single Symbol Encoding is only available without sub sections.");
      return 1;
    }

    if (lowEntropyShortMode && subSections != 0)
    {
      puts("Ultra Mode Encoding is only available without sub sections.");
      return 1;
    }

    if (extremeMode && subSections != 0)
    {
      puts("Extreme Mode Encoding is only available without sub sections.");
      return 1;
    }

    if (lowEntropyMode + lowEntropyShortMode + extremeMode + mmtfMode + extremeMmtfMode + shMode > 1)
    {
      puts("Normal, Extreme, Ultra and MMTF cannot be used at the same time.");
      return 1;
    }

    if (extremeMode && singleSymbol && extremeSize != 8)
    {
      puts("Single Symbol in Extreme Mode is only supported for symbol size 8.");
      return 1;
    }

    if (extremeMode && extremeSize == 256)
    {
      puts("Extreme Mode doesn't support symbol size 256.");
      return 1;
    }

    if ((mmtfMode || extremeMmtfMode) && extremeSize != 128 && extremeSize != 256)
    {
      puts("MMTF Modes only supports mtf width of 128 or 256.");
      return 1;
    }

    benchmarkAll = !lowEntropyMode && !lowEntropyShortMode && !extremeMode && !mmtfMode && !extremeMmtfMode && !shMode;
  }

  size_t fileSize = 0;
  uint32_t compressedBufferSize = 0;
  uint8_t *pUncompressedData = NULL;
  uint8_t *pDecompressedData = NULL;
  uint8_t *pCompressedData = NULL;

  FILE *pFile = fopen(pArgv[1], "rb");

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

  compressedBufferSize = max(compressedBufferSize, rle_mmtf_bounds((uint32_t)fileSize));
  compressedBufferSize = max(compressedBufferSize, rle8_sh_bounds((uint32_t)fileSize));
  compressedBufferSize = max(compressedBufferSize, rle8_mmtf128_compress_bounds((uint32_t)fileSize));
  compressedBufferSize = max(compressedBufferSize, rle8_mmtf256_compress_bounds((uint32_t)fileSize));
  compressedBufferSize = max(compressedBufferSize, rle8_low_entropy_compress_bounds((uint32_t)fileSize));
  compressedBufferSize = max(compressedBufferSize, rle8_low_entropy_short_compress_bounds((uint32_t)fileSize));
  
  if (subSections != 0)
    compressedBufferSize = max(compressedBufferSize, rle8m_compress_bounds((uint32_t)subSections, (uint32_t)fileSize));

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

  //////////////////////////////////////////////////////////////////////////

  if (benchmarkAll)
  {
    enum
    {
      Extreme8,
      Extreme8Short,
      Extreme8Packed,
      Extreme8_1SLShort,
      Extreme8_3SL,
      Extreme8_3SLShort,
      Extreme8_7SL,
      Extreme8_7SLShort,
      Extreme8Single,
      Extreme8SingleShort,
      Extreme8PackedSingle,
      Extreme16Sym,
      Extreme16SymPacked,
      Extreme16SymShort,
      Extreme16Sym_1SLShort,
      Extreme16Sym_3SL,
      Extreme16Sym_3SLShort,
      Extreme16Sym_7SL,
      Extreme16Sym_7SLShort,
      Extreme16Byte,
      Extreme16BytePacked,
      Extreme16ByteShort,
      Extreme16Byte_1SLShort,
      Extreme16Byte_3SL,
      Extreme16Byte_3SLShort,
      Extreme16Byte_7SL,
      Extreme16Byte_7SLShort,
      Extreme24Sym,
      Extreme24SymPacked,
      Extreme24Byte,
      Extreme24BytePacked,
      Extreme32Sym,
      Extreme32SymPacked,
      Extreme32SymShort,
      Extreme32Sym_1SLShort,
      Extreme32Sym_3SL,
      Extreme32Sym_3SLShort,
      Extreme32Sym_7SL,
      Extreme32Sym_7SLShort,
      Extreme32Byte,
      Extreme32BytePacked,
      Extreme32ByteShort,
      Extreme32Byte_1SLShort,
      Extreme32Byte_3SL,
      Extreme32Byte_3SLShort,
      Extreme32Byte_7SL,
      Extreme32Byte_7SLShort,
      Extreme48Sym,
      Extreme48SymPacked,
      Extreme48Byte,
      Extreme48BytePacked,
      Extreme64Sym,
      Extreme64SymPacked,
      Extreme64SymShort,
      Extreme64Sym_1SLShort,
      Extreme64Sym_3SL,
      Extreme64Sym_3SLShort,
      Extreme64Sym_7SL,
      Extreme64Sym_7SLShort,
      Extreme64Byte,
      Extreme64BytePacked,
      Extreme64ByteShort,
      Extreme64Byte_1SLShort,
      Extreme64Byte_3SL,
      Extreme64Byte_3SLShort,
      Extreme64Byte_7SL,
      Extreme64Byte_7SLShort,
      Extreme128Sym,
      Extreme128SymPacked,
      Extreme128Byte,
      Extreme128BytePacked,
      Rle8SH,
      Extreme8MultiMTF128,
      LowEntropy,
      LowEntropySingle,
      LowEntropyShort,
      LowEntropyShortSingle,

      MultiMTF128,
      MultiMTF256,

      MemCopy,

      CodecCount
    } currentCodec = 0;

    const char *codecNames[] = 
    {
      "8 Bit                         ",
      "8 Bit Short                   ",
      "8 Bit Packed                  ",
      "8 Bit 1LUT Short              ",
      "8 Bit 3LUT                    ",
      "8 Bit 3LUT Short              ",
      "8 Bit 7LUT                    ",
      "8 Bit 7LUT Short              ",
      "8 Bit Single                  ",
      "8 Bit Single Short            ",
      "8 Bit Single Packed           ",
      "16 Bit (Symbol)               ",
      "16 Bit Packed (Symbol)        ",
      "16 Bit Short (Symbol)         ",
      "16 Bit 1LUT Short (Symbol)    ",
      "16 Bit 3LUT (Symbol)          ",
      "16 Bit 3LUT Short (Symbol)    ",
      "16 Bit 7LUT (Symbol)          ",
      "16 Bit 7LUT Short (Symbol)    ",
      "16 Bit (Byte)                 ",
      "16 Bit Packed (Byte)          ",
      "16 Bit Short (Byte)           ",
      "16 Bit 1LUT Short (Byte)      ",
      "16 Bit 3LUT (Byte)            ",
      "16 Bit 3LUT Short (Byte)      ",
      "16 Bit 7LUT (Byte)            ",
      "16 Bit 7LUT Short (Byte)      ",
      "24 Bit (Symbol)               ",
      "24 Bit Packed (Symbol)        ",
      "24 Bit (Byte)                 ",
      "24 Bit Packed (Byte)          ",
      "32 Bit (Symbol)               ",
      "32 Bit Packed (Symbol)        ",
      "32 Bit Short (Symbol)         ",
      "32 Bit 1LUT Short (Symbol)    ",
      "32 Bit 3LUT (Symbol)          ",
      "32 Bit 3LUT Short (Symbol)    ",
      "32 Bit 7LUT (Symbol)          ",
      "32 Bit 7LUT Short (Symbol)    ",
      "32 Bit (Byte)                 ",
      "32 Bit Packed (Byte)          ",
      "32 Bit Short (Byte)           ",
      "32 Bit 1LUT Short (Byte)      ",
      "32 Bit 3LUT (Byte)            ",
      "32 Bit 3LUT Short (Byte)      ",
      "32 Bit 7LUT (Byte)            ",
      "32 Bit 7LUT Short (Byte)      ",
      "48 Bit (Symbol)               ",
      "48 Bit Packed (Symbol)        ",
      "48 Bit (Byte)                 ",
      "48 Bit Packed (Byte)          ",
      "64 Bit (Symbol)               ",
      "64 Bit Packed (Symbol)        ",
      "64 Bit Short (Symbol)         ",
      "64 Bit 1LUT Short (Symbol)    ",
      "64 Bit 3LUT (Symbol)          ",
      "64 Bit 3LUT Short (Symbol)    ",
      "64 Bit 7LUT (Symbol)          ",
      "64 Bit 7LUT Short (Symbol)    ",
      "64 Bit (Byte)                 ",
      "64 Bit Packed (Byte)          ",
      "64 Bit Short (Byte)           ",
      "64 Bit 1LUT Short (Byte)      ",
      "64 Bit 3LUT (Byte)            ",
      "64 Bit 3LUT Short (Byte)      ",
      "64 Bit 7LUT (Byte)            ",
      "64 Bit 7LUT Short (Byte)      ",
      "128 Bit (Symbol)              ",
      "128 Bit Packed (Symbol)       ",
      "128 Bit (Byte)                ",
      "128 Bit Packed (Byte)         ",
      "8 Bit RLE + Huffman-esque     ",
      "8 Bit MMTF 128                ",
      "Low Entropy                   ",
      "Low Entropy Single            ",
      "Low Entropy Short             ",
      "Low Entropy Short Single      ",
      "Multi MTF 128 Bit (Transform) ",
      "Multi MTF 256 Bit (Transform) ",
      "memcpy                        ",
    };

    _STATIC_ASSERT(ARRAYSIZE(codecNames) == CodecCount);

    uint32_t fileSize32 = (uint32_t)fileSize;

    printf("\nBenchmarking File '%s' (%" PRIu64 " Bytes)\n\n"
      "Codec                           Ratio      Encoder Throughput (Maximum)    Decoder Throughput (Maximum)    R*H/log2(|S|)\n"
      "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", pArgv[1], fileSize);

    for (; currentCodec < CodecCount; currentCodec++)
    {
      if (currentCodec > 0)
        SleepNs(500 * 1000 * 1000);

      printf("%s|          | (dry run)", codecNames[currentCodec]);

      uint64_t compressionTime = 0;
      uint64_t fastestCompresionTime = UINT64_MAX;
      int64_t compressionRuns = -1;
      uint32_t compressedSize = 0;

      const uint64_t compressStartTicks = GetCurrentTimeTicks();
      uint64_t lastSleepTicks = compressStartTicks;

      while (compressionRuns < (int64_t)runs || TicksToNs(GetCurrentTimeTicks() - compressStartTicks) < 1000000000 * (uint64_t)minSeconds)
      {
        uint64_t runTime = GetCurrentTimeTicks();

        switch (currentCodec)
        {
        case Extreme8:
          compressedSize = rle8_multi_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme8Single:
          compressedSize = rle8_single_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme16Sym:
          compressedSize = rle16_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme24Sym:
          compressedSize = rle24_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme32Sym:
          compressedSize = rle32_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme48Sym:
          compressedSize = rle48_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme64Sym:
          compressedSize = rle64_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme128Sym:
          compressedSize = rle128_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16SymPacked:
          compressedSize = rle16_sym_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme24SymPacked:
          compressedSize = rle24_sym_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32SymPacked:
          compressedSize = rle32_sym_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme48SymPacked:
          compressedSize = rle48_sym_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64SymPacked:
          compressedSize = rle64_sym_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme128SymPacked:
          compressedSize = rle128_sym_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme16Byte:
          compressedSize = rle16_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme24Byte:
          compressedSize = rle24_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme32Byte:
          compressedSize = rle32_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme48Byte:
          compressedSize = rle48_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme64Byte:
          compressedSize = rle64_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme128Byte:
          compressedSize = rle128_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme8Packed:
          compressedSize = rle8_packed_multi_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme8PackedSingle:
          compressedSize = rle8_packed_single_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme16BytePacked:
          compressedSize = rle16_byte_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme24BytePacked:
          compressedSize = rle24_byte_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme32BytePacked:
          compressedSize = rle32_byte_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme48BytePacked:
          compressedSize = rle48_byte_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme64BytePacked:
          compressedSize = rle64_byte_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;
    
        case Extreme128BytePacked:
          compressedSize = rle128_byte_packed_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme8Short:
          compressedSize = rle8_multi_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme8SingleShort:
          compressedSize = rle8_single_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme8_1SLShort:
          compressedSize = rle8_1symlut_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme8_3SL:
          compressedSize = rle8_3symlut_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme8_3SLShort:
          compressedSize = rle8_3symlut_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme8_7SL:
          compressedSize = rle8_7symlut_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme8_7SLShort:
          compressedSize = rle8_7symlut_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16SymShort:
          compressedSize = rle16_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_1SLShort:
          compressedSize = rle16_1symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_3SL:
          compressedSize = rle16_3symlut_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_3SLShort:
          compressedSize = rle16_3symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_7SL:
          compressedSize = rle16_7symlut_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_7SLShort:
          compressedSize = rle16_7symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16ByteShort:
          compressedSize = rle16_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_1SLShort:
          compressedSize = rle16_1symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_3SL:
          compressedSize = rle16_3symlut_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_3SLShort:
          compressedSize = rle16_3symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_7SL:
          compressedSize = rle16_7symlut_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_7SLShort:
          compressedSize = rle16_7symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32SymShort:
          compressedSize = rle32_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_1SLShort:
          compressedSize = rle32_1symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_3SL:
          compressedSize = rle32_3symlut_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_3SLShort:
          compressedSize = rle32_3symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_7SL:
          compressedSize = rle32_7symlut_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_7SLShort:
          compressedSize = rle32_7symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32ByteShort:
          compressedSize = rle32_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_1SLShort:
          compressedSize = rle32_1symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_3SL:
          compressedSize = rle32_3symlut_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_3SLShort:
          compressedSize = rle32_3symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_7SL:
          compressedSize = rle32_7symlut_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_7SLShort:
          compressedSize = rle32_7symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64SymShort:
          compressedSize = rle64_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_1SLShort:
          compressedSize = rle64_1symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_3SL:
          compressedSize = rle64_3symlut_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_3SLShort:
          compressedSize = rle64_3symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_7SL:
          compressedSize = rle64_7symlut_sym_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_7SLShort:
          compressedSize = rle64_7symlut_sym_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64ByteShort:
          compressedSize = rle64_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_1SLShort:
          compressedSize = rle64_1symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_3SL:
          compressedSize = rle64_3symlut_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_3SLShort:
          compressedSize = rle64_3symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_7SL:
          compressedSize = rle64_7symlut_byte_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_7SLShort:
          compressedSize = rle64_7symlut_byte_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Extreme8MultiMTF128:
          compressedSize = rle8_mmtf128_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case MultiMTF128:
          compressedSize = rle_mmtf128_encode(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case MultiMTF256:
          compressedSize = rle_mmtf256_encode(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case Rle8SH:
          compressedSize = rle8_sh_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case LowEntropy:
          compressedSize = rle8_low_entropy_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case LowEntropySingle:
          compressedSize = rle8_low_entropy_compress_only_max_frequency(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case LowEntropyShort:
          compressedSize = rle8_low_entropy_short_compress(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        case LowEntropyShortSingle:
          compressedSize = rle8_low_entropy_short_compress_only_max_frequency(pUncompressedData, fileSize32, pCompressedData, compressedBufferSize);
          break;

        default:
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

        compressionRuns++;

        if (compressionRuns > 0)
          printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s)", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1000000000.0), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1000000000.0));

        const uint64_t sinceSleepNs = TicksToNs(GetCurrentTimeTicks() - lastSleepTicks);

        if (sinceSleepNs > 500 * 1000 * 1000) // Prevent thermal saturation.
        {
          SleepNs(min(sinceSleepNs / 4, 2 * 1000 * 1000 * 1000));
          lastSleepTicks = GetCurrentTimeTicks();
        }
      }

      if (compressedSize == 0)
      {
        printf("\r%s| <FAILED TO COMRPESS>\n", codecNames[currentCodec]);
        continue;
      }

      int64_t decompressionRuns = -1;
      uint64_t decompressionTime = 0;
      uint64_t fastestDecompresionTime = UINT64_MAX;
      uint32_t decompressedSize = 0;

      printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | (dry run)", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1000000000.0), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1000000000.0));

      SleepNs(500 * 1000 * 1000);

      const uint64_t decompressStartTicks = GetCurrentTimeTicks();

      while (decompressionRuns < (int64_t)runs || TicksToNs(GetCurrentTimeTicks() - decompressStartTicks) < 1000000000 * (uint64_t)minSeconds)
      {
        uint64_t runTime = GetCurrentTimeTicks();

        switch (currentCodec)
        {
        case Extreme8:
        case Extreme8Single:
          decompressedSize = rle8_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;
     
        case Extreme16Sym:
          decompressedSize = rle16_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;
     
        case Extreme24Sym:
          decompressedSize = rle24_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;
     
        case Extreme32Sym:
          decompressedSize = rle32_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;
     
        case Extreme48Sym:
          decompressedSize = rle48_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;
     
        case Extreme64Sym:
          decompressedSize = rle64_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;
     
        case Extreme128Sym:
          decompressedSize = rle128_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16SymPacked:
          decompressedSize = rle16_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme24SymPacked:
          decompressedSize = rle24_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32SymPacked:
          decompressedSize = rle32_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme48SymPacked:
          decompressedSize = rle48_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64SymPacked:
          decompressedSize = rle64_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme128SymPacked:
          decompressedSize = rle128_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Byte:
          decompressedSize = rle16_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme24Byte:
          decompressedSize = rle24_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Byte:
          decompressedSize = rle32_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme48Byte:
          decompressedSize = rle48_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Byte:
          decompressedSize = rle64_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme128Byte:
          decompressedSize = rle128_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8Packed:
        case Extreme8PackedSingle:
          decompressedSize = rle8_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16BytePacked:
          decompressedSize = rle16_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme24BytePacked:
          decompressedSize = rle24_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32BytePacked:
          decompressedSize = rle32_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme48BytePacked:
          decompressedSize = rle48_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64BytePacked:
          decompressedSize = rle64_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme128BytePacked:
          decompressedSize = rle128_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8Short:
          decompressedSize = rle8_multi_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8SingleShort:
          decompressedSize = rle8_single_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8_1SLShort:
          decompressedSize = rle8_1symlut_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8_3SL:
          decompressedSize = rle8_3symlut_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8_3SLShort:
          decompressedSize = rle8_3symlut_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8_7SL:
          decompressedSize = rle8_7symlut_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8_7SLShort:
          decompressedSize = rle8_7symlut_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16SymShort:
          decompressedSize = rle16_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_1SLShort:
          decompressedSize = rle16_1symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_3SL:
          decompressedSize = rle16_3symlut_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_3SLShort:
          decompressedSize = rle16_3symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_7SL:
          decompressedSize = rle16_7symlut_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Sym_7SLShort:
          decompressedSize = rle16_7symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16ByteShort:
          decompressedSize = rle16_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_1SLShort:
          decompressedSize = rle16_1symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_3SL:
          decompressedSize = rle16_3symlut_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_3SLShort:
          decompressedSize = rle16_3symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_7SL:
          decompressedSize = rle16_7symlut_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme16Byte_7SLShort:
          decompressedSize = rle16_7symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32SymShort:
          decompressedSize = rle32_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_1SLShort:
          decompressedSize = rle32_1symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_3SL:
          decompressedSize = rle32_3symlut_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_3SLShort:
          decompressedSize = rle32_3symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_7SL:
          decompressedSize = rle32_7symlut_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Sym_7SLShort:
          decompressedSize = rle32_7symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32ByteShort:
          decompressedSize = rle32_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_1SLShort:
          decompressedSize = rle32_1symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_3SL:
          decompressedSize = rle32_3symlut_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_3SLShort:
          decompressedSize = rle32_3symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_7SL:
          decompressedSize = rle32_7symlut_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme32Byte_7SLShort:
          decompressedSize = rle32_7symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64SymShort:
          decompressedSize = rle64_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_1SLShort:
          decompressedSize = rle64_1symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_3SL:
          decompressedSize = rle64_3symlut_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_3SLShort:
          decompressedSize = rle64_3symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_7SL:
          decompressedSize = rle64_7symlut_sym_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Sym_7SLShort:
          decompressedSize = rle64_7symlut_sym_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64ByteShort:
          decompressedSize = rle64_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_1SLShort:
          decompressedSize = rle64_1symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_3SL:
          decompressedSize = rle64_3symlut_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_3SLShort:
          decompressedSize = rle64_3symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_7SL:
          decompressedSize = rle64_7symlut_byte_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme64Byte_7SLShort:
          decompressedSize = rle64_7symlut_byte_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Extreme8MultiMTF128:
          decompressedSize = rle8_mmtf128_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case MultiMTF128:
          decompressedSize = rle_mmtf128_decode(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case MultiMTF256:
          decompressedSize = rle_mmtf256_decode(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case Rle8SH:
          decompressedSize = rle8_sh_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case LowEntropy:
        case LowEntropySingle:
          decompressedSize = rle8_low_entropy_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        case LowEntropyShort:
        case LowEntropyShortSingle:
          decompressedSize = rle8_low_entropy_short_decompress(pCompressedData, compressedSize, pDecompressedData, compressedBufferSize);
          break;

        default:
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

        decompressionRuns++;

        if (decompressionRuns > 0)
          printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %7.1f MiB/s (%7.1f MiB/s)", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1000000000.0), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1000000000.0), (fileSize * (double)decompressionRuns / (double)(1024 * 1024)) / (decompressionTime / 1000000000.0), (fileSize / (double)(1024 * 1024)) / (fastestDecompresionTime / 1000000000.0));

        const uint64_t sinceSleepNs = TicksToNs(GetCurrentTimeTicks() - lastSleepTicks);

        if (sinceSleepNs > 500 * 1000 * 1000) // Prevent thermal saturation.
        {
          SleepNs(min(sinceSleepNs / 4, 2 * 1000 * 1000 * 1000));
          lastSleepTicks = GetCurrentTimeTicks();
        }
      }

      if (decompressedSize == 0)
      {
        printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | <FAILED TO DECOMRPESS>\n", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1000000000.0), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1000000000.0));

        continue;
      }

      printf("\r%s| %6.2f %% | %7.1f MiB/s (%7.1f MiB/s) | %7.1f MiB/s (%7.1f MiB/s) | %11.7f %%", codecNames[currentCodec], compressedSize / (double)fileSize * 100.0, (fileSize * (double)compressionRuns / (double)(1024 * 1024)) / (compressionTime / 1000000000.0), (fileSize / (double)(1024 * 1024)) / (fastestCompresionTime / 1000000000.0), (fileSize * (double)decompressionRuns / (double)(1024 * 1024)) / (decompressionTime / 1000000000.0), (fileSize / (double)(1024 * 1024)) / (fastestDecompresionTime / 1000000000.0), ((compressedSize / (double)fileSize) * (GetInformationRatio(pCompressedData, compressedSize))) * 100.0f);

      puts("");

      Validate(pUncompressedData, pDecompressedData, fileSize);
    }
  }
  else // !benchmarkAll
  {
    // Print Codec Description.
    if (!benchmarkAll)
    {
      printf("Mode: hypersonic rle kit ");

      if (lowEntropyMode)
        fputs("Normal ", stdout);
      else if (extremeMode)
        fputs("Extreme ", stdout);
      else if (mmtfMode)
        fputs("MMTF ", stdout);
      else if (extremeMmtfMode)
        fputs("Exreme MMTF ", stdout);
      else if (shMode)
        fputs("SH ", stdout);
      else
        fputs("Ultra ", stdout);

      if ((!extremeMode || extremeSize == 8) && singleSymbol)
        fputs("Single-Symbol-Mode ", stdout);

      if (extremeMode && packed)
        fputs("Packed ", stdout);

      if (extremeMode && byteGranular && extremeSize != 8)
        fputs("Unbound ", stdout);

      if (extremeMode)
        printf("with %" PRIu64 " Bit Symbols ", extremeSize);
      else if (mmtfMode || extremeMmtfMode)
        printf("with %" PRIu64 " Bit width ", extremeSize);

      printf("(%" PRIi32 " Run%s)\n\n", runs, runs > 1 ? "s" : "");
    }

    uint32_t decompressedSize = 0;
    uint32_t compressedSize = 0;

    uint64_t subTimeMin = UINT64_MAX;
    uint64_t subTimeMax = 0;

    uint64_t time = GetCurrentTimeTicks();

    for (int32_t i = 0; i < runs; i++)
    {
      uint64_t subTime = GetCurrentTimeTicks();

      if (subSections == 0)
      {
        if (lowEntropyMode)
        {
          if (singleSymbol)
            compressedSize = rle8_low_entropy_compress_only_max_frequency(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
          else
            compressedSize = rle8_low_entropy_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
        }
        else if (lowEntropyShortMode)
        {
          if (singleSymbol)
            compressedSize = rle8_low_entropy_short_compress_only_max_frequency(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
          else
            compressedSize = rle8_low_entropy_short_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
        }
        else if (extremeMode)
        {
          if (!byteGranular)
          {
            if (packed)
            {
              switch (extremeSize)
              {
              default:
              case 8:
                if (singleSymbol)
                  compressedSize = rle8_packed_single_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                else
                  compressedSize = rle8_packed_multi_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 16:
                compressedSize = rle16_sym_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 24:
                compressedSize = rle24_sym_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 32:
                compressedSize = rle32_sym_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 48:
                compressedSize = rle48_sym_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 64:
                compressedSize = rle64_sym_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 128:
                compressedSize = rle128_sym_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;
              }
            }
            else
            {
              switch (extremeSize)
              {
              case 8:
              default:
                if (singleSymbol)
                  compressedSize = rle8_single_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                else
                  compressedSize = rle8_multi_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 16:
                compressedSize = rle16_sym_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 24:
                compressedSize = rle24_sym_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 32:
                compressedSize = rle32_sym_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 48:
                compressedSize = rle48_sym_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 64:
                compressedSize = rle64_sym_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 128:
                compressedSize = rle128_sym_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;
              }
            }
          }
          else
          {
            if (packed)
            {
              switch (extremeSize)
              {
              default:
              case 8:
                if (singleSymbol)
                  compressedSize = rle8_packed_single_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                else
                  compressedSize = rle8_packed_multi_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 16:
                compressedSize = rle16_byte_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 24:
                compressedSize = rle24_byte_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 32:
                compressedSize = rle32_byte_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 48:
                compressedSize = rle48_byte_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 64:
                compressedSize = rle64_byte_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 128:
                compressedSize = rle128_byte_packed_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;
              }
            }
            else
            {
              switch (extremeSize)
              {
              default:
              case 16:
                compressedSize = rle16_byte_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 24:
                compressedSize = rle24_byte_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 32:
                compressedSize = rle32_byte_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 48:
                compressedSize = rle48_byte_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 64:
                compressedSize = rle64_byte_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;

              case 128:
                compressedSize = rle128_byte_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
                break;
              }
            }
          }
        }
        else if (mmtfMode)
        {
          switch (extremeSize)
          {
          case 128:
            compressedSize = rle_mmtf128_encode(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
            break;

          case 256:
            compressedSize = rle_mmtf256_encode(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
            break;
          }
        }
        else if (extremeMmtfMode)
        {
          switch (extremeSize)
          {
          case 128:
            compressedSize = rle8_mmtf128_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
            break;

          //case 256:
          //  compressedSize = rle8_mmtf256_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
          //  break;
          }
        }
        else if (shMode)
        {
          compressedSize = rle8_sh_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
        }
      }
      else
      {
        compressedSize = rle8m_compress((uint32_t)subSections, pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
      }

      subTime = TicksToNs(GetCurrentTimeTicks() - subTime);

      if (subTime < subTimeMin)
        subTimeMin = subTime;

      if (subTime > subTimeMax)
        subTimeMax = subTime;
    }

    time = TicksToNs(GetCurrentTimeTicks() - time);

    if (0 == compressedSize)
    {
      puts("Failed to compress file.");
      goto epilogue;
    }

    printf("Compressed %" PRIu64 " bytes -> %" PRIu32 " bytes (%f %%) in %f ms. (=> %f MB/s)\n", fileSize, compressedSize, (double)compressedSize / (double)fileSize * 100.0, time / (double)runs / 1000000.0, (fileSize / (1024.0 * 1024.0)) / (time / (double)runs / 1000000000.0));

    if (runs > 1)
      printf(" [%f ms .. %f ms | %f MB/s .. %f MB/s]\n\n", subTimeMin / 1000000.0, subTimeMax / 1000000.0, (fileSize / (1024.0 * 1024.0)) / (subTimeMax / 1000000000.0), (fileSize / (1024.0 * 1024.0)) / (subTimeMin / 1000000000.0));

    if (outputFileName)
    {
      FILE *pCompressed = fopen(outputFileName, "wb");
      
      if (!pCompressed)
      {
        puts("Failed to open file for writing.");
        fclose(pCompressed);
        goto epilogue;
      }

      if (compressedSize != fwrite(pCompressedData, 1, compressedSize, pCompressed))
      {
        puts("Failed to write to file.");
        fclose(pCompressed);
        goto epilogue;
      }

      fclose(pCompressed);
    }

    // Scramble data outside the claimed length, to ensure the compressors are telling the truth about the claimed length.
    for (size_t i = compressedSize; i < compressedBufferSize; i++)
      pCompressedData[i] = ~pCompressedData[i];

    subTimeMin = UINT64_MAX;
    subTimeMax = 0;

    time = GetCurrentTimeTicks();

    for (int32_t i = 0; i < runs; i++)
    {
      uint64_t subTime = GetCurrentTimeTicks();

      if (subSections == 0)
      {
        if (lowEntropyMode)
        {
          decompressedSize = rle8_low_entropy_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
        }
        else if (lowEntropyShortMode)
        {
          decompressedSize = rle8_low_entropy_short_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
        }
        else if (extremeMode)
        {
          if (!byteGranular)
          {
            if (packed)
            {
              switch (extremeSize)
              {
              default:
              case 8:
                decompressedSize = rle8_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 16:
                decompressedSize = rle16_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 24:
                decompressedSize = rle24_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 32:
                decompressedSize = rle32_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 48:
                decompressedSize = rle48_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 64:
                decompressedSize = rle64_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 128:
                decompressedSize = rle128_sym_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;
              }
            }
            else
            {
              switch (extremeSize)
              {
              case 8:
              default:
                decompressedSize = rle8_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 16:
                decompressedSize = rle16_sym_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 24:
                decompressedSize = rle24_sym_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 32:
                decompressedSize = rle32_sym_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 48:
                decompressedSize = rle48_sym_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 64:
                decompressedSize = rle64_sym_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 128:
                decompressedSize = rle128_sym_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;
              }
            }
          }
          else
          {
            if (packed)
            {
              switch (extremeSize)
              {
              default:
              case 8:
                decompressedSize = rle8_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 16:
                decompressedSize = rle16_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 24:
                decompressedSize = rle24_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 32:
                decompressedSize = rle32_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 48:
                decompressedSize = rle48_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 64:
                decompressedSize = rle64_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 128:
                decompressedSize = rle128_byte_packed_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;
              }
            }
            else
            {
              switch (extremeSize)
              {
              default:
              case 16:
                decompressedSize = rle16_byte_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 24:
                decompressedSize = rle24_byte_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 32:
                decompressedSize = rle32_byte_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 48:
                decompressedSize = rle48_byte_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 64:
                decompressedSize = rle64_byte_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;

              case 128:
                decompressedSize = rle128_byte_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
                break;
              }
            }
          }
        }
        else if (mmtfMode)
        {
          switch (extremeSize)
          {
          case 128:
            decompressedSize = rle_mmtf128_decode(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
            break;

          case 256:
            decompressedSize = rle_mmtf256_decode(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
            break;
          }
        }
        else if (extremeMmtfMode)
        {
          switch (extremeSize)
          {
          case 128:
            decompressedSize = rle8_mmtf128_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
            break;

          //case 256:
          //  decompressedSize = rle8_mmtf256_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
          //  break;
          }
        }
        else if (shMode)
        {
          decompressedSize = rle8_sh_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
        }
      }
      else
      {
        decompressedSize = rle8m_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
      }

      subTime = TicksToNs(GetCurrentTimeTicks() - subTime);

      if (subTime < subTimeMin)
        subTimeMin = subTime;

      if (subTime > subTimeMax)
        subTimeMax = subTime;
    }

    time = TicksToNs(GetCurrentTimeTicks() - time);

    if ((uint32_t)fileSize != decompressedSize)
    {
      puts("Failed to decompress file.");
      goto epilogue;
    }
    
    printf("Decompressed in %f ms. (=> %f MB/s)\n", time / (double)runs / 1000000.0, (fileSize / (1024.0 * 1024.0)) / (time / (double)runs / 1000000000.0));

    if (runs > 1)
      printf(" [%f ms .. %f ms | %f MB/s .. %f MB/s]\n\n", subTimeMin / 1000000.0, subTimeMax / 1000000.0, (fileSize / (1024.0 * 1024.0)) / (subTimeMax / 1000000000.0), (fileSize / (1024.0 * 1024.0)) / (subTimeMin / 1000000000.0));

    if (!Validate(pUncompressedData, pDecompressedData, fileSize))
    {
      puts("Validation Failed.");
      goto epilogue;
    }

#ifdef BUILD_WITH_OPENCL
    if (lowEntropyMode && subSections > 0)
    {
      memset(pDecompressedData, 0, fileSize);

      if (!rle8m_opencl_init(fileSize, compressedSize, subSections))
      {
        puts("Initialization Failed (OpenCL).");
        goto epilogue;
      }

      time = GetCurrentTimeTicks();

      for (int32_t i = 0; i < runs; i++)
        decompressedSize = rle8m_opencl_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);

      time = TicksToNs(GetCurrentTimeTicks() - time);

      rle8m_opencl_destroy();

      if ((uint32_t)fileSize != decompressedSize)
      {
        puts("Failed to decompress file (OpenCL).");
        goto epilogue;
      }

      printf("Decompressed in %f ms (OpenCL).\n", time / (double)runs / 1000000.0);

      if (!Validate(pUncompressedData, pDecompressedData, fileSize))
      {
        puts("Validation Failed.");
        goto epilogue;
      }
    }
#endif
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

bool Validate(const uint8_t *pUncompressedData, const uint8_t *pDecompressedData, const size_t fileSize)
{
  if (memcmp(pUncompressedData, pDecompressedData, (size_t)fileSize) != 0)
  {
    puts("Validation Failed.");

    for (size_t i = 0; i < fileSize; i++)
    {
      if (pUncompressedData[i] != pDecompressedData[i])
      {
        printf("First invalid char at %" PRIu64 " [0x%" PRIX64 "] (0x%" PRIX8 " != 0x%" PRIX8 ").\n", i, i, pUncompressedData[i], pDecompressedData[i]);

        const int64_t start = max(0, (int64_t)i - 64);
        const int64_t end = min((int64_t)fileSize, (int64_t)(i + 64));

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

          pRLE->copyBitsVsRleLengthBits[(max(0, min(lastNonLengthBits - 1, 15))) * 16 + (max(0, min(index - 1, 15)))]++;

          const int64_t copyLengthDiff = pRLE->lastNonLengthDiff;
          const int64_t lengthDiff = pRLE->currentLength - pRLE->lastLength;

          pRLE->copyDiffVsCountDiff[(max(0, min(31, copyLengthDiff + 15))) * 32 + (max(0, min(31, lengthDiff + 15)))]++;

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

            pRLE->alignedCopyBitsVsRleLengthBits[(max(0, min(lastNonLengthBits - 1, 15))) * 16 + (max(0, min(index - 1, 15)))]++;

            const int64_t copyLengthDiff = pRLE->alignedLastNonLengthDiff;
            const int64_t lengthDiff = pRLE->alignedCurrentLength - pRLE->alignedLastLength;

            pRLE->alignedCopyDiffVsCountDiff[(max(0, min(31, copyLengthDiff + 15))) * 32 + (max(0, min(31, lengthDiff + 15)))]++;

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