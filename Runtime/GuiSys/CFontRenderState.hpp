#ifndef __URDE_CFONTRENDERSTATE_HPP__
#define __URDE_CFONTRENDERSTATE_HPP__

#include "CGuiTextSupport.hpp"
#include "CSaveableState.hpp"
#include "CDrawStringOptions.hpp"

namespace urde
{

class CFontRenderState : public CSaveableState
{
    u32 x54_ = 0;
    CDrawStringOptions x58_drawOpts;
    u32 x6c_ = 0;
    bool xa0_ = true;
    std::vector<CSaveableState> xa4_pushedStates;
public:
    CFontRenderState();
    zeus::CColor ConvertToTextureSpace(const CTextColor& col) const;
    void PopState();
    void PushState();
    void SetColor(EColorType tp, const CTextColor& col);
    void RefreshPalette();
    void RefreshColor(EColorType tp);
};

}

#endif // __URDE_CFONTRENDERSTATE_HPP__
