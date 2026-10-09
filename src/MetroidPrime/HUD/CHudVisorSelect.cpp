#include "MetroidPrime/HUD/CHudVisorSelect.hpp"

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

static const char skFrameResource[] = "FRME_VisorSelect";
static const char* const skFrameName = skFrameResource;
static CControlMapper::ECommands skCommands[] = {
    CControlMapper::kC_CombatVisor, CControlMapper::kC_ScanVisor, CControlMapper::kC_ThermalVisor,
    CControlMapper::kC_XRayVisor};

CHudVisorSelect::CHudVisorSelect(const rstl::rc_ptr< TToken< CStringTable > >& stringTable)
: mFrameLoader(rs_new CGuiFrameLoader(gpResourceFactory->GetResourceIdByName(skFrameName)->GetId(),
                                      *gpResourceFactory, *gpSimplePool))
, mFrame(nullptr)
, mAlpha(0.f)
, mSelectionFade(0.f)
, mStringTable(stringTable)
, mSelectionChanged(false) {}

void CHudVisorSelect::InitializeWidgets() {
  mModelMainFrame = mFrame->FindWidget("model_main_frame");
  mBasewidgetSelect = mFrame->FindWidget("basewidget_visiorSelect");

  mHighlights.push_back(mFrame->FindWidget("model_highlight_center"));
  mIcons.push_back(mFrame->FindWidget("model_combat_icon"));
  mHighlightAlpha.push_back(0.f);

  mHighlights.push_back(mFrame->FindWidget("model_highlight_right"));
  mIcons.push_back(mFrame->FindWidget("model_xray_icon"));
  mHighlightAlpha.push_back(0.f);

  mHighlights.push_back(mFrame->FindWidget("model_highlight_bottom"));
  mIcons.push_back(mFrame->FindWidget("model_scan_icon"));
  mHighlightAlpha.push_back(0.f);

  mHighlights.push_back(mFrame->FindWidget("model_highlight_left"));
  mIcons.push_back(mFrame->FindWidget("model_thermal_icon"));
  mHighlightAlpha.push_back(0.f);

  mTextpaneScanWarning = mFrame->FindWidget("textpane_scan_warning");
}

void CHudVisorSelect::InitializeSelection(const CStateManager& mgr) {
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

  for (int i = 1; i <= CPlayerState::kPV_Thermal; ++i) {
    const bool owned = state.HasVisor(static_cast< CPlayerState::EPlayerVisor >(i));
    mHighlights[i]->SetIsVisible(owned);
    mIcons[i]->SetIsVisible(owned);
  }
  mSelectionChanged = false;
  mSelectionFade = 1.f;
}

bool CHudVisorSelect::HasSelectionChanged(const CStateManager& mgr) const {
  const CPlayer* const player = mgr.GetPlayer();
  CPlayerState& state = *mgr.GetPlayerState();
  CPlayerState::EPlayerVisor visors[] = {CPlayerState::kPV_Combat, CPlayerState::kPV_Scan,
                                         CPlayerState::kPV_Thermal, CPlayerState::kPV_XRay};
  for (uint i = 0; i < 4; ++i) {
    if (player->GetControlMapper().GetDigitalInput(skCommands[i], player->GetLastInput(),
                                                   CControlMapper::kFT_Unfiltered)) {
      const CPlayerState::EPlayerVisor& visor = visors[i];
      return visor != state.GetCurrentVisor() && state.HasVisor(visors[i]);
    }
  }
  return false;
}

void CHudVisorSelect::UpdateInterpolation(bool increase, float& value, float step) {
  if (increase) {
    step = value + step;
    value = step < 1.f ? step : 1.f;
  } else {
    step = value - step;
    value = 0.f < step ? step : 0.f;
  }
}

void CHudVisorSelect::Update(float dt, const CStateManager& mgr) {
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
      player.GetControlMapper().GetActiveSelectorCommand() == CControlMapper::kC_VisorMenu;
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
        CControlMapper::kC_CombatVisor, player.GetLastInput(), CControlMapper::kFT_Unfiltered);
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
  UpdateInterpolation(player.GetControlMapper().GetDigitalInput(CControlMapper::kC_CombatVisor,
                                                                player.GetLastInput(),
                                                                CControlMapper::kFT_Unfiltered),
                      mHighlightAlpha[0], dt / 0.25f);
  UpdateInterpolation(player.GetControlMapper().GetDigitalInput(CControlMapper::kC_ScanVisor,
                                                                player.GetLastInput(),
                                                                CControlMapper::kFT_Unfiltered),
                      mHighlightAlpha[2], dt / 0.25f);
  UpdateInterpolation(player.GetControlMapper().GetDigitalInput(CControlMapper::kC_ThermalVisor,
                                                                player.GetLastInput(),
                                                                CControlMapper::kFT_Unfiltered),
                      mHighlightAlpha[3], dt / 0.25f);
  UpdateInterpolation(player.GetControlMapper().GetDigitalInput(CControlMapper::kC_XRayVisor,
                                                                player.GetLastInput(),
                                                                CControlMapper::kFT_Unfiltered),
                      mHighlightAlpha[1], dt / 0.25f);

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

void CHudVisorSelect::Draw() const {
  if (!close_enough(mAlpha, 0.f) && !mFrame.null()) {
    mFrame->Draw(CGuiWidgetDrawParms(mAlpha, CVector3f::Zero()));
  }
}

bool CHudVisorSelect::GetIsVisible() const { return !close_enough(mAlpha, 0.f); }

float CHudVisorSelect::GetAlpha() const { return mAlpha; }
