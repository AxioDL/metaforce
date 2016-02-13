#ifndef __PSHAG_CBASICS_HPP__
#define __PSHAG_CBASICS_HPP__

#include <stdint.h>
#include <stdlib.h>

#include "RetroTypes.hpp"

namespace pshag
{

class CBasics
{
public:
    static void Init();
    static const char* Stringize(const char* fmt, ...);
};

}

#endif // __PSHAG_CBASICS_HPP__
