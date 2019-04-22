#include "rle8.h"

#include <time.h>
#include <string.h>

const char ArgumentTo[] = "-to";

//////////////////////////////////////////////////////////////////////////

int main(int argc, char **pArgv)
{
  if (argc <= 1)
  {
    printf("Usage: rle8 <InputFileName> [%s <OutputFileName>]\n", ArgumentTo);
    return 1;
  }

  const char *outputFileName = NULL;

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

  compressedBufferSize = rle8_compress_bounds((uint32_t)size);

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

    clock_t time = clock();

    const uint32_t compressedSize = rle8_compress(pUncompressedData, (uint32_t)size, pCompressedData, compressedBufferSize);

    time = clock() - time;

    if (0 == compressedSize)
    {
      puts("Failed to compress file.");
      goto epilogue;
    }

    printf("Compressed %" PRIi32 " bytes -> %" PRIu32 " bytes (%f %%) in %" PRIu64 " ms.\n", size, compressedSize, (double)compressedSize / (double)size * 100.0, (uint64_t)time);

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

    decompressedSize = rle8_decompress(pCompressedData, compressedSize, pDecompressedData, (uint32_t)size);

    time = clock() - time;

    if ((uint32_t)size != decompressedSize)
    {
      puts("Failed to decompress file.");
      goto epilogue;
    }
    
    printf("Decompressed in %" PRIu64 " ms.\n", (uint64_t)time);

    if (memcmp(pUncompressedData, pDecompressedData, (size_t)size) != 0)
    {
      puts("Validation Failed.");
      goto epilogue;
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