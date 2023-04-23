#include "rle8.h"
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
