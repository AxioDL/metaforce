#ifndef __PSHAG_CMODEL_HPP__
#define __PSHAG_CMODEL_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

struct CModelFlags
{
    u8 f1;
    u8 f2;
    u16 f3;
    zeus::CColor color;
};

class CModel
{
public:
    void Draw(const CModelFlags& flags) const;
};

}

#endif // __PSHAG_CMODEL_HPP__
