#include "CInstruction.hpp"
#include "CFontRenderState.hpp"
#include "CTextRenderBuffer.hpp"
#include "CRasterFont.hpp"
#include "Graphics/CTexture.hpp"

namespace urde {

void CInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {}

void CInstruction::GetAssets(std::vector<CToken>& assetsOut) const {}

size_t CInstruction::GetAssetCount() const { return 0; }

void CColorInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.SetColor(x4_cType, x8_color);
}

void CColorInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const { Invoke(state, buf); }

void CColorOverrideInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.x64_colorOverrides[x4_overrideIdx] = true;
  zeus::CColor convCol = state.ConvertToTextureSpace(x8_color);
  state.x0_drawStrOpts.x4_colors[x4_overrideIdx] = convCol;
}

void CColorOverrideInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}

void CFontInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  buf->AddFontChange(x4_font);
  state.x48_font = x4_font;
  state.RefreshPalette();
}

void CFontInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const { Invoke(state, buf); }

void CFontInstruction::GetAssets(std::vector<CToken>& assetsOut) const { assetsOut.push_back(x4_font); }

size_t CFontInstruction::GetAssetCount() const { return 1; }

void CLineExtraSpaceInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.x78_extraLineSpace = x4_extraSpace;
}

void CLineExtraSpaceInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}

void CLineInstruction::TestLargestFont(s32 w, s32 h, s32 b) {
  if (!x18_largestMonoBaseline)
    x18_largestMonoBaseline = b;

  if (x14_largestMonoWidth < w)
    x14_largestMonoWidth = w;

  if (x10_largestMonoHeight < h) {
    x10_largestMonoHeight = h;
    x18_largestMonoBaseline = b;
  }
}

void CLineInstruction::TestLargestImage(s32 w, s32 h, s32 b) {
  if (!x24_largestImageBaseline)
    x24_largestImageBaseline = b;

  if (x20_largestImageWidth < w)
    x20_largestImageWidth = w;

  if (x1c_largestImageHeight < h) {
    x1c_largestImageHeight = h;
    x24_largestImageBaseline = b;
  }
}

void CLineInstruction::InvokeLTR(CFontRenderState& state) const {
  switch (x28_just) {
  case EJustification::Left:
  case EJustification::Full:
  case EJustification::NLeft:
  case EJustification::LeftMono:
    state.xd4_curX = state.x88_curBlock->x4_offsetX;
    break;
  case EJustification::Center:
  case EJustification::CenterMono:
    state.xd4_curX = state.x88_curBlock->x4_offsetX + state.x88_curBlock->xc_blockExtentX / 2 - x8_curX / 2;
    break;
  case EJustification::NCenter:
    if (x4_wordCount == 1) {
      state.xd4_curX = state.x88_curBlock->x4_offsetX + state.x88_curBlock->xc_blockExtentX / 2 - x8_curX / 2;
    } else {
      state.xd4_curX =
          state.x88_curBlock->x4_offsetX + state.x88_curBlock->xc_blockExtentX / 2 - state.x88_curBlock->x2c_lineX / 2;
    }
    break;
  case EJustification::Right:
  case EJustification::RightMono:
    state.xd4_curX = state.x88_curBlock->x4_offsetX + state.x88_curBlock->xc_blockExtentX - x8_curX;
    break;
  case EJustification::NRight:
    state.xd4_curX =
        state.x88_curBlock->x4_offsetX + state.x88_curBlock->xc_blockExtentX - state.x88_curBlock->x2c_lineX;
    break;
  default:
    break;
  }

  if (state.xdc_currentLineInst) {
    const CLineInstruction& inst = *state.xdc_currentLineInst;
    s32 val = 0;
    switch (state.x88_curBlock->x1c_vertJustification) {
    case EVerticalJustification::Top:
    case EVerticalJustification::Center:
    case EVerticalJustification::Bottom:
    case EVerticalJustification::NTop:
    case EVerticalJustification::NCenter:
    case EVerticalJustification::NBottom:
      val = inst.xc_curY;
      break;
    case EVerticalJustification::Full:
      val = state.x88_curBlock->x10_blockExtentY - state.x88_curBlock->x30_lineY;
      if (state.x88_curBlock->x34_lineCount > 1)
        val /= state.x88_curBlock->x34_lineCount - 1;
      else
        val = 0;
      val += inst.xc_curY;
      break;
    case EVerticalJustification::TopMono:
      val = state.x88_curBlock->x24_largestMonoH;
      break;
    case EVerticalJustification::CenterMono:
      val = (inst.xc_curY - state.x88_curBlock->x24_largestMonoH) / 2 + state.x88_curBlock->x24_largestMonoH;
      break;
    case EVerticalJustification::RightMono:
      val = state.x88_curBlock->x24_largestMonoH * 2 - inst.xc_curY;
      break;
    }

    if (state.x88_curBlock->x1c_vertJustification != EVerticalJustification::Full)
      val = val * state.x74_lineSpacing + state.x78_extraLineSpace;

    state.xd8_curY += val;
  }
}

void CLineInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  InvokeLTR(state);
  state.x108_lineInitialized = true;
  state.xdc_currentLineInst = this;
}

void CLineInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  if (!state.xdc_currentLineInst)
    Invoke(state, buf);
}

void CLineSpacingInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.x74_lineSpacing = x4_lineSpacing;
}

void CLineSpacingInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const { Invoke(state, buf); }

void CPopStateInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const { state.PopState(); }

void CPopStateInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const { Invoke(state, buf); }

void CPushStateInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const { state.PushState(); }

void CPushStateInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const { Invoke(state, buf); }

void CRemoveColorOverrideInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.x64_colorOverrides[x4_idx] = false;
}

void CRemoveColorOverrideInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}

void CImageInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  if (x4_image.IsLoaded() && x4_image.x4_texs.size()) {
    const CTexture* tex = x4_image.x4_texs[0].GetObj();
    if (state.x88_curBlock->x14_dir == ETextDirection::Horizontal) {
      if (buf) {
        int y = state.xd8_curY + state.xdc_currentLineInst->GetBaseline() - x4_image.CalculateBaseline();
        zeus::CVector2i coords(state.xd4_curX, y);
        buf->AddImage(coords, x4_image);
      }
      state.xd4_curX = state.xd4_curX + tex->GetWidth() * x4_image.x14_cropFactor.x();
    } else {
      int scale = state.xdc_currentLineInst->x8_curX - tex->GetWidth() * x4_image.x14_cropFactor.x();
      if (buf) {
        zeus::CVector2i coords(scale / 2 + state.xd4_curX, state.xd8_curY);
        buf->AddImage(coords, x4_image);
      }
      state.xd8_curY += x4_image.CalculateHeight();
    }
  }
}

void CImageInstruction::GetAssets(std::vector<CToken>& assetsOut) const {
  for (const CToken& tok : x4_image.x4_texs)
    assetsOut.push_back(tok);
}

size_t CImageInstruction::GetAssetCount() const { return x4_image.x4_texs.size(); }

void CTextInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  int xOut, yOut;
  if (state.x88_curBlock->x14_dir == ETextDirection::Horizontal) {
    state.x48_font->DrawString(state.x0_drawStrOpts, state.xd4_curX,
                               state.xdc_currentLineInst->GetBaseline() + state.xd8_curY, xOut, yOut, buf,
                               x4_str.c_str(), x4_str.size());
    state.xd4_curX = xOut;
  } else {
    int scale = state.xdc_currentLineInst->x8_curX - state.x48_font->GetMonoWidth();
    state.x48_font->DrawString(state.x0_drawStrOpts, scale / 2 + state.xd4_curX, state.xd8_curY, xOut, yOut, buf,
                               x4_str.c_str(), x4_str.size());
    state.xd8_curY = yOut;
  }
}

void CBlockInstruction::TestLargestFont(s32 monoW, s32 monoH, s32 baseline) {
  if (!x28_largestBaseline)
    x28_largestBaseline = baseline;

  if (x20_largestMonoW < monoW)
    monoW = x20_largestMonoW;

  if (x24_largestMonoH < monoH) {
    x24_largestMonoH = monoH;
    x28_largestBaseline = baseline;
  }
}

void CBlockInstruction::SetupPositionLTR(CFontRenderState& state) const {
  switch (x1c_vertJustification) {
  case EVerticalJustification::Top:
  case EVerticalJustification::Full:
  case EVerticalJustification::NTop:
  case EVerticalJustification::TopMono:
    state.xd8_curY = x8_offsetY;
    break;
  case EVerticalJustification::Center:
  case EVerticalJustification::NCenter:
    state.xd8_curY = x8_offsetY + (x10_blockExtentY - x30_lineY) / 2;
    break;
  case EVerticalJustification::CenterMono:
    state.xd8_curY = x8_offsetY + (x10_blockExtentY - x34_lineCount * x24_largestMonoH) / 2;
    break;
  case EVerticalJustification::Bottom:
  case EVerticalJustification::NBottom:
    state.xd8_curY = x8_offsetY + x10_blockExtentY - x30_lineY;
    break;
  case EVerticalJustification::RightMono:
    state.xd8_curY = x8_offsetY + x10_blockExtentY - x34_lineCount * x24_largestMonoH;
    break;
  }
}

void CBlockInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.x0_drawStrOpts.x0_direction = x14_dir;
  state.x88_curBlock = const_cast<CBlockInstruction*>(this);
  if (x14_dir == ETextDirection::Horizontal)
    SetupPositionLTR(state);
}

void CBlockInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const { Invoke(state, buf); }

void CWordInstruction::InvokeLTR(CFontRenderState& state) const {
  CRasterFont* font = state.x48_font.GetObj();
  char16_t space = u' ';
  int w, h;
  font->GetSize(state.x0_drawStrOpts, w, h, &space, 1);

  const CLineInstruction& inst = *state.xdc_currentLineInst;
  switch (state.x88_curBlock->x18_justification) {
  case EJustification::Full:
    w += (state.x88_curBlock->xc_blockExtentX - inst.x8_curX) / (inst.x4_wordCount - 1);
    break;
  case EJustification::NLeft:
  case EJustification::NCenter:
  case EJustification::NRight:
    w += (state.x88_curBlock->x2c_lineX - inst.x8_curX) / (inst.x4_wordCount - 1);
    break;
  default:
    break;
  }

  int wOut = state.xd4_curX;
  font->DrawSpace(state.x0_drawStrOpts, wOut, inst.xc_curY - font->GetMonoHeight() + state.xd8_curY, wOut, h, w);
  state.xd4_curX = wOut;
}

void CWordInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  if (state.x108_lineInitialized) {
    state.x108_lineInitialized = false;
    return;
  }

  if (state.x0_drawStrOpts.x0_direction == ETextDirection::Horizontal)
    InvokeLTR(state);
}

void CWordInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.x108_lineInitialized = false;
}

} // namespace urde
