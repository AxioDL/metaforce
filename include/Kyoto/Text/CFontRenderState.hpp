#ifndef _CFONTRENDERSTATE
#define _CFONTRENDERSTATE

#include "rstl/list.hpp"

#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CDrawStringOptions.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CSaveableState.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#include <rstl/vector.hpp>

class CBlockInstruction;
class CLineInstruction;
class CFontRenderState {
public:
  CFontRenderState();
  void RefreshColor(EColorType col);
  uint ConvertToTextureSpace(const CTextColor& color) const;
  void PushState();
  void PopState();
  void SetColor(EColorType type, const CTextColor& color);
  void RefreshPalette();
  TToken< CRasterFont >& GetFont() { return *mState.GetFont(); }
  bool IsFinishedLoading() { return mState.IsFinishedLoading(); }
  CDrawStringOptions& GetOptions() { return mState.GetOptions(); }
  void SetFont(const TToken< CRasterFont >& font) { mState.SetFont(font); }
  rstl::vector< CTextColor >& GetColors() { return mState.GetColors(); }
  rstl::vector< bool >& GetOverride() { return mState.GetOverride(); }
  float GetLineSpacing() const { return mState.GetLineSpacing(); }
  void SetLineSpacing(float spacing) { mState.SetLineSpacing(spacing); }
  int GetLineExtraSpacing() const { return mState.GetLineExtraSpacing(); }
  void SetExtraLineSpace(int spacing) { mState.SetLineExtraSpace(spacing); }
  const CBlockInstruction* GetBlock() const { return mCurBlock; }
  void SetBlock(const CBlockInstruction* block) {
    mCurBlock = const_cast< CBlockInstruction* >(block);
  }
  void SetX(int x) { mCurX = x; }
  int GetX() const { return mCurX; }
  void SetY(int y) { mCurY = y; }
  int GetY() const { return mCurY; }
  const CLineInstruction* GetLine() const { return mCurrentLineInst; }
  bool IsFirstWordOnLine() const { return mLineInitialized; }
  void SetFirstWordOnLine(bool v) { mLineInitialized = v; }

  void SetLine(const CLineInstruction* line) { mCurrentLineInst = line; }

  void SubX(const int x) { mCurX -= x; }

  void AddX(const int x) { mCurX += x; }

  void SubY(const int y) { mCurY -= y; }
  void AddY(const int y) { mCurY += y; }

  int GetSpacing(const int v) const {
    if (GetBlock()->GetVerticalJustification() == kVerticalJustification_Full) {
      return v;
    }

    return (static_cast< int >(static_cast< float >(v) * GetLineSpacing()) + GetLineExtraSpacing());
  }

private:
  CSaveableState mState;
  CBlockInstruction* mCurBlock;
  CDrawStringOptions mDrawOpts;
  int mCurX;
  int mCurY;
  const CLineInstruction* mCurrentLineInst;
  uint xe0_;
  uint xe4_;
  rstl::vector< uint > xe8_;
  rstl::vector< uchar > xf8_;
  bool mLineInitialized;
  rstl::list< CSaveableState > mPushedStates;
};

CHECK_SIZEOF(CFontRenderState,
             (VERSION >= VERSION_GM8P_00 ? 0x12c : 0x124))

#endif // _CFONTRENDERSTATE
