#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include "rstl/rc_ptr.hpp"
#include "stdio.h"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include <string.h>

namespace rstl {
#if RSTL_VERSION >= RSTL_R3IJ
int CRefData::sNull = 0x1000000 - 1;
#else
CRefData CRefData::sNull(nullptr, 0x1000000 - 1);
#endif
}