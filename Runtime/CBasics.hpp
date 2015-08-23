#ifndef __RETRO_CBASICS_HPP__
#define __RETRO_CBASICS_HPP__

#include <stdint.h>
#include <stdlib.h>

#include "RetroTypes.hpp"

namespace Retro
{

class CBasics
{
public:
    static void Init();
    static const char* Stringize(const char* fmt, ...);
};

}

#endif // __RETRO_CBASICS_HPP__
