#include "rle8.h"
#include "rle8_ocl_kernel.h"

#include "CL/cl.h"

#include <string.h>

#define ENABLE_LOGGING
#define MAX_SUB_SECTION_COUNT 1024

#ifdef ENABLE_LOGGING
#include <time.h>
#endif

bool initialized = false;

cl_context context = NULL;
cl_platform_id *pPlatforms = NULL;
cl_device_id deviceID = NULL;
cl_command_queue commandQueue = NULL;
cl_mem gpu_in_buffer = NULL;
cl_mem gpu_out_buffer = NULL;
cl_mem gpu_in_rle = NULL;
cl_mem gpu_in_symbolToCount = NULL;
cl_mem gpu_in_startOffsets = NULL;
cl_program program = NULL;
cl_kernel decompressKernel = NULL;
cl_kernel decompressSingleKernel = NULL;

size_t _initialized_inputDataSize;
size_t _initialized_outputDataSize;
size_t _initialized_maxSubsectionCount;

bool rle8m_opencl_init(const size_t inputDataSize, const size_t outputDataSize, const size_t maxSubsectionCount)
{
  if (initialized)
  {
    if (_initialized_inputDataSize >= inputDataSize && _initialized_outputDataSize >= outputDataSize && _initialized_maxSubsectionCount >= maxSubsectionCount)
      return true;
    else
      rle8m_opencl_destroy();
  }

  bool success = false;
  cl_int ret = CL_SUCCESS;
  cl_uint platformCount;
  const size_t kernelSourceLength = strlen(kernelsource);

  if (CL_SUCCESS != (ret = clGetPlatformIDs(0, NULL, &platformCount)) || platformCount == 0)
    goto epilogue;

  pPlatforms = malloc(sizeof(cl_platform_id) * platformCount);

  if (pPlatforms == NULL)
    goto epilogue;

  ret = clGetPlatformIDs(platformCount, pPlatforms, NULL);
  ret |= clGetDeviceIDs(pPlatforms[0], CL_DEVICE_TYPE_GPU, 1, &deviceID, &platformCount);

  if (ret != CL_SUCCESS)
    goto epilogue;

  context = clCreateContext(NULL, 1, &deviceID, NULL, NULL, &ret);

  if (ret != CL_SUCCESS || context == NULL)
    goto epilogue;

  commandQueue = clCreateCommandQueue(context, deviceID, 0, &ret);

  if (ret != CL_SUCCESS || commandQueue == NULL)
    goto epilogue;

  gpu_in_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, inputDataSize, NULL, &ret);

  if (ret != CL_SUCCESS || gpu_in_buffer == NULL)
    goto epilogue;

  gpu_out_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputDataSize, NULL, &ret);

  if (ret != CL_SUCCESS || gpu_out_buffer == NULL)
    goto epilogue;

  gpu_in_rle = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uchar) * 256, NULL, &ret);

  if (ret != CL_SUCCESS || gpu_in_rle == NULL)
    goto epilogue;

  gpu_in_symbolToCount = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uchar) * 256, NULL, &ret);

  if (ret != CL_SUCCESS || gpu_in_symbolToCount == NULL)
    goto epilogue;

  gpu_in_startOffsets = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint) * (maxSubsectionCount + 1), NULL, &ret);

  if (ret != CL_SUCCESS || gpu_in_startOffsets == NULL)
    goto epilogue;

  program = clCreateProgramWithSource(context, 1, (const char **)&kernelsource, (const size_t *)&kernelSourceLength, &ret);

  if (ret != CL_SUCCESS || program == NULL)
    goto epilogue;

  ret = clBuildProgram(program, 1, &deviceID, NULL, NULL, NULL);

  if (ret == CL_BUILD_PROGRAM_FAILURE)
  {
#ifdef _DEBUG
    // Determine the size of the log
    size_t logSize;
    ret = clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);

    // Allocate memory for the log
    char *log = malloc(logSize);

    if (log)
    {
      // Get the log
      ret = clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);

      // Print the log
      puts("Failed to compile rle8 OpenCL kernel:");
      puts(log);
      free(log);
    }
#endif

    goto epilogue;
  }

  if (ret != CL_SUCCESS)
    goto epilogue;

  decompressKernel = clCreateKernel(program, "rle8_decompress", &ret);

  if (ret != CL_SUCCESS || decompressKernel == NULL)
    goto epilogue;

  decompressSingleKernel = clCreateKernel(program, "rle8_decompress_single", &ret);

  if (ret != CL_SUCCESS || decompressSingleKernel == NULL)
    goto epilogue;

  initialized = true;
  success = true;
  _initialized_inputDataSize = inputDataSize;
  _initialized_outputDataSize = outputDataSize;
  _initialized_maxSubsectionCount = maxSubsectionCount;

epilogue:
  if (!success)
    rle8m_opencl_destroy();

  free(pPlatforms);

  return success;
}

void rle8m_opencl_destroy()
{
  initialized = false;
  _initialized_inputDataSize = 0;
  _initialized_outputDataSize = 0;
  _initialized_maxSubsectionCount = 0;

  if (decompressKernel)
  {
    clReleaseKernel(decompressKernel);
    decompressKernel = NULL;
  }

  if (decompressSingleKernel)
  {
    clReleaseKernel(decompressSingleKernel);
    decompressSingleKernel = NULL;
  }

  if (program)
  {
    clReleaseProgram(program);
    program = NULL;
  }

  if (gpu_in_buffer)
  {
    clReleaseMemObject(gpu_in_buffer);
    gpu_in_buffer = NULL;
  }

  if (gpu_out_buffer)
  {
    clReleaseMemObject(gpu_out_buffer);
    gpu_out_buffer = NULL;
  }

  if (gpu_in_rle)
  {
    clReleaseMemObject(gpu_in_rle);
    gpu_in_rle = NULL;
  }

  if (gpu_in_symbolToCount)
  {
    clReleaseMemObject(gpu_in_symbolToCount);
    gpu_in_symbolToCount = NULL;
  }

  if (gpu_in_startOffsets)
  {
    clReleaseMemObject(gpu_in_startOffsets);
    gpu_in_startOffsets = NULL;
  }

  if (commandQueue)
  {
    clReleaseCommandQueue(commandQueue);
    commandQueue = NULL;
  }

  if (deviceID)
  {
    clReleaseDevice(deviceID);
    deviceID = NULL;
  }

  if (context)
  {
    clReleaseContext(context);
    context = NULL;
  }
}

uint32_t rle8m_opencl_decompress(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
  if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
    return 0;

  const size_t expectedInSize = ((uint32_t *)pIn)[0];
  const size_t expectedOutSize = ((uint32_t *)pIn)[1];

  if (expectedOutSize > outSize || expectedInSize > inSize)
    return 0;

  size_t index = 2 * sizeof(uint32_t);

  const uint32_t subSections = *((uint32_t *)(&pIn[index]));
  index += sizeof(uint32_t);

  if (subSections == 0 || subSections > MAX_SUB_SECTION_COUNT)
    return 0;

  const size_t subSectionIndex = index;
  index += (subSections - 1) * sizeof(uint32_t);

  rle8_decompress_info_t decompressInfo;

  index += rle8_read_decompress_info(&pIn[index], inSize, &decompressInfo);

  const uint32_t subSectionSize = (uint32_t)(expectedOutSize / subSections);

  cl_uint subSectionOffsets[MAX_SUB_SECTION_COUNT + 1];

  subSectionOffsets[0] = 0;

  for (size_t i = 1; i < subSections; i++)
    subSectionOffsets[i] = ((uint32_t *)(&pIn[subSectionIndex]))[i - 1] - (uint32_t)index;

  subSectionOffsets[subSections] = (uint32_t)expectedInSize - (uint32_t)index;

  uint8_t rleSymbolCount = 0;
  uint8_t symbol;

  for (size_t i = 0; i < 256; i++)
  {
    if (decompressInfo.rle[i])
    {
      rleSymbolCount++;
      symbol = (uint8_t)i;

      if (rleSymbolCount > 1)
        break;
    }
  }

  cl_int ret = CL_SUCCESS;
  size_t localSize[2] = { 1, 1 };
  size_t globalSize[2] = { 1, 1 };

#ifdef ENABLE_LOGGING
  clock_t time;
#endif

  globalSize[0] = subSections;
  
  bool success = rle8m_opencl_init(expectedInSize - index, expectedOutSize, subSections);

  if (!success)
    goto epilogue;

#ifdef ENABLE_LOGGING
  time = clock();
#endif
  
  if (CL_SUCCESS != (ret = clEnqueueWriteBuffer(commandQueue, gpu_in_buffer, CL_FALSE, 0, inSize - index, pIn + index, 0, NULL, NULL)))
    goto epilogue;

  if (rleSymbolCount != 1)
  {
    if (CL_SUCCESS != (ret = clEnqueueWriteBuffer(commandQueue, gpu_in_rle, CL_FALSE, 0, 256, decompressInfo.rle, 0, NULL, NULL)))
      goto epilogue;
  }

  if (CL_SUCCESS != (ret = clEnqueueWriteBuffer(commandQueue, gpu_in_symbolToCount, CL_FALSE, 0, 256, decompressInfo.symbolToCount, 0, NULL, NULL)))
    goto epilogue;

  if (CL_SUCCESS != (ret = clEnqueueWriteBuffer(commandQueue, gpu_in_startOffsets, CL_FALSE, 0, sizeof(cl_uint) * (subSections + 1), subSectionOffsets, 0, NULL, NULL)))
    goto epilogue;


  if (rleSymbolCount != 1)
  {
    if (CL_SUCCESS != (ret = clSetKernelArg(decompressKernel, 0, sizeof(cl_mem), (void *)&gpu_out_buffer)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressKernel, 1, sizeof(cl_mem), (void *)&gpu_in_buffer)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressKernel, 2, sizeof(cl_mem), (void *)&gpu_in_rle)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressKernel, 3, sizeof(cl_mem), (void *)&gpu_in_symbolToCount)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressKernel, 4, sizeof(cl_mem), (void *)&gpu_in_startOffsets)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressKernel, 5, sizeof(cl_uint), (void *)&subSectionSize)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clEnqueueNDRangeKernel(commandQueue, decompressKernel, 1, NULL, globalSize, localSize, 0, NULL, NULL)))
      goto epilogue;
  }
  else
  {
    if (CL_SUCCESS != (ret = clSetKernelArg(decompressSingleKernel, 0, sizeof(cl_mem), (void *)&gpu_out_buffer)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressSingleKernel, 1, sizeof(cl_mem), (void *)&gpu_in_buffer)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressSingleKernel, 2, sizeof(cl_uchar), (void *)&symbol)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressSingleKernel, 3, sizeof(cl_mem), (void *)&gpu_in_symbolToCount)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressSingleKernel, 4, sizeof(cl_mem), (void *)&gpu_in_startOffsets)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clSetKernelArg(decompressSingleKernel, 5, sizeof(cl_uint), (void *)&subSectionSize)))
      goto epilogue;

    if (CL_SUCCESS != (ret = clEnqueueNDRangeKernel(commandQueue, decompressSingleKernel, 1, NULL, globalSize, localSize, 0, NULL, NULL)))
      goto epilogue;
  }

#ifdef ENABLE_LOGGING
  clFinish(commandQueue);

  time = clock() - time;

  printf("Uploaded & Decompressed on GPU: %f ms (OpenCL).\n", time / (double)CLOCKS_PER_SEC * 1000.0);
#endif

  if (CL_SUCCESS != (ret = clEnqueueReadBuffer(commandQueue, gpu_out_buffer, CL_TRUE, 0, expectedOutSize, pOut, 0, NULL, NULL)))
    goto epilogue;

  success = true;

epilogue:
  return success ? (uint32_t)expectedOutSize : 0;
}
