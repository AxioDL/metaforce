#ifndef __URDE_CDRAWSTRINGOPTIONS_HPP__
#define __URDE_CDRAWSTRINGOPTIONS_HPP__

#include <vector>
#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

class CDrawStringOptions
{
    friend class CColorOverrideInstruction;
    friend class CFontRenderState;
    friend class CRasterFont;
    u32 x0_ = 0;
    std::vector<zeus::CColor> x4_vec;
public:
    CDrawStringOptions()
    {
        x4_vec.resize(16);
    }
};

}

#endif // __URDE_CDRAWSTRINGOPTIONS_HPP__
