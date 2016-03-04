#include <stdio.h>
#include <stdarg.h>

#include "CBasics.hpp"

namespace urde
{

void CBasics::Init()
{
}

const char* CBasics::Stringize(const char* fmt, ...)
{
    static char STRINGIZE_STR[2048] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(STRINGIZE_STR, 2048, fmt, ap);
    va_end(ap);
    return STRINGIZE_STR;
}

}
