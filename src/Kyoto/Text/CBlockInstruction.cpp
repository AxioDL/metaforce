#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CFontRenderState.hpp"

void CBlockInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  state.GetOptions().SetTextDirection(mDir);
  state.SetBlock(this);
  if (mDir == kTD_Horizontal) {
    SetupPositionLTR(state);
  }
}
void CBlockInstruction::SetupPositionLTR(CFontRenderState& state) const {
  switch (mVertJustification) {
  case kVerticalJustification_Top:
  case kVerticalJustification_Full:
  case kVerticalJustification_NTop:
  case kVerticalJustification_TopMono:
    state.SetY(mOffsetY);
    break;
  case kVerticalJustification_Center:
  case kVerticalJustification_NCenter:
    state.SetY(mOffsetY + (mBlockExtentY - mLineY) / 2);
    break;
  case kVerticalJustification_CenterMono:
    state.SetY(mOffsetY + (mBlockExtentY - mLineCount * mLargestMonoH) / 2);
    break;
  case kVerticalJustification_Bottom:
  case kVerticalJustification_NBottom:
    state.SetY(mOffsetY + mBlockExtentY - mLineY);
    break;
  case kVerticalJustification_RightMono:
    state.SetY(mOffsetY + mBlockExtentY - mLineCount * mLargestMonoH);
    break;
  }
}

void CBlockInstruction::TestLargestFont(int monoW, int monoH, int baseline) {
  if (!mLargestBaseline) {
    mLargestBaseline = baseline;
  }

  if (mLargestMonoW < monoW) {
    mLargestMonoW = monoW;
  }

  if (mLargestMonoH < monoH) {
    mLargestMonoH = monoH;
    mLargestBaseline = baseline;
  }
}

void CBlockInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}
