#ifndef _CLINEEXTRASPACINGINSTRUCTION
#define _CLINEEXTRASPACINGINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"

class CLineExtraSpaceInstruction : public CInstruction {
public:
  CLineExtraSpaceInstruction(int spacing) : mSpacing(spacing) {}
  ~CLineExtraSpaceInstruction() {}
  
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
private:
  int mSpacing;
};

#endif // _CLINEEXTRASPACINGINSTRUCTION
