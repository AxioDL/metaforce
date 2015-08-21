#ifndef __RETRO_IOBJ_HPP__
#define __RETRO_IOBJ_HPP__

#include <HECL/HECL.hpp>
#include "RetroTypes.hpp"

namespace Retro
{

struct SObjectTag
{
    HECL::FourCC fcc;
    u32 id;
};

class IObj
{
};

}

#endif // __RETRO_IOBJ_HPP__
