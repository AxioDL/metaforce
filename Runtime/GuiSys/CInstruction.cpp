#include "CInstruction.hpp"
#include "CFontRenderState.hpp"

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

}
