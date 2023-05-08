#include "rle8.h"

#include "rleX_extreme_common.h"

//////////////////////////////////////////////////////////////////////////

typedef struct
{
  uint8_t symbol;
  uint8_t lastSymbols[3];
  int64_t count;
  int64_t lastRLE;
  size_t index;
} rle8_3symlut_short_compress_state_t;

//////////////////////////////////////////////////////////////////////////

#include "rle8_3sl_short.h"
