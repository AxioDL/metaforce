#include "Kyoto/Text/CColorInstruction.hpp"

#include "Kyoto/Text/CFontRenderState.hpp"

void CColorInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.SetColor(mType, mColor);
}

void CColorInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}
