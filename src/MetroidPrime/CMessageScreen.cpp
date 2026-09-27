#include "MetroidPrime/CMessageScreen.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/SFX/UI.h"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"

#include <stdlib.h>

CMessageScreen::CMessageScreen(CAssetId msg, float time)
: mMsg(gpSimplePool->GetObj(SObjectTag('STRG', msg)))
, mMsgScreen(gpSimplePool->GetObj("FRME_MsgScreen"))
, mLoadedMsgScreen(nullptr)
, mTopPos(CVector3f::Zero())
, mBottomPos(CVector3f::Zero())
, mVideoBandPos(CVector3f::Zero())
, mVideoBandOffset(10.f)
, mPage(0)
, mBlurAmt(0.f)
, mDelayTime(time)
, mExit(false) {
  mMsgScreen.Lock();
  mMsg.Lock();
}

bool CMessageScreen::Update(float dt, float blurAmt) {
  mBlurAmt = blurAmt;
  if (!mLoadedMsgScreen) {
    const bool ready = mMsgScreen.TryCache() && mMsg.TryCache();
    if (ready) {
      mLoadedMsgScreen = mMsgScreen.GetObject();
      mTextpane_message =
          static_cast< CGuiTextPane* >(mLoadedMsgScreen->FindWidget("textpane_message"));
      mBasewidget_top = mLoadedMsgScreen->FindWidget("basewidget_top");
      mBasewidget_center = mLoadedMsgScreen->FindWidget("basewidget_center");
      mBasewidget_bottom = mLoadedMsgScreen->FindWidget("basewidget_bottom");
      mModel_abutton =
          static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_abutton"));
      mModel_top = static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_top"));
      mModel_bottom = static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_bottom"));
      mModel_center = static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_center"));
      mModel_bg = static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_bg"));
      mModel_videoband =
          static_cast< CGuiModel* >(mLoadedMsgScreen->FindWidget("model_videoband"));
      mTopPos = mBasewidget_top->GetLocalPosition();
      mBottomPos = mBasewidget_bottom->GetLocalPosition();
      mVideoBandPos = mModel_videoband->GetLocalPosition();

      if (CGuiWidget* widget = mLoadedMsgScreen->FindWidget("basewidget_top")) {
        widget->SetColor(gpTweakGuiColors->GetHudFrameColor());
      }
      if (CGuiWidget* widget = mLoadedMsgScreen->FindWidget("basewidget_centerdeco")) {
        widget->SetColor(gpTweakGuiColors->GetHudFrameColor());
      }
      if (CGuiWidget* widget = mLoadedMsgScreen->FindWidget("model_bottom")) {
        widget->SetColor(gpTweakGuiColors->GetHudFrameColor());
      }

      mModel_videoband->SetIsDepthBackwards(true);
      mModel_top->SetIsAlwaysDepthWrite(true);
      mModel_bottom->SetIsAlwaysDepthWrite(true);
      mModel_center->SetIsAlwaysDepthWrite(true);
      mModel_bg->SetIsAlwaysDepthWrite(true);

      if (mMsg.GetObject()->GetStringCount() > 0) {
        CGuiTextSupport& text = mTextpane_message->TextSupport();
        text.SetTypeWriteEffectOptions(false, 0.1f, 30.f);
        text.SetText(mMsg.GetObject()->GetString(0));
        text.SetFontColor(gpTweakGuiColors->GetHudMessageFill());
        text.SetControlTXTRMap(&gpGameState->GameOptions().GetControlTXTRMap());
      }
    }
  }

  if (mLoadedMsgScreen) {
    if (mDelayTime > 0.f) {
      mDelayTime -= dt;
    }

    float xT = rstl::max_val(0.f, (mBlurAmt - 0.5f) / 0.5f);
    float scaleX;
    if (xT < 0.7f) {
      scaleX = xT / 0.7f;
    } else if (xT < 0.85f) {
      scaleX = (1.f - 0.9f) * (1.f - (xT - 0.7f) / 0.15f) + 0.9f;
    } else {
      scaleX = (1.f - 0.9f) * ((xT - 0.7f - 0.15f) / 0.3f) + 0.9f;
    }
    mBasewidget_center->SetO2PTransform(CTransform4f::Scale(scaleX, 1.f, 1.f));
    mBasewidget_top->SetO2PTransform(CTransform4f::Translate(0.f, 0.f, 12.f * (1.f - xT)));
    mBasewidget_bottom->SetO2PTransform(CTransform4f::Translate(0.f, 0.f, -12.f * (1.f - xT)));

    float alpha = rstl::max_val(0.f, (mBlurAmt - 0.7f) / 0.3f);
    mBasewidget_top->SetColor(gpTweakGuiColors->GetHudFrameColor().WithAlphaModulatedBy(alpha));
    mBasewidget_bottom->SetColor(CColor::White().WithAlphaOf(alpha));

    const float pulse =
        mDelayTime <= 0.f
            ? CMath::Clamp(
                  0.f,
                  0.5f * (1.f + CMath::FastSinR(5.f * CGraphics::GetSecondsMod900() - M_PIF / 2.f)),
                  1.f)
            : 0.f;
    mModel_abutton->SetColor(CColor::White().WithAlphaOf(pulse));

    mVideoBandOffset += 12.f * dt;
    if (mVideoBandOffset > 10.f) {
      mVideoBandOffset -= 20.f;
    }
    float random = rand() / static_cast< float >(RAND_MAX);
    mModel_videoband->SetColor(CColor::White().WithAlphaOf(0.04f * random + 0.08f));
    mModel_videoband->SetO2PTransform(
        CTransform4f::Translate(mVideoBandPos + CVector3f(0.f, 0.f, mVideoBandOffset)));
    mLoadedMsgScreen->Update(dt);
  }

  return !mExit;
}

void CMessageScreen::ProcessControllerInput(const CFinalInput& input) {
  if (mLoadedMsgScreen && mDelayTime <= 0.f && input.PA()) {
    CGuiTextSupport& text = mTextpane_message->TextSupport();
    if (text.GetCurTime() < text.GetTotalAnimationTime()) {
      text.SetCurTime(text.GetTotalAnimationTime());
      return;
    }

    ++mPage;
    if (mPage >= mMsg.GetObject()->GetStringCount()) {
      mExit = true;
      return;
    }

    text.SetTypeWriteEffectOptions(false, 0.1f, 30.f);
    text.SetText(mMsg.GetObject()->GetString(mPage));
    CSfxManager::SfxStart(SFXui_x_override_02);
    mDelayTime = 0.8f;
  }
}

void CMessageScreen::Draw() const {
  if (mLoadedMsgScreen) {
    mLoadedMsgScreen->Draw(CGuiWidgetDrawParms(mBlurAmt, CVector3f::Zero()));
  }
}
