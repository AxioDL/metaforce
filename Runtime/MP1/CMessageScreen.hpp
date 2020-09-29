#pragma once

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
struct CFinalInput;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;

namespace MP1 {

class CMessageScreen {
  TLockedToken<CStringTable> x0_msg;
  TLockedToken<CGuiFrame> xc_msgScreen;
  CGuiFrame* x18_loadedMsgScreen = nullptr;
  CGuiTextPane* x1c_textpane_message = nullptr;
  CGuiWidget* x20_basewidget_top = nullptr;
  CGuiWidget* x24_basewidget_center = nullptr;
  CGuiWidget* x28_basewidget_bottom = nullptr;
  CGuiModel* x2c_model_abutton = nullptr;
  CGuiModel* x30_model_top = nullptr;
  CGuiModel* x34_model_center = nullptr;
  CGuiModel* x38_model_bottom = nullptr;
  CGuiModel* x3c_model_bg = nullptr;
  CGuiModel* x40_model_videoband = nullptr;
  zeus::CVector3f x44_topPos;
  zeus::CVector3f x50_bottomPos;
  zeus::CVector3f x5c_videoBandPos;
  float x68_videoBandOffset = 10.f;
  u32 x6c_page = 0;
  float x70_blurAmt = 0.f;
  float x74_delayTime;
  bool x78_24_exit : 1 = false;

public:
  explicit CMessageScreen(CAssetId msg, float time);
  void ProcessControllerInput(const CFinalInput& input);
  bool Update(float dt, float blurAmt);
  void Draw() const;
};

} // namespace MP1
} // namespace urde
