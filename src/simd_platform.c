#include "simd_platform.h"

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#define cpuid __cpuid
#else
#include <cpuid.h>

void cpuid(int info[4], int infoType)
{
  __cpuid_count(infoType, 0, info[0], info[1], info[2], info[3]);
}

// This appears to be defined in newer versions of clang & gcc
//uint64_t _xgetbv(unsigned int index)
//{
//  uint32_t eax, edx;
//  __asm__ __volatile__("xgetbv" : "=a"(eax), "=d"(edx) : "c"(index));
//  return ((uint64_t)edx << 32) | eax;
//}

#ifndef _XCR_XFEATURE_ENABLED_MASK
#define _XCR_XFEATURE_ENABLED_MASK  0
#endif
#endif

bool _CpuFeaturesDetected = false;
bool sseSupported = false;
bool sse2Supported = false;
bool sse3Supported = false;
bool ssse3Supported = false;
bool sse41Supported = false;
bool sse42Supported = false;
bool avxSupported = false;
bool avx2Supported = false;
bool fma3Supported = false;
bool avx512FSupported = false;
bool avx512PFSupported = false;
bool avx512ERSupported = false;
bool avx512CDSupported = false;
bool avx512BWSupported = false;
bool avx512DQSupported = false;
bool avx512VLSupported = false;
bool avx512IFMASupported = false;
bool avx512VBMISupported = false;
bool avx512VNNISupported = false;
bool avx512VBMI2Supported = false;
bool avx512POPCNTDQSupported = false;
bool avx512BITALGSupported = false;
bool avx5124VNNIWSupported = false;
bool avx5124FMAPSSupported = false;
bool aesNiSupported = false;

void _DetectCPUFeatures()
{
  if (_CpuFeaturesDetected)
    return;

  int32_t info[4];
  cpuid(info, 0);
  int32_t idCount = info[0];

  if (idCount >= 0x1)
  {
    int32_t cpuInfo[4];
    cpuid(cpuInfo, 1);

    const bool osUsesXSAVE_XRSTORE = (cpuInfo[2] & (1 << 27)) != 0;
    const bool cpuAVXSuport = (cpuInfo[2] & (1 << 28)) != 0;

    if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
    {
      uint64_t xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
      avxSupported = (xcrFeatureMask & 0x6) != 0;
    }

    sseSupported = (cpuInfo[3] & (1 << 25)) != 0;
    sse2Supported = (cpuInfo[3] & (1 << 26)) != 0;
    sse3Supported = (cpuInfo[2] & (1 << 0)) != 0;
    ssse3Supported = (cpuInfo[2] & (1 << 9)) != 0;
    sse41Supported = (cpuInfo[2] & (1 << 19)) != 0;
    sse42Supported = (cpuInfo[2] & (1 << 20)) != 0;
    fma3Supported = (cpuInfo[2] & (1 << 12)) != 0;
    aesNiSupported = (cpuInfo[2] & (1 << 25)) != 0;
  }

  if (idCount >= 0x7)
  {
    int32_t cpuInfo[4];
    cpuid(cpuInfo, 7);

    avx2Supported = (cpuInfo[1] & (1 << 5)) != 0;
    avx512FSupported = (cpuInfo[1] & 1 << 16) != 0;
    avx512PFSupported = (cpuInfo[1] & 1 << 26) != 0;
    avx512ERSupported = (cpuInfo[1] & 1 << 27) != 0;
    avx512CDSupported = (cpuInfo[1] & 1 << 28) != 0;
    avx512BWSupported = (cpuInfo[1] & 1 << 30) != 0;
    avx512DQSupported = (cpuInfo[1] & 1 << 17) != 0;
    avx512VLSupported = (cpuInfo[1] & 1 << 31) != 0;
    avx512IFMASupported = (cpuInfo[1] & 1 << 21) != 0;
    avx512VBMISupported = (cpuInfo[2] & 1 << 1) != 0;
    avx512VNNISupported = (cpuInfo[2] & 1 << 11) != 0;
    avx512VBMI2Supported = (cpuInfo[2] & 1 << 6) != 0;
    avx512POPCNTDQSupported = (cpuInfo[2] & 1 << 14) != 0;
    avx512BITALGSupported = (cpuInfo[2] & 1 << 12) != 0;
    avx5124VNNIWSupported = (cpuInfo[3] & 1 << 2) != 0;
    avx5124FMAPSSupported = (cpuInfo[3] & 1 << 3) != 0;
  }

  _CpuFeaturesDetected = true;
}
