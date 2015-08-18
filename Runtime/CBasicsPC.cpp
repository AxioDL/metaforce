#include <stdio.h>
#include <stdarg.h>

#include "CBasics.hpp"

namespace Retro
{

void CBasics::Init()
{
}

char STRINGIZE_STR[2048];

const char* CBasics::Stringize(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(STRINGIZE_STR, 2048, fmt, ap);
    va_end(ap);
    return STRINGIZE_STR;
}

}
