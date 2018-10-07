#pragma once

#include "CGuiTextSupport.hpp"
#include "CSaveableState.hpp"
#include "CDrawStringOptions.hpp"

namespace urde
{
class CLineInstruction;
class CBlockInstruction;

class CFontRenderState : public CSaveableState
{
    friend class CLineInstruction;
    friend class CBlockInstruction;
    friend class CTextInstruction;
    friend class CImageInstruction;
    friend class CWordInstruction;

    CBlockInstruction* x88_curBlock = nullptr;
    CDrawStringOptions x8c_drawOpts;
    s32 xd4_curX = 0;
    s32 xd8_curY = 0;
    const CLineInstruction* xdc_currentLineInst = nullptr;
    std::vector<u32> xe8_;
    std::vector<u8> xf8_;
    bool x108_lineInitialized = true;
    std::list<CSaveableState> x10c_pushedStates;
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

