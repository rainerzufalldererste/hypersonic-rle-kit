#include "rle8.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

const char ArgumentTo[] = "-to";
const char ArgumentSubSections[] = "-s";
const char ArgumentRuns[] = "-r";

//////////////////////////////////////////////////////////////////////////

int main(int argc, char **pArgv)
{
  if (argc <= 1)
  {
    printf("Usage: rle8 <InputFileName> [%s <Output File Name>][%s <Sub Section Count>][%s <Run Count>]\n", ArgumentTo, ArgumentSubSections, ArgumentRuns);
    return 1;
  }

  const char *outputFileName = NULL;
  int32_t subSections = 0;
  int32_t runs = 1;

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
      else
      {
        puts("Invalid Parameter.");
        return 1;
      }
    }
  }

  int32_t fileSize = 0;
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

  if (subSections == 0)
    compressedBufferSize = rle8_compress_bounds((uint32_t)fileSize);
  else
    compressedBufferSize = rle8m_compress_bounds((uint32_t)subSections, (uint32_t)fileSize);

  pUncompressedData = (uint8_t *)malloc((size_t)fileSize);
  pDecompressedData = (uint8_t *)malloc((size_t)fileSize);
  pCompressedData = (uint8_t *)malloc((size_t)compressedBufferSize);

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

  // Compress.
  {
    uint32_t decompressedSize = 0;
    uint32_t compressedSize = 0;

    clock_t time = clock();

    for (int32_t i = 0; i < runs; i++)
    {
      if (subSections == 0)
        compressedSize = rle8_compress(pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
      else
        compressedSize = rle8m_compress((uint32_t)subSections, pUncompressedData, (uint32_t)fileSize, pCompressedData, compressedBufferSize);
    }

    time = clock() - time;

    if (0 == compressedSize)
    {
      puts("Failed to compress file.");
      goto epilogue;
    }

    printf("Compressed %" PRIi32 " bytes -> %" PRIu32 " bytes (%f %%) in %f ms.\n", fileSize, compressedSize, (double)compressedSize / (double)fileSize * 100.0, time / (double)runs / (double)CLOCKS_PER_SEC * 1000.0);

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

    time = clock();

    for (int32_t i = 0; i < runs; i++)
    {
      if (subSections == 0)
        decompressedSize = rle8_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
      else
        decompressedSize = rle8m_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);
    }

    time = clock() - time;

    if ((uint32_t)fileSize != decompressedSize)
    {
      puts("Failed to decompress file.");
      goto epilogue;
    }
    
    printf("Decompressed in %f ms.\n", time / (double)runs / (double)CLOCKS_PER_SEC * 1000.0);

    if (memcmp(pUncompressedData, pDecompressedData, (size_t)fileSize) != 0)
    {
      puts("Validation Failed.");
      goto epilogue;
    }

    memset(pDecompressedData, 0, fileSize);

    if (subSections > 0)
    {
      if (!rle8m_opencl_init(fileSize, compressedSize, subSections))
      {
        puts("Initialization Failed (OpenCL).");
        goto epilogue;
      }

      time = clock();

      for (int32_t i = 0; i < runs; i++)
        decompressedSize = rle8m_opencl_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)fileSize);

      time = clock() - time;

      rle8m_opencl_destroy();

      if ((uint32_t)fileSize != decompressedSize)
      {
        puts("Failed to decompress file (OpenCL).");
        goto epilogue;
      }

      printf("Decompressed in %f ms (OpenCL).\n", time / (double)runs / (double)CLOCKS_PER_SEC * 1000.0);

      if (memcmp(pUncompressedData, pDecompressedData, (size_t)fileSize) != 0)
      {
        puts("Validation Failed. (OpenCL)");
        goto epilogue;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////

  goto epilogue;

epilogue:
  fclose(pFile);
  free(pUncompressedData);
  free(pDecompressedData);
  free(pCompressedData);

  return 0;
}