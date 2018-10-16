#pragma once

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
    friend class CWordInstruction;

    ETextDirection x0_direction = ETextDirection::Horizontal;
    std::vector<CTextColor> x4_colors;
public:
    CDrawStringOptions()
    {
        x4_colors.resize(16);
    }
};

}

