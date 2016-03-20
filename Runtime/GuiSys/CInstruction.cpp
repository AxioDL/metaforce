#include "CInstruction.hpp"
#include "CFontRenderState.hpp"
#include "CTextRenderBuffer.hpp"

namespace urde
{

void CInstruction::GetAssets(std::vector<CToken>& assetsOut) const
{
}

size_t CInstruction::GetAssetCount() const
{
    return 0;
}

void CColorInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.SetColor(x4_cType, x8_color);
}

void CColorOverrideInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.x30_[x4_overrideIdx] = true;
    zeus::CColor convCol = state.ConvertToTextureSpace(x8_color);
    state.x0_drawStrOpts.x4_vec[x4_overrideIdx] = convCol;
}

void CFontInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    buf->AddFontChange(x4_font);
    state.x14_token = x4_font;
    state.RefreshPalette();
}

void CFontInstruction::GetAssets(std::vector<CToken>& assetsOut) const
{
    assetsOut.push_back(x4_font);
}

size_t CFontInstruction::GetAssetCount() const
{
    return 1;
}

void CLineExtraSpaceInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.x44_extraLineSpace = x4_extraSpace;
}

void CLineInstruction::TestLargestFont(s32 w, s32 h, s32 b)
{
    if (!x18_largestBaseline)
        x18_largestBaseline = b;

    if (x14_largestMonoWidth < w)
        w = x14_largestMonoWidth;

    if (x10_largestMonoHeight < h)
    {
        x10_largestMonoHeight = h;
        x18_largestBaseline = b;
    }
}

void CLineInstruction::InvokeLTR(CFontRenderState& state) const
{
}

void CLineInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    InvokeLTR(state);
    state.xa0_ = true;
    state.x74_currentLineInst = this;
}

CTextInstruction::CTextInstruction(const wchar_t* str, int len)
{
}

void CTextInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
}

}
