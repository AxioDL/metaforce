#include "Kyoto/Text/CRemoveColorOverrideInstruction.hpp"

#include "Kyoto/Text/CFontRenderState.hpp"

void CRemoveColorOverrideInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.GetOverride()[mIdx] = false;
  state.RefreshPalette();
}

void CRemoveColorOverrideInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}
