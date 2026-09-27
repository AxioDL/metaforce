#include "Kyoto/Text/CSaveableState.hpp"

CSaveableState::CSaveableState()
: mColors(3, CTextColor(0, 0, 0, 255))
, mColorOverrides(16, false)
, mLineSpacing(1.f)
, mExtraLineSpacing(0)
, mEnableWordWrap(false)
, mJust(kJustification_Left)
, mVjust(kVerticalJustification_Top) {}

bool CSaveableState::IsFinishedLoading() {
  if (mFont) {
    TToken< CRasterFont > font(mFont.data());
    return font.IsLoaded() && font.GetT()->IsFinishedLoading();
  }

  return false;
}
