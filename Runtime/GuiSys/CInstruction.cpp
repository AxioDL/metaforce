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

void CExtraLineSpaceInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.x44_extraLineSpace = x4_extraSpace;
}

void CLineInstruction::TestLargestFont(int, int, int)
{
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

}
