#include "Runtime/GuiSys/CFontRenderState.hpp"

#include "Runtime/GuiSys/CRasterFont.hpp"

namespace urde {

CFontRenderState::CFontRenderState() {
  x54_colors[0] = zeus::skWhite;
  x54_colors[1] = zeus::skGrey;
  x54_colors[2] = zeus::skWhite;
  RefreshPalette();
}

zeus::CColor CFontRenderState::ConvertToTextureSpace(const CTextColor& col) const { return col; }

void CFontRenderState::PopState() {
  static_cast<CSaveableState&>(*this) = x10c_pushedStates.back();
  x10c_pushedStates.pop_back();
  RefreshPalette();
}

void CFontRenderState::PushState() { x10c_pushedStates.push_back(*this); }

void CFontRenderState::SetColor(EColorType tp, const CTextColor& col) {
  switch (tp) {
  case EColorType::Main:
  case EColorType::Outline:
  case EColorType::Geometry:
    x54_colors[int(tp)] = col;
    break;
  case EColorType::Foreground:
    x54_colors[0] = col;
    break;
  case EColorType::Background:
    x54_colors[1] = col;
    break;
  }
  RefreshColor(tp);
}

void CFontRenderState::RefreshPalette() {
  RefreshColor(EColorType::Foreground);
  RefreshColor(EColorType::Background);
}

void CFontRenderState::RefreshColor(EColorType tp) {
  switch (tp) {
  case EColorType::Main:
    if (!x48_font)
      return;
    switch (x48_font->GetMode()) {
    case EColorType::Main:
      if (!x64_colorOverrides[0])
        x0_drawStrOpts.x4_colors[0] = ConvertToTextureSpace(x54_colors[0]);
      break;
    case EColorType::Outline:
      if (!x64_colorOverrides[0])
        x0_drawStrOpts.x4_colors[0] = ConvertToTextureSpace(x54_colors[0]);
      break;
    default:
      break;
    }
    break;
  case EColorType::Outline:
    if (!x48_font)
      return;
    if (x64_colorOverrides[1])
      return;
    if (x48_font->GetMode() == EColorType::Outline)
      x0_drawStrOpts.x4_colors[1] = ConvertToTextureSpace(x54_colors[1]);
    break;
  case EColorType::Geometry:
    if (!x64_colorOverrides[2])
      x0_drawStrOpts.x4_colors[2] = ConvertToTextureSpace(x54_colors[2]);
    break;
  case EColorType::Foreground:
    RefreshColor(EColorType::Main);
    RefreshColor(EColorType::Geometry);
    break;
  case EColorType::Background:
    RefreshColor(EColorType::Outline);
    break;
  }
}

} // namespace urde
