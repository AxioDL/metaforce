#include "Kyoto/Text/CImageInstruction.hpp"

#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Text/CFontRenderState.hpp"
#include "Kyoto/Text/CLineInstruction.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"

int CFontImageDef::CalculateHeight() const {
  return GetHeight() - (GetHeight() - CalculateBaseline());
}

void CImageInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  if (mImage.IsLoaded() && !mImage.GetImages().empty()) {
    if (state.GetBlock()->GetTextDirection() == kTD_Horizontal) {
      if (buf) {
        buf->AddImage(CVector2i(state.GetX(), state.GetY() + state.GetLine()->GetBaseline() -
                                                  mImage.CalculateBaseline()),
                      mImage);
      }
      state.SetX(state.GetX() + mImage.GetWidth());
    } else {
      int offset = (state.GetLine()->GetWidth() - mImage.GetWidth()) / 2;
      if (buf) {
        buf->AddImage(CVector2i(offset + state.GetX(), state.GetY()), mImage);
      }
      state.SetY(state.GetY() + mImage.CalculateHeight());
    }
  }
}

void CImageInstruction::GetAssets(rstl::vector< CToken >& assetsOut) const {
  for (int i = 0; i < mImage.GetImages().size(); ++i) {
    assetsOut.push_back(mImage.GetImages()[i]);
  }
}
