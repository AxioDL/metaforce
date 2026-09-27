#include "Kyoto/Text/CLineSpacingInstruction.hpp"

#include "Kyoto/Text/CFontRenderState.hpp"

void CLineSpacingInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.SetLineSpacing(mSpacing);
}

void CLineSpacingInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}
