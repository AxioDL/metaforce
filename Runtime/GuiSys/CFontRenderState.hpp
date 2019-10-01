#pragma once

#include <list>
#include <vector>

#include "Runtime/GuiSys/CDrawStringOptions.hpp"
#include "Runtime/GuiSys/CGuiTextSupport.hpp"
#include "Runtime/GuiSys/CSaveableState.hpp"

namespace urde {
class CBlockInstruction;
class CLineInstruction;

class CFontRenderState : public CSaveableState {
  friend class CBlockInstruction;
  friend class CImageInstruction;
  friend class CLineInstruction;
  friend class CTextInstruction;
  friend class CWordInstruction;

  CBlockInstruction* x88_curBlock = nullptr;
  CDrawStringOptions x8c_drawOpts;
  s32 xd4_curX = 0;
  s32 xd8_curY = 0;
  const CLineInstruction* xdc_currentLineInst = nullptr;
  std::vector<u32> xe8_;
  std::vector<u8> xf8_;
  bool x108_lineInitialized = true;
  std::list<CSaveableState> x10c_pushedStates;

public:
  CFontRenderState();
  zeus::CColor ConvertToTextureSpace(const CTextColor& col) const;
  void PopState();
  void PushState();
  void SetColor(EColorType tp, const CTextColor& col);
  void RefreshPalette();
  void RefreshColor(EColorType tp);
};

} // namespace urde
