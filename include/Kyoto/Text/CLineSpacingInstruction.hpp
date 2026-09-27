#ifndef _CLINESPACINGINSTRUCTION
#define _CLINESPACINGINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

class CLineSpacingInstruction : public CInstruction {
public:
  CLineSpacingInstruction(float spacing) : mSpacing(spacing) {}
  ~CLineSpacingInstruction() {}
  
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
private:
  float mSpacing;
};

#endif // _CLINESPACINGINSTRUCTION
