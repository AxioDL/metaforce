#include "Kyoto/Text/CLineInstruction.hpp"

#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CFontRenderState.hpp"

void CLineInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  if (state.GetBlock()->GetTextDirection() == kTD_Horizontal) {
    InvokeTTB(state);
  }

  state.SetFirstWordOnLine(true);
  state.SetLine(this);
}

void CLineInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
#if VERSION < VERSION_GM8P_00
  if (state.GetLine() != nullptr) {
    return;
  }
  Invoke(state, buf);
#endif
}

void CLineInstruction::InvokeTTB(CFontRenderState& state) const {
  switch (mJustification) {
  case kJustification_Left:
  case kJustification_Full:
  case kJustification_NLeft:
  case kJustification_LeftMono:
    state.SetX(state.GetBlock()->GetOutputX());
    break;
  case kJustification_Center:
  case kJustification_CenterMono:
    state.SetX(state.GetBlock()->GetOutputX() + state.GetBlock()->GetOutputWidth() / 2 - mCurX / 2);
    break;
  case kJustification_NCenter:
    if (mWordCount == 1) {
      state.SetX(state.GetBlock()->GetOutputX() + state.GetBlock()->GetOutputWidth() / 2 -
                 mCurX / 2);
    } else {
      state.SetX(state.GetBlock()->GetOutputX() + state.GetBlock()->GetOutputWidth() / 2 -
                 state.GetBlock()->GetLineX() / 2);
    }
    break;
  case kJustification_Right:
  case kJustification_RightMono:
    state.SetX(state.GetBlock()->GetOutputX() + state.GetBlock()->GetOutputWidth() - mCurX);
    break;
  case kJustification_NRight:
    state.SetX(state.GetBlock()->GetOutputX() + state.GetBlock()->GetOutputWidth() -
               state.GetBlock()->GetLineX());
    break;
  default:
    break;
  }

  if (state.GetLine() == nullptr) {
    return;
  }
  const CLineInstruction& inst = *state.GetLine();
  int val = 0;

  switch (state.GetBlock()->GetVerticalJustification()) {
  case kVerticalJustification_Top:
  case kVerticalJustification_Center:
  case kVerticalJustification_Bottom:
  case kVerticalJustification_NTop:
  case kVerticalJustification_NCenter:
  case kVerticalJustification_NBottom:
    val = inst.GetY();
    break;
  case kVerticalJustification_Full: {
    const int tmp = state.GetBlock()->GetOutputHeight() - state.GetBlock()->GetLineY();
    val = (state.GetBlock()->GetLines() > 1 ? tmp / (state.GetBlock()->GetLines() - 1) : 0) +
          inst.GetY();
    break;
  }
  case kVerticalJustification_TopMono: {
    val = state.GetBlock()->GetLargestFontHeight();
  } break;
  case kVerticalJustification_CenterMono: {
    val = ((inst.GetY() - state.GetBlock()->GetLargestFontHeight()) / 2) +
          state.GetBlock()->GetLargestFontHeight();
  } break;
  case kVerticalJustification_RightMono: {
    int tmp = state.GetBlock()->GetLargestFontHeight() * 2;
    val = tmp - inst.GetY();
  } break;
  default:
    break;
  }
  state.AddY(state.GetSpacing(val));
}

void CLineInstruction::TestLargestFont(const int width, const int height, const int baseLine) {
  if (mLargestFontBaseline == 0) {
    mLargestFontBaseline = baseLine;
  }
  if (mLargestFontWidth < width) {
    mLargestFontWidth = width;
  }
  if (mLargestFontHeight < height) {
    mLargestFontHeight = height;
    mLargestFontBaseline = baseLine;
  }
}

void CLineInstruction::TestLargestImage(const int width, const int height, const int baseline) {
  if (mLargestImageBaseline == 0) {
    mLargestImageBaseline = baseline;
  }
  if (mLargestImageWidth < width) {
    mLargestImageWidth = width;
  }
  if (mLargestImageHeight < height) {
    mLargestImageHeight = height;
    mLargestImageBaseline = baseline;
  }
}

int CLineInstruction::GetHeight() const {
  return (mLargestFontHeight != 0 && !mImageBaseline) ? mLargestFontHeight : mLargestImageHeight;
}

int CLineInstruction::GetBaseline() const {
  return (mLargestFontHeight != 0 && !mImageBaseline) ? mLargestFontBaseline
                                                      : mLargestImageBaseline;
}