#ifndef __PSHAG_CMODEL_HPP__
#define __PSHAG_CMODEL_HPP__

#include "RetroTypes.hpp"
#include "CColor.hpp"

namespace pshag
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

#endif // __PSHAG_CMODEL_HPP__
