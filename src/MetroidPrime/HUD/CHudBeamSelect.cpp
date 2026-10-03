#include "MetroidPrime/HUD/CHudBeamSelect.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"

static const char skFrameResource[] = "FRME_BeamSelect";
static const char* const skFrameName = skFrameResource;
static CControlMapper::ECommands skCommands[] = {
    CControlMapper::kC_PowerBeam, CControlMapper::kC_IceBeam, CControlMapper::kC_WaveBeam,
    CControlMapper::kC_PlasmaBeam};

CHudBeamSelect::CHudBeamSelect(const rstl::rc_ptr< TToken< CStringTable > >& stringTable)
: mFrameLoader(rs_new CGuiFrameLoader(gpResourceFactory->GetResourceIdByName(skFrameName)->GetId(),
                                      *gpResourceFactory, *gpSimplePool))
, mFrame(nullptr)
, mAlpha(0.f)
, mSelectionFade(0.f)
, mStringTable(stringTable)
, mSelectionChanged(false) {}

void CHudBeamSelect::InitializeWidgets() {
  mModelMainFrame = mFrame->FindWidget("model_main_frame");
  mBasewidgetSelect = mFrame->FindWidget("basewidget_visiorSelect");

  mHighlights.push_back(mFrame->FindWidget("model_highlight_center"));
  mIcons.push_back(mFrame->FindWidget("model_power_icon"));
  mHighlightAlpha.push_back(0.f);

  mHighlights.push_back(mFrame->FindWidget("model_highlight_left"));
  mIcons.push_back(mFrame->FindWidget("model_ice_icon"));
  mHighlightAlpha.push_back(0.f);

  mHighlights.push_back(mFrame->FindWidget("model_highlight_bottom"));
  mIcons.push_back(mFrame->FindWidget("model_wave_icon"));
  mHighlightAlpha.push_back(0.f);

  mHighlights.push_back(mFrame->FindWidget("model_highlight_right"));
  mIcons.push_back(mFrame->FindWidget("model_plasma_icon"));
  mHighlightAlpha.push_back(0.f);
}

void CHudBeamSelect::InitializeSelection(const CStateManager& mgr) {
  const CPlayerState& state = *mgr.GetPlayerState();
  int colorIdx = 0;
  switch (state.GetCurrentVisor()) {
  case CPlayerState::kPV_Combat:
    colorIdx = 0;
    break;
  case CPlayerState::kPV_Scan:
    colorIdx = 1;
    break;
  case CPlayerState::kPV_Thermal:
    colorIdx = 3;
    break;
  case CPlayerState::kPV_XRay:
    colorIdx = 2;
    break;
  }
  const CColor color = gpTweakGuiColors->GetVisorMenuColors(colorIdx).mFrame;
  if (mBasewidgetSelect != nullptr) {
    mBasewidgetSelect->SetColor(color);
  }

  for (int i = 0; i <= CPlayerState::kIT_PlasmaBeam; ++i) {
    const bool owned = state.HasPowerUp(static_cast< CPlayerState::EItemType >(i));
    mHighlights[i]->SetIsVisible(owned);
    mIcons[i]->SetIsVisible(owned);
  }
  mSelectionChanged = false;
  mSelectionFade = 1.f;
}

bool CHudBeamSelect::HasSelectionChanged(const CStateManager& mgr) const {
  const CPlayer* const player = mgr.GetPlayer();
  CPlayerState& state = *mgr.GetPlayerState();
  const CPlayerState::EBeamId beams[] = {CPlayerState::kBI_Power, CPlayerState::kBI_Ice,
                                         CPlayerState::kBI_Wave, CPlayerState::kBI_Plasma};
  const CPlayerState::EItemType items[] = {CPlayerState::kIT_PowerBeam, CPlayerState::kIT_IceBeam,
                                           CPlayerState::kIT_WaveBeam,
                                           CPlayerState::kIT_PlasmaBeam};
  for (uint i = 0; i < 4; ++i) {
    if (player->GetControlMapper().GetDigitalInput(skCommands[i], player->GetLastInput(),
                                                   CControlMapper::kFT_Unfiltered)) {
      return beams[i] != state.GetCurrentBeam() && state.HasPowerUp(items[i]);
    }
  }
  return false;
}

void CHudBeamSelect::UpdateInterpolation(bool increase, float& value, float step) {
  if (increase) {
    step = value + step;
    value = step < 1.f ? step : 1.f;
  } else {
    step = value - step;
    value = 0.f < step ? step : 0.f;
  }
}

void CHudBeamSelect::Update(float dt, const CStateManager& mgr) {
  if (!mFrameLoader.null()) {
    if (!mFrameLoader->CheckLoadComplete()) {
      return;
    }
    mFrame = mFrameLoader->TryBuildFrame();
    mFrameLoader = nullptr;
    InitializeWidgets();
  }

  const CPlayer& player = *mgr.GetPlayer();
  const float oldAlpha = mAlpha;
  const float oldFade = mSelectionFade;
  const bool active =
      player.GetControlMapper().GetSelectorActive() == 1 &&
      player.GetControlMapper().GetActiveSelectorCommand() == CControlMapper::kC_BeamMenu;
  UpdateInterpolation(active, mSelectionFade, 3.f * dt);
  UpdateInterpolation(active || mSelectionFade > 0.f, mAlpha, 10.f * dt);
  if (close_enough(oldAlpha, 0.f) && !close_enough(oldAlpha, mAlpha)) {
    InitializeSelection(mgr);
  }
  if (oldAlpha != mAlpha) {
    if (close_enough(oldAlpha, 0.f)) {
      CSfxManager::SfxStart(0x570, 60, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else if (close_enough(oldAlpha, 1.f)) {
      CSfxManager::SfxStart(0x56e, 60, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
  }
  if (!active && close_enough(oldFade, 1.f)) {
    mSelectionChanged = HasSelectionChanged(mgr);
    const bool defaultSelected = player.GetControlMapper().GetDigitalInput(
        CControlMapper::kC_PowerBeam, player.GetLastInput(), CControlMapper::kFT_Unfiltered);
    if (mSelectionChanged && !defaultSelected) {
      if (mSelectionSfx && CSfxManager::IsPlaying(mSelectionSfx)) {
        CSfxManager::SfxStop(mSelectionSfx);
      }
      mSelectionSfx = CSfxManager::SfxStart(0x590, 30, 64, false, CSfxManager::kMedPriority, false,
                                            CSfxManager::kAllAreas);
      CSfxManager::PitchBend(mSelectionSfx, 0x8000);
    }
  }

  mModelMainFrame->SetO2PTransform(mModelMainFrame->GetIdleXform() *
                                   CTransform4f::Scale(mAlpha, 1.f, mAlpha));
  UpdateInterpolation(player.GetControlMapper().GetDigitalInput(CControlMapper::kC_PowerBeam,
                                                                player.GetLastInput(),
                                                                CControlMapper::kFT_Unfiltered),
                      mHighlightAlpha[0], dt / 0.25f);
  UpdateInterpolation(player.GetControlMapper().GetDigitalInput(CControlMapper::kC_IceBeam,
                                                                player.GetLastInput(),
                                                                CControlMapper::kFT_Unfiltered),
                      mHighlightAlpha[1], dt / 0.25f);
  UpdateInterpolation(player.GetControlMapper().GetDigitalInput(CControlMapper::kC_WaveBeam,
                                                                player.GetLastInput(),
                                                                CControlMapper::kFT_Unfiltered),
                      mHighlightAlpha[2], dt / 0.25f);
  UpdateInterpolation(player.GetControlMapper().GetDigitalInput(CControlMapper::kC_PlasmaBeam,
                                                                player.GetLastInput(),
                                                                CControlMapper::kFT_Unfiltered),
                      mHighlightAlpha[3], dt / 0.25f);

  float flash = 1.f;
  if (!active && mSelectionChanged) {
    flash = (1.f + CMath::FastCosR(1.5f * (2.f * (M_PIF * mSelectionFade)))) / 2.f;
  }
  const CColor& white = CColor(CColor::White());
  for (int i = 0; i < mHighlights.size(); ++i) {
    if (mHighlights[i] != nullptr) {
      mHighlights[i]->SetColor(white.WithAlphaOf(flash * mHighlightAlpha[i]));
    }
  }
}

void CHudBeamSelect::Draw() const {
  if (!close_enough(mAlpha, 0.f) && !mFrame.null()) {
    mFrame->Draw(CGuiWidgetDrawParms(mAlpha, CVector3f::Zero()));
  }
}

bool CHudBeamSelect::GetIsVisible() const { return !close_enough(mAlpha, 0.f); }

float CHudBeamSelect::GetAlpha() const { return mAlpha; }
