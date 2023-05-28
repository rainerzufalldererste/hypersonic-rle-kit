#include "rle.h"
#include "rleX_extreme_common.h"

#define TYPE_SIZE 16
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define TYPE_SIZE 32
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define TYPE_SIZE 64
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define UNBOUND

#define TYPE_SIZE 16
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define TYPE_SIZE 32
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define TYPE_SIZE 64
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define PACKED

#ifndef PREFER_7_BIT_OR_4_BYTE_COPY
  #define PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#define TYPE_SIZE 16
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define TYPE_SIZE 32
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define TYPE_SIZE 64
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#undef UNBOUND

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #undef PREFER_7_BIT_OR_4_BYTE_COPY
#endif

#define TYPE_SIZE 16
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define TYPE_SIZE 32
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE

#define TYPE_SIZE 64
#include "rleX_extreme_cpu.h"
#undef TYPE_SIZE
