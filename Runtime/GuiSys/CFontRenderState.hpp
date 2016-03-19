#ifndef __URDE_CFONTRENDERSTATE_HPP__
#define __URDE_CFONTRENDERSTATE_HPP__

#include "CGuiTextSupport.hpp"
#include "CSaveableState.hpp"
#include "CDrawStringOptions.hpp"

namespace urde
{
class CLineInstruction;

class CFontRenderState : public CSaveableState
{
    friend class CLineInstruction;

    /* void* x54_ = 0; top-to-bottom state */
    CDrawStringOptions x58_drawOpts;
    u32 x6c_ = 0;
    const CLineInstruction* x74_currentLineInst = nullptr;
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
