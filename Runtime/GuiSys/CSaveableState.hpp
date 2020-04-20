#pragma once

#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/GCNTypes.hpp"
#include "Runtime/GuiSys/CDrawStringOptions.hpp"
#include "Runtime/GuiSys/CGuiTextSupport.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"

#include <zeus/CColor.hpp>

namespace urde {
class CSaveableState {
  friend class CColorOverrideInstruction;
  friend class CFontInstruction;
  friend class CGuiTextSupport;
  friend class CLineExtraSpaceInstruction;
  friend class CLineSpacingInstruction;
  friend class CRemoveColorOverrideInstruction;
  friend class CTextExecuteBuffer;
  friend class CTextInstruction;
  friend class CWordInstruction;

protected:
  CDrawStringOptions x0_drawStrOpts;
  TLockedToken<CRasterFont> x48_font;
  std::vector<CTextColor> x54_colors;
  std::vector<bool> x64_colorOverrides;
  float x74_lineSpacing = 1.f;
  s32 x78_extraLineSpace = 0;
  bool x7c_enableWordWrap = false;
  EJustification x80_just = EJustification::Left;
  EVerticalJustification x84_vjust = EVerticalJustification::Top;

public:
  CSaveableState() : x54_colors(3, zeus::skBlack), x64_colorOverrides(16) {}

  bool IsFinishedLoading() const;
};

} // namespace urde
