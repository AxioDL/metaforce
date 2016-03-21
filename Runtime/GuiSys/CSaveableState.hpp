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
    friend class CTextInstruction;
    friend class CLineSpacingInstruction;
    friend class CRemoveColorOverrideInstruction;
    friend class CWordInstruction;
    friend class CGuiTextSupport;
protected:
    CDrawStringOptions x0_drawStrOpts;
    TToken<CRasterFont> x14_font;
    std::vector<CTextColor> x20_;
    std::vector<bool> x30_colorOverrides;
    float x40_lineSpacing = 1.f;
    s32 x44_extraLineSpace = 0;
    bool x48_enableWordWrap = false;
    EJustification x4c_just = EJustification::Left;
    EVerticalJustification x50_vjust = EVerticalJustification::Top;

public:
    CSaveableState()
    {
        x20_.resize(3, zeus::CColor::skBlack);
        x30_colorOverrides.resize(16);
    }
};

}

#endif // __URDE_CSAVEABLESTATE_HPP__
