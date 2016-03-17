#ifndef __PSHAG_CMODEL_HPP__
#define __PSHAG_CMODEL_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

struct CModelFlags
{
    u8 f1; /* Blend state 3/5 enable additive */
    u8 f2;
    u16 f3; /* Depth state */
    zeus::CColor color; /* Set into kcolor slot specified by material */

    /* depth flags
        0x8: greater
        0x10: non-inclusive
     */
};

class CModel
{
public:
    void Draw(const CModelFlags& flags) const;
    void Touch(int) const;
    bool IsLoaded(int) const;
};

}

#endif // __PSHAG_CMODEL_HPP__
