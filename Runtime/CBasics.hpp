#ifndef __URDE_CBASICS_HPP__
#define __URDE_CBASICS_HPP__

#include <stdint.h>
#include <stdlib.h>

#include "RetroTypes.hpp"

namespace urde
{

class CBasics
{
public:
    static void Init();
    static const char* Stringize(const char* fmt, ...);
};

}

#endif // __URDE_CBASICS_HPP__
