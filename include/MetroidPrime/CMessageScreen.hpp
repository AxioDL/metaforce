#ifndef _CMESSAGESCREEN
#define _CMESSAGESCREEN

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

class CFinalInput;
class CGuiFrame;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;
class CStringTable;

class CMessageScreen {
public:
  CMessageScreen(CAssetId msg, float time);
  void ProcessControllerInput(const CFinalInput& input);
  bool Update(float dt, float blurAmt);
  void Draw() const;

private:
  TCachedToken< CStringTable > mMsg;
  TCachedToken< CGuiFrame > mMsgScreen;
  CGuiFrame* mLoadedMsgScreen;
  CGuiTextPane* mTextpane_message;
  CGuiWidget* mBasewidget_top;
  CGuiWidget* mBasewidget_center;
  CGuiWidget* mBasewidget_bottom;
  CGuiModel* mModel_abutton;
  CGuiModel* mModel_top;
  CGuiModel* mModel_center;
  CGuiModel* mModel_bottom;
  CGuiModel* mModel_bg;
  CGuiModel* mModel_videoband;
  CVector3f mTopPos;
  CVector3f mBottomPos;
  CVector3f mVideoBandPos;
  float mVideoBandOffset;
  int mPage;
  float mBlurAmt;
  float mDelayTime;
  bool mExit : 1;
};
CHECK_SIZEOF(CMessageScreen, 0x7c)

#endif
