#ifndef __RETRO_CMODEL_HPP__
#define __RETRO_CMODEL_HPP__

#include "RetroTypes.hpp"
#include "CColor.hpp"

namespace Retro
{

struct CModelFlags
{
    u8 f1;
    u8 f2;
    u16 f3;
    Zeus::CColor color;
};

class CModel
{
public:
    void Draw(const CModelFlags& flags) const;
};

}

#endif // __RETRO_CMODEL_HPP__
