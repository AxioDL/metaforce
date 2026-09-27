#ifndef _CSAVEABLESTATE
#define _CSAVEABLESTATE

#include "Kyoto/Text/CDrawStringOptions.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CTextColor.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CSaveableState {
public:
  CSaveableState();
  bool IsFinishedLoading();

  CDrawStringOptions& GetOptions() { return mDrawStringOptions; }
  rstl::optional_object< TToken< CRasterFont > >& GetFont() { return mFont; }
  void SetFont(const TToken< CRasterFont >& font) { mFont = font; }
  rstl::vector< CTextColor >& GetColors() { return mColors; }
  rstl::vector< bool >& GetOverride() { return mColorOverrides; }
  float GetLineSpacing() const { return mLineSpacing; }
  void SetLineSpacing(float spacing) { mLineSpacing = spacing; }
  int GetLineExtraSpacing() const { return mExtraLineSpacing; }
  void SetLineExtraSpace(int spacing) { mExtraLineSpacing = spacing; }
  void SetWordWrapping(const bool wrap) { mEnableWordWrap = wrap; }
  bool IsWordWrapping() const { return mEnableWordWrap; }
  void SetJustification(EJustification just) { mJust = just; }
  void SetVerticalJustification(EVerticalJustification just) { mVjust = just; }

  EJustification GetJustification() const { return mJust; }
  EVerticalJustification GetVerticalJustification() const { return mVjust; }

  // private:
  CDrawStringOptions mDrawStringOptions;
  rstl::optional_object< TToken< CRasterFont > > mFont;
  rstl::vector< CTextColor > mColors;
  rstl::vector< bool > mColorOverrides;
  float mLineSpacing;
  int mExtraLineSpacing;
  bool mEnableWordWrap;
  EJustification mJust;
  EVerticalJustification mVjust;
};

CHECK_SIZEOF(CSaveableState,
             (VERSION >= VERSION_GM8P_00 ? 0x8c : 0x88))

#endif // _CSAVEABLESTATE
