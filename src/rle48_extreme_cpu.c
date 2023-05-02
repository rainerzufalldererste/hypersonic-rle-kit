#include "rle8.h"
#include "rleX_extreme_common.h"

#include "rle48_extreme_cpu.h"

#define UNBOUND

#include "rle48_extreme_cpu.h"

#define PACKED

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
  #define PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle48_extreme_cpu.h"

#undef UNBOUND

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #undef PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#include "rle48_extreme_cpu.h"
