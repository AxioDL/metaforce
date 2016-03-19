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
    friend class CExtraLineSpaceInstruction;
protected:
    CDrawStringOptions x0_drawStrOpts;
    TToken<CRasterFont> x14_token;
    std::vector<CTextColor> x20_;
    std::vector<bool> x30_;
    float x40_ = 1.f;
    s32 x44_extraLineSpace = 0;
    bool x48_ = false;
    u32 x4c_ = 0;
    u32 x50_ = 0;

public:
    CSaveableState()
    {
        x20_.resize(3, zeus::CColor::skBlack);
        x30_.resize(16);
    }
};

}

#endif // __URDE_CSAVEABLESTATE_HPP__
