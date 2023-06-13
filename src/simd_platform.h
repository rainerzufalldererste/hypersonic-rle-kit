#ifndef simd_platform_h__
#define simd_platform_h__

#include <stdbool.h>
#include <stdint.h>

#ifdef _MSC_VER
#include <intrin.h>
#define __builtin_popcount __popcnt
#else
#include <x86intrin.h>
#endif

//////////////////////////////////////////////////////////////////////////

extern bool _CpuFeaturesDetected;
extern bool sseSupported;
extern bool sse2Supported;
extern bool sse3Supported;
extern bool ssse3Supported;
extern bool sse41Supported;
extern bool sse42Supported;
extern bool avxSupported;
extern bool avx2Supported;
extern bool fma3Supported;
extern bool avx512FSupported;
extern bool avx512PFSupported;
extern bool avx512ERSupported;
extern bool avx512CDSupported;
extern bool avx512BWSupported;
extern bool avx512DQSupported;
extern bool avx512VLSupported;
extern bool avx512IFMASupported;
extern bool avx512VBMISupported;
extern bool avx512VNNISupported;
extern bool avx512VBMI2Supported;
extern bool avx512POPCNTDQSupported;
extern bool avx512BITALGSupported;
extern bool avx5124VNNIWSupported;
extern bool avx5124FMAPSSupported;
extern bool aesNiSupported;

void _DetectCPUFeatures();

#endif // simd_platform_h__
