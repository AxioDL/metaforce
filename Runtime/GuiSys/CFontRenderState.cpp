#include "Runtime/GuiSys/CFontRenderState.hpp"

#include "Runtime/GuiSys/CRasterFont.hpp"

namespace metaforce {

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
    x54_colors[size_t(tp)] = col;
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

void CFontRenderState::RefreshColor(EColorType color) {
  if (color == EColorType::Geometry && !x64_colorOverrides[2]) {
    x0_drawStrOpts.x4_colors[2] = ConvertToTextureSpace(x54_colors[2]);
  } else if (color == EColorType::Main && IsFinishedLoading() && !x48_font.IsNull()) {
    const auto mode = x48_font->GetMode();
    if (mode == EFontMode::OneLayerOutline && !x64_colorOverrides[0]) {
      x0_drawStrOpts.x4_colors[0] = ConvertToTextureSpace(x54_colors[0]);
    } else if (mode == EFontMode::OneLayer && !x64_colorOverrides[0]) {
      x0_drawStrOpts.x4_colors[0] = ConvertToTextureSpace(x54_colors[0]);
    }
  } else if (color == EColorType::Outline && IsFinishedLoading() && !x48_font.IsNull() && !x64_colorOverrides[1]) {
    if (x48_font->GetMode() == EFontMode::OneLayerOutline) {
      x0_drawStrOpts.x4_colors[1] = ConvertToTextureSpace(x54_colors[1]);
    }
  } else if (color == EColorType::Background) {
    RefreshColor(EColorType::Outline);
  } else if (color == EColorType::Foreground) {
    RefreshColor(EColorType::Main);
    RefreshColor(EColorType::Geometry);
  }
}

} // namespace metaforce
