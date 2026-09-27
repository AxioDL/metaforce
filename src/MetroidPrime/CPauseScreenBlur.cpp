#include "MetroidPrime/CPauseScreenBlur.hpp"

#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/SFX/UI.h"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"

#include "float.h"
#include "rstl/math.hpp"

CPauseScreenBlur::CPauseScreenBlur()
: mMapLightQuarter(gpSimplePool->GetObj("TXTR_MapLightQuarter"))
, mPrevState(kS_InGame)
, mNextState(kS_InGame)
, mBlurAmt(0.f)
, mBlurring(false)
, mGameDraw(true) {}

CPauseScreenBlur::~CPauseScreenBlur() {}

void CPauseScreenBlur::OnNewInGameGuiState(EInGameGuiState state, const CStateManager& stateMgr) {
  bool valid = false;
  if (state >= 0 && state <= 1) {
    valid = true;
  }
  if (valid) {
    SetState(kS_InGame);
    return;
  }

  switch (state) {
  case kIGGS_MapScreen:
    SetState(kS_MapScreen);
    break;
  case kIGGS_PauseSaveGame:
    SetState(kS_SaveGame);
    break;
  case kIGGS_PauseHUDMessage:
    SetState(kS_HUDMessage);
    break;
  case kIGGS_PauseGame:
  case kIGGS_PauseLogBook:
    SetState(kS_Pause);
    break;
  }
}

void CPauseScreenBlur::Update(float dt, const CStateManager& stateMgr, bool b) {
  if (mPrevState == mNextState)
    return;

  if (mBlurAmt < 0.f)
    mBlurAmt = rstl::min_val(0.f, 2.f * dt + mBlurAmt);
  else if (mBlurAmt > 0.f)
    mBlurAmt = rstl::min_val(1.f, 2.f * dt + mBlurAmt);

  if (mBlurAmt == 0.f || mBlurAmt == 1.f)
    OnBlurComplete(b);

  if (mBlurAmt == 0.f && b) {
    mCamBlur.DisableBlur(0.f);
  } else {
    mCamBlur.SetBlur(CCameraBlurPass::kBT_HiBlur,
                        gpTweakGui->GetPauseBlurFactor() * GetBlurAmtInline(), 0.f, true);
    mBlurring = true;
  }
}

void CPauseScreenBlur::Draw(const CStateManager&) {
  mCamBlur.Draw();
  CGraphics::DisableAllLights();
  CGraphics::SetAmbientColor(CColor(0xffffffff));
  const float t = fabs(mBlurAmt);
  if (mCamBlur.GetCurrType() != CCameraBlurPass::kBT_NoBlur) {
    const CColor& white = CColor::White();
    CCameraFilterPass::DrawFilter(
        CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_FullscreenQuarters,
        CColor::Lerp(white, gpTweakGuiColors->GetPauseBlurFilterColor(), t), *mMapLightQuarter,
        t);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply,
                                  CCameraFilterPass::kFS_ScanLinesEven,
                                  CColor::Lerp(CColor::White(), CColor(0xc0c0c0ff), t), nullptr, t);
  }

  if (mBlurring && mCamBlur.GetNoPersistentCopy()) {
    mBlurring = false;
    mGameDraw = false;
  }
}

void CPauseScreenBlur::OnBlurComplete(bool b) {
  if (mNextState == kS_InGame && !b)
    return;
  mPrevState = mNextState;
  if (mPrevState == kS_InGame)
    mGameDraw = true;
}

void CPauseScreenBlur::SetState(EState state) {
  switch (mPrevState) {
  case kS_InGame:
    if (state != kS_InGame) {
      CSfxManager::SetChannel(CSfxManager::kSC_PauseScreen);
      if (state == kS_HUDMessage)
        CSfxManager::SfxStart(SFXui_x_override_00, 0x7f, 0x40, false, CSfxManager::kMedPriority);
      else if (state == kS_MapScreen)
        CSfxManager::SfxStart(SFXui_x_pause_00, 0x7f, 0x40, false, CSfxManager::kMedPriority);
      mBlurAmt = FLT_EPSILON;
    }
    break;
  case kS_MapScreen:
  case kS_SaveGame:
  case kS_HUDMessage:
  case kS_Pause:
    break;
  }

  switch (state) {
  case kS_InGame: {
    bool valid = true;
    if (mPrevState == kS_InGame && mNextState == kS_InGame) {
      valid = false;
    }
    if (valid) {
      CSfxManager::SetChannel(CSfxManager::kSC_Game);

      if (mPrevState == kS_HUDMessage)
        CSfxManager::SfxStart(SFXui_x_override_01, 0x7f, 0x40, false, CSfxManager::kMedPriority);
      else if (mPrevState == kS_MapScreen)
        CSfxManager::SfxStart(SFXui_x_pause_01, 0x7f, 0x40, false, CSfxManager::kMedPriority);

      mBlurAmt = -1.f;
    }
    break;
  }
  case kS_MapScreen:
  case kS_SaveGame:
  case kS_HUDMessage:
  case kS_Pause:
    break;
  }

  mNextState = state;
}

float CPauseScreenBlur::GetBlurAmt() const { return GetBlurAmtInline(); }
