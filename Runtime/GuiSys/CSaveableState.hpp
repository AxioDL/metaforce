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
    TLockedToken<CRasterFont> x48_font;
    std::vector<CTextColor> x54_;
    std::vector<bool> x64_colorOverrides;
    float x74_lineSpacing = 1.f;
    s32 x78_extraLineSpace = 0;
    bool x7c_enableWordWrap = false;
    EJustification x80_just = EJustification::Left;
    EVerticalJustification x84_vjust = EVerticalJustification::Top;

public:
    CSaveableState()
    {
        x54_.resize(3, zeus::CColor::skBlack);
        x64_colorOverrides.resize(16);
    }
};

}

#endif // __URDE_CSAVEABLESTATE_HPP__
