#include "Kyoto/Text/CColorOverrideInstruction.hpp"
#include "Kyoto/Text/CFontRenderState.hpp"

void CColorOverrideInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.GetOverride()[mIdx] = true;
  state.GetOptions().SetPaletteEntry(mIdx, state.ConvertToTextureSpace(mColor));
}

void CColorOverrideInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}
