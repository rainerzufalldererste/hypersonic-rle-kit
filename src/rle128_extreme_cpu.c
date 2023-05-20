#include "rle.h"
#include "rleX_extreme_common.h"

#ifndef _MSC_VER
	#define popcnt16 __builtin_popcount
	#define popcnt32 __builtin_popcountl
	#define popcnt64 __builtin_popcountll
#else
	#define popcnt16 __popcnt16
	#define popcnt32 __popcnt
	#define popcnt64 __popcnt64
#endif

#include "rle128_extreme_cpu.h"

#define UNBOUND

#include "rle128_extreme_cpu.h"

#define PACKED

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
  #define PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle128_extreme_cpu.h"

#undef UNBOUND

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #undef PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle128_extreme_cpu.h"
