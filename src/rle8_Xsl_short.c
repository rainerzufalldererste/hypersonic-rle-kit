#include "rle.h"

#include "rleX_extreme_common.h"

//////////////////////////////////////////////////////////////////////////

#define TYPE_SIZE 8
#define SYMBOL_COUNT 3

#include "rle8_Xsl_short.h"

#undef SYMBOL_COUNT
#define SYMBOL_COUNT 7

#include "rle8_Xsl_short.h"

#undef SYMBOL_COUNT
#define SYMBOL_COUNT 1

#include "rle8_Xsl_short.h"

#undef SYMBOL_COUNT
#define SYMBOL_COUNT 0

#include "rle8_Xsl_short.h"

#define SINGLE

#include "rle8_Xsl_short.h"
