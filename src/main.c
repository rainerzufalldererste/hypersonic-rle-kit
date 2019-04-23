#include "rle8.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

const char ArgumentTo[] = "-to";
const char ArgumentSubSections[] = "-s";

//////////////////////////////////////////////////////////////////////////

int main(int argc, char **pArgv)
{
  if (argc <= 1)
  {
    printf("Usage: rle8 <InputFileName> [%s <OutputFileName>][%s <SubSectionCount>]\n", ArgumentTo, ArgumentSubSections);
    return 1;
  }

  const char *outputFileName = NULL;
  int32_t subSections = 0;

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
      else
      {
        puts("Invalid Parameter.");
        return 1;
      }
    }
  }

  int32_t size = 0;
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
  size = ftell(pFile);

  if (size <= 0)
  {
    puts("Invalid File size / failed to read file.");
    goto epilogue;
  }

  fseek(pFile, 0, SEEK_SET);

  if (subSections == 0)
    compressedBufferSize = rle8_compress_bounds((uint32_t)size);
  else
    compressedBufferSize = rle8m_compress_bounds((uint32_t)subSections, (uint32_t)size);

  pUncompressedData = (uint8_t *)malloc((size_t)size);
  pDecompressedData = (uint8_t *)malloc((size_t)size);
  pCompressedData = (uint8_t *)malloc((size_t)compressedBufferSize);

  if (!pUncompressedData || !pDecompressedData || !pCompressedData)
  {
    puts("Failed to allocate memory.");
    goto epilogue;
  }

  if (size != fread(pUncompressedData, 1, (size_t)size, pFile))
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

    if (subSections == 0)
      compressedSize = rle8_compress(pUncompressedData, (uint32_t)size, pCompressedData, compressedBufferSize);
    else
      compressedSize = rle8m_compress((uint32_t)subSections, pUncompressedData, (uint32_t)size, pCompressedData, compressedBufferSize);

    time = clock() - time;

    if (0 == compressedSize)
    {
      puts("Failed to compress file.");
      goto epilogue;
    }

    printf("Compressed %" PRIi32 " bytes -> %" PRIu32 " bytes (%f %%) in %f ms.\n", size, compressedSize, (double)compressedSize / (double)size * 100.0, time / (double)CLOCKS_PER_SEC * 1000.0);

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

    if (subSections == 0)
      decompressedSize = rle8_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)size);
    else
      decompressedSize = rle8m_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)size);

    time = clock() - time;

    if ((uint32_t)size != decompressedSize)
    {
      puts("Failed to decompress file.");
      goto epilogue;
    }
    
    printf("Decompressed in %f ms.\n", time / (double)CLOCKS_PER_SEC * 1000.0);

    if (memcmp(pUncompressedData, pDecompressedData, (size_t)size) != 0)
    {
      puts("Validation Failed.");
      goto epilogue;
    }

    if (subSections > 0)
    {
      if (!rle8m_opencl_init(size, compressedSize, subSections))
      {
        puts("Initialization Failed (OpenCL).");
        goto epilogue;
      }

      time = clock();

      decompressedSize = rle8m_opencl_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)size);

      time = clock() - time;

      rle8m_opencl_destroy();

      if ((uint32_t)size != decompressedSize)
      {
        puts("Failed to decompress file (OpenCL).");
        goto epilogue;
      }

      printf("Decompressed in %f ms (OpenCL).\n", time / (double)CLOCKS_PER_SEC * 1000.0);

      if (memcmp(pUncompressedData, pDecompressedData, (size_t)size) != 0)
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