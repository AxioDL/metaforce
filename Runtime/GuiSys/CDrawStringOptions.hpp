#ifndef __URDE_CDRAWSTRINGOPTIONS_HPP__
#define __URDE_CDRAWSTRINGOPTIONS_HPP__

#include <vector>
#include "RetroTypes.hpp"
#include "CGuiTextSupport.hpp"

namespace urde
{

class CDrawStringOptions
{
    friend class CColorOverrideInstruction;
    friend class CFontRenderState;
    friend class CRasterFont;
    friend class CTextExecuteBuffer;
    friend class CBlockInstruction;
    ETextDirection x0_direction = ETextDirection::Horizontal;
    std::vector<CTextColor> x4_vec;
public:
    CDrawStringOptions()
    {
        x4_vec.resize(16);
    }
};

}

#endif // __URDE_CDRAWSTRINGOPTIONS_HPP__
