#ifndef _CCOLOROVERRIDEINSTRUCTION
#define _CCOLOROVERRIDEINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CTextColor.hpp"

class CColorOverrideInstruction : public CInstruction {
public:
  explicit CColorOverrideInstruction(int idx, const CTextColor& color)
  : mIdx(idx), mColor(color) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;

private:
  int mIdx;
  CTextColor mColor;
};

#endif // _CCOLOROVERRIDEINSTRUCTION
