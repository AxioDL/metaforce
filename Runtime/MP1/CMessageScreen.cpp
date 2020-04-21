#include "Runtime/MP1/CMessageScreen.hpp"

#include "Runtime/CGameState.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/Input/CFinalInput.hpp"

namespace urde::MP1 {

CMessageScreen::CMessageScreen(CAssetId msg, float delayTime) : x74_delayTime(delayTime) {
  x0_msg = g_SimplePool->GetObj({FOURCC('STRG'), msg});
  xc_msgScreen = g_SimplePool->GetObj("FRME_MsgScreen");
}

void CMessageScreen::ProcessControllerInput(const CFinalInput& input) {
  if (!x18_loadedMsgScreen || x74_delayTime > 0.f || !(input.PA() ||
                                                       input.PSpecialKey(boo::ESpecialKey::Enter) ||
                                                       input.PMouseButton(boo::EMouseButton::Primary)))
    return;

  if (x1c_textpane_message->TextSupport().GetCurTime() < x1c_textpane_message->TextSupport().GetTotalAnimationTime()) {
    x1c_textpane_message->TextSupport().SetCurTime(x1c_textpane_message->TextSupport().GetTotalAnimationTime());
    return;
  }

  x6c_page += 1;

  if (x6c_page >= x0_msg->GetStringCount()) {
    x78_24_exit = true;
    return;
  }

  x1c_textpane_message->TextSupport().SetTypeWriteEffectOptions(false, 0.1f, 30.f);
  x1c_textpane_message->TextSupport().SetText(x0_msg->GetString(x6c_page));

  CSfxManager::SfxStart(SFXui_message_screen_key, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

  x74_delayTime = 0.8f;
}

bool CMessageScreen::Update(float dt, float blurAmt) {
  x70_blurAmt = blurAmt;
  if (!x18_loadedMsgScreen && xc_msgScreen.IsLoaded() && x0_msg.IsLoaded()) {
    x18_loadedMsgScreen = xc_msgScreen.GetObj();
    x18_loadedMsgScreen->Reset();
    x1c_textpane_message = static_cast<CGuiTextPane*>(x18_loadedMsgScreen->FindWidget("textpane_message"));
    x20_basewidget_top = x18_loadedMsgScreen->FindWidget("basewidget_top");
    x24_basewidget_center = x18_loadedMsgScreen->FindWidget("basewidget_center");
    x28_basewidget_bottom = x18_loadedMsgScreen->FindWidget("basewidget_bottom");
    x2c_model_abutton = static_cast<CGuiModel*>(x18_loadedMsgScreen->FindWidget("model_abutton"));
    x30_model_top = static_cast<CGuiModel*>(x18_loadedMsgScreen->FindWidget("model_top"));
    x38_model_bottom = static_cast<CGuiModel*>(x18_loadedMsgScreen->FindWidget("model_bottom"));
    x34_model_center = static_cast<CGuiModel*>(x18_loadedMsgScreen->FindWidget("model_center"));
    x3c_model_bg = static_cast<CGuiModel*>(x18_loadedMsgScreen->FindWidget("model_bg"));
    x40_model_videoband = static_cast<CGuiModel*>(x18_loadedMsgScreen->FindWidget("model_videoband"));
    x44_topPos = x20_basewidget_top->GetLocalPosition();
    x50_bottomPos = x28_basewidget_bottom->GetLocalPosition();
    x5c_videoBandPos = x40_model_videoband->GetLocalPosition();

    if (CGuiWidget* w = x18_loadedMsgScreen->FindWidget("basewidget_top"))
      w->SetColor(g_tweakGuiColors->GetHudFrameColor());
    if (CGuiWidget* w = x18_loadedMsgScreen->FindWidget("basewidget_centerdeco"))
      w->SetColor(g_tweakGuiColors->GetHudFrameColor());
    if (CGuiWidget* w = x18_loadedMsgScreen->FindWidget("model_bottom"))
      w->SetColor(g_tweakGuiColors->GetHudFrameColor());

    x40_model_videoband->SetDepthGreater(true);
    x30_model_top->SetDepthWrite(true);
    x38_model_bottom->SetDepthWrite(true);
    x34_model_center->SetDepthWrite(true);
    x3c_model_bg->SetDepthWrite(true);

    if (x0_msg->GetStringCount()) {
      x1c_textpane_message->TextSupport().SetTypeWriteEffectOptions(false, 0.1f, 30.f);
      x1c_textpane_message->TextSupport().SetText(x0_msg->GetString(0));
      x1c_textpane_message->TextSupport().SetFontColor(g_tweakGuiColors->GetHudMessageFill());
      x1c_textpane_message->TextSupport().SetControlTXTRMap(&g_GameState->GameOptions().GetControlTXTRMap());
    }
  }

  if (x18_loadedMsgScreen) {
    if (x74_delayTime > 0.f)
      x74_delayTime -= dt;

    float xT = std::max(0.f, (x70_blurAmt - 0.5f) / 0.5f);
    float scaleX;
    if (xT < 0.7f)
      scaleX = xT / 0.7f;
    else if (xT < 0.85f)
      scaleX = 0.1f * (1.f - (xT - 0.7f) / 0.15f) + 0.9f;
    else
      scaleX = 0.1f * ((xT - 0.7f - 0.15f) / 0.3f) + 0.9f;

    x24_basewidget_center->SetLocalTransform(zeus::CTransform::Scale(scaleX, 1.f, 1.f));
    x20_basewidget_top->SetLocalTransform(zeus::CTransform::Translate(0.f, 0.f, 12.f * (1.f - xT)));
    x28_basewidget_bottom->SetLocalTransform(zeus::CTransform::Translate(0.f, 0.f, -12.f * (1.f - xT)));

    float alpha = std::max(0.f, (x70_blurAmt - 0.7f) / 0.3f);
    zeus::CColor color = g_tweakGuiColors->GetHudFrameColor();
    color.a() *= alpha;
    x20_basewidget_top->SetColor(color);
    x28_basewidget_bottom->SetColor(zeus::CColor(1.f, alpha));

    float pulse = 0.f;
    if (x74_delayTime <= 0.f)
      pulse = zeus::clamp(0.f, 0.5f * (1.f + std::sin(5.f * CGraphics::GetSecondsMod900() - M_PIF / 2.f)), 1.f);
    x2c_model_abutton->SetColor(zeus::CColor(1.f, pulse));

    x68_videoBandOffset += 12.f * dt;
    if (x68_videoBandOffset > 10.f)
      x68_videoBandOffset -= 20.f;

    x40_model_videoband->SetColor(zeus::CColor(1.f, 0.04f * (rand() / float(RAND_MAX)) + 0.08f));
    x40_model_videoband->SetLocalTransform(
        zeus::CTransform::Translate(x5c_videoBandPos + zeus::CVector3f(0.f, 0.f, x68_videoBandOffset)));

    x18_loadedMsgScreen->Update(dt);
  }

  return !x78_24_exit;
}

void CMessageScreen::Draw() const {
  if (!x18_loadedMsgScreen)
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CMessageScreen::Draw", zeus::skPurple);

  x18_loadedMsgScreen->Draw(CGuiWidgetDrawParms(x70_blurAmt, zeus::skZero3f));
}

} // namespace urde::MP1
