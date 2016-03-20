#ifndef __URDE_CSAVEABLESTATE_HPP__
#define __URDE_CSAVEABLESTATE_HPP__

#include "CGuiTextSupport.hpp"
#include "CDrawStringOptions.hpp"
#include "CToken.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
class CRasterFont;

class CSaveableState
{
    friend class CTextExecuteBuffer;
    friend class CColorOverrideInstruction;
    friend class CFontInstruction;
    friend class CLineExtraSpaceInstruction;
protected:
    CDrawStringOptions x0_drawStrOpts;
    TToken<CRasterFont> x14_token;
    std::vector<CTextColor> x20_;
    std::vector<bool> x30_;
    float x40_lineSpacing = 1.f;
    s32 x44_extraLineSpace = 0;
    bool x48_ = false;
    EJustification x4c_just = EJustification::Zero;
    EVerticalJustification x50_vjust = EVerticalJustification::Zero;

public:
    CSaveableState()
    {
        x20_.resize(3, zeus::CColor::skBlack);
        x30_.resize(16);
    }
};

}

#endif // __URDE_CSAVEABLESTATE_HPP__
