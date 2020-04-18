#include "Runtime/MP1/CInventoryScreen.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTableGroup.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/Input/ControlMapper.hpp"
#include "Runtime/IMain.hpp"

namespace urde::MP1 {
namespace {
struct SInventoryItem {
  u32 idx;
  u32 nameStrIdx;
  u32 entryStrIdx;
};

constexpr std::array<SInventoryItem, 5> ArmCannonItems{{
    {0, 0x24, 0x46}, // Power Beam
    {1, 0x25, 0x48}, // Ice Beam
    {2, 0x26, 0x4a}, // Wave Beam
    {3, 0x27, 0x4c}, // Plasma Beam
    {4, 0x28, 0x4e}, // Phazon Beam
}};

constexpr std::array<SInventoryItem, 5> MorphballItems{{
    {5, 0x2e, 0x57}, // Morph Ball
    {6, 0x2f, 0x58}, // Boost Ball
    {7, 0x30, 0x59}, // Spider Ball
    {8, 0x31, 0x5a}, // Morph Ball Bomb
    {9, 0x32, 0x5b}, // Power Bomb
}};

constexpr std::array<SInventoryItem, 5> SuitItems{{
    {10, 0x33, 0x52}, // Power Suit
    {11, 0x34, 0x53}, // Varia Suit
    {12, 0x35, 0x54}, // Gravity Suit
    {13, 0x36, 0x55}, // Phazon Suit
    {14, 0x37, 0x56}, // Energy Tank
}};

constexpr std::array<SInventoryItem, 4> VisorItems{{
    {15, 0x38, 0x42}, // Combat Visor
    {16, 0x39, 0x43}, // Scan Visor
    {17, 0x3a, 0x44}, // X-Ray Visor
    {18, 0x3b, 0x45}, // Thermal Visor
}};

constexpr std::array<SInventoryItem, 5> SecondaryItems{{
    {19, 0x3c, 0x4f}, // Space Jump Boots
    {20, 0x3d, 0x50}, // Grapple Beam
    {21, 0x3e, 0x51}, // Missile Launcher
    {22, 0x3f, 0x5c}, // Charge Beam
    {23, 0x40, 0x5d}, // Beam Combo
}};

constexpr std::array<std::pair<size_t, const SInventoryItem*>, 5> InventoryRegistry{{
    {ArmCannonItems.size(), ArmCannonItems.data()},
    {MorphballItems.size(), MorphballItems.data()},
    {SuitItems.size(), SuitItems.data()},
    {VisorItems.size(), VisorItems.data()},
    {SecondaryItems.size(), SecondaryItems.data()},
}};
} // Anonymous namespace

CInventoryScreen::CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg,
                                   const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp)
: CPauseScreenBase(mgr, frame, pauseStrg) {
  CPlayerState& playerState = *mgr.GetPlayerState();
  x19c_samusDoll = std::make_unique<CSamusDoll>(
      suitDgrp, ballDgrp,
      CPlayerState::EPlayerSuit(int(playerState.GetCurrentSuitRaw()) + (playerState.IsFusionEnabled() * 4)),
      playerState.GetCurrentBeam(), playerState.HasPowerUp(CPlayerState::EItemType::SpiderBall),
      playerState.HasPowerUp(CPlayerState::EItemType::GrappleBeam));
}

CInventoryScreen::~CInventoryScreen() {
  for (int i = 0; i < 5; ++i) {
    xd8_textpane_titles[i]->TextSupport().SetFontColor(zeus::skWhite);
    x15c_model_righttitledecos[i]->SetColor(zeus::skWhite);
    x144_model_titles[i]->SetColor(zeus::skWhite);
  }
  x8c_model_righthighlight->SetColor(zeus::skWhite);
}

bool CInventoryScreen::InputDisabled() const {
  return std::fabs(x19c_samusDoll->GetViewInterpolation()) > 0 || x1a8_state == EState::Leaving;
}

void CInventoryScreen::TransitioningAway() { x1a8_state = EState::Leaving; }

void CInventoryScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue) {
  CPauseScreenBase::Update(dt, rand, archQueue);
  x19c_samusDoll->Update(dt, rand);

  if (x10_mode == EMode::TextScroll) {
    if (x1ad_textViewing)
      x1a4_textBodyAlpha = std::min(4.f * dt + x1a4_textBodyAlpha, 1.f);
    else
      x1a4_textBodyAlpha = std::max(0.f, x1a4_textBodyAlpha - 4.f * dt);
    if (x1a4_textBodyAlpha == 0.f && x1a8_state == EState::Active)
      ChangeMode(EMode::RightTable);
  } else {
    x1a4_textBodyAlpha = std::max(0.f, x1a4_textBodyAlpha - 4.f * dt);
  }
  x174_textpane_body->SetColor(zeus::CColor(1.f, x1a4_textBodyAlpha));
  x180_basewidget_yicon->SetColor(zeus::CColor(1.f, 1.f - x1a4_textBodyAlpha));

  x19c_samusDoll->SetInMorphball(x70_tablegroup_leftlog->GetUserSelection() == 1 && x10_mode != EMode::LeftTable);
  UpdateSamusDollPulses();
  if (x1a8_state == EState::Leaving && x1a4_textBodyAlpha == 0.f)
    x1a8_state = EState::Inactive;
}

void CInventoryScreen::Touch() {
  CPauseScreenBase::Touch();
  x19c_samusDoll->Touch();
}

void CInventoryScreen::ProcessControllerInput(const CFinalInput& input) {
  float viewInterp = x19c_samusDoll->GetViewInterpolation();
  if (x1a8_state == EState::Inactive || (viewInterp != 0.f && viewInterp != 1.f))
    return;

  float absViewInterp = std::fabs(viewInterp);
  if ((input.PY() || ((m_bodyUpClicked || m_bodyClicked) && absViewInterp == 0.f)) &&
      x19c_samusDoll->IsLoaded() && (absViewInterp > 0.f || x10_mode != EMode::TextScroll)) {
    x19c_samusDoll->BeginViewInterpolate(absViewInterp == 0.f);
    if (absViewInterp == 0.f) {
      if (const auto& kbm = input.GetKBM()) {
        m_lastMouseCoord = zeus::CVector2f(kbm->m_mouseCoord.norm[0], kbm->m_mouseCoord.norm[1]);
        m_lastAccumScroll = kbm->m_accumScroll;
        m_dollScroll = boo::SScrollDelta();
      }
    }
  }

  if (absViewInterp == 1.f) {
    if (input.PStart()) {
      x19c_samusDoll->BeginViewInterpolate(false);
      x198_26_exitPauseScreen = true;
    } else if (input.PB() || input.PSpecialKey(boo::ESpecialKey::Esc)) {
      x19c_samusDoll->BeginViewInterpolate(false);
    }
  }

  if (std::fabs(x19c_samusDoll->GetViewInterpolation()) > 0.f) {
    CPauseScreenBase::ResetMouseState();

    float motionAmt = input.DeltaTime() * 6.f;
    float circleUp = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleUp, input);
    float circleDown = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleDown, input);
    float circleLeft = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleLeft, input);
    float circleRight = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleRight, input);
    float moveForward = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveForward, input);
    float moveBack = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveBack, input);
    float moveLeft = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveLeft, input);
    float moveRight = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveRight, input);
    float zoomIn = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapZoomIn, input);
    float zoomOut = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapZoomOut, input);

    if (const auto& kbm = input.GetKBM()) {
      zeus::CVector2f mouseCoord = zeus::CVector2f(kbm->m_mouseCoord.norm[0], kbm->m_mouseCoord.norm[1]);
      zeus::CVector2f mouseDelta = mouseCoord - m_lastMouseCoord;
      m_lastMouseCoord = mouseCoord;
      mouseDelta.x() *= g_Viewport.aspect;
      mouseDelta *= 100.f;
      if (kbm->m_mouseButtons[size_t(boo::EMouseButton::Middle)] ||
          kbm->m_mouseButtons[size_t(boo::EMouseButton::Secondary)]) {
        if (float(mouseDelta.x()) < 0.f)
          moveRight += -mouseDelta.x();
        else if (float(mouseDelta.x()) > 0.f)
          moveLeft += mouseDelta.x();
        if (float(mouseDelta.y()) < 0.f)
          moveForward += -mouseDelta.y();
        else if (float(mouseDelta.y()) > 0.f)
          moveBack += mouseDelta.y();
      }
      if (kbm->m_mouseButtons[size_t(boo::EMouseButton::Primary)]) {
        if (float(mouseDelta.x()) < 0.f)
          circleRight += -mouseDelta.x();
        else if (float(mouseDelta.x()) > 0.f)
          circleLeft += mouseDelta.x();
        if (float(mouseDelta.y()) < 0.f)
          circleUp += -mouseDelta.y();
        else if (float(mouseDelta.y()) > 0.f)
          circleDown += mouseDelta.y();
      }

      m_dollScroll += kbm->m_accumScroll - m_lastAccumScroll;
      m_lastAccumScroll = kbm->m_accumScroll;
      if (m_dollScroll.delta[1] > 0.0) {
        zoomIn = 1.f;
        m_dollScroll.delta[1] = std::max(0.0, m_dollScroll.delta[1] - (15.0 / 60.0));
      } else if (m_dollScroll.delta[1] < 0.0) {
        if (x19c_samusDoll->IsZoomedOut()) {
          x19c_samusDoll->BeginViewInterpolate(false);
          return;
        }
        zoomOut = 1.f;
        m_dollScroll.delta[1] = std::min(0.0, m_dollScroll.delta[1] + (15.0 / 60.0));
      }
    }

    zeus::CVector3f moveVec = {(moveRight - moveLeft) * 0.25f * motionAmt, (zoomIn - zoomOut) * 0.5f * motionAmt,
                               (moveForward - moveBack) * 0.25f * motionAmt};
    x19c_samusDoll->SetOffset(moveVec, input.DeltaTime());
    x19c_samusDoll->SetRotation(0.5f * motionAmt * (circleDown - circleUp),
                                0.5f * motionAmt * (circleRight - circleLeft), input.DeltaTime());
  } else {
    x1ad_textViewing = false;
    if (x10_mode == EMode::TextScroll) {
      int oldPage = x174_textpane_body->TextSupport().GetPageCounter();
      int newPage = oldPage;
      int totalCount = x174_textpane_body->TextSupport().GetTotalPageCount();
      bool lastPage = totalCount - 1 == oldPage;
      if (totalCount != -1) {
        if (input.PLAUp() || m_bodyUpClicked)
          newPage = std::max(oldPage - 1, 0);
        else if (input.PLADown() || m_bodyDownClicked ||
                 ((input.PA() || input.PSpecialKey(boo::ESpecialKey::Enter) || m_bodyClicked) && !lastPage))
          newPage = std::min(oldPage + 1, totalCount - 1);
        x174_textpane_body->TextSupport().SetPage(newPage);
        if (oldPage != newPage)
          CSfxManager::SfxStart(SFXui_pause_screen_next_page, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x198_28_pulseTextArrowTop = newPage > 0;
        x198_29_pulseTextArrowBottom = !lastPage;
      } else {
        x198_29_pulseTextArrowBottom = false;
        x198_28_pulseTextArrowTop = false;
      }
      if (!x1ac_textLeaveRequested)
        x1ac_textLeaveRequested = input.PB() || input.PSpecialKey(boo::ESpecialKey::Esc) ||
          ((input.PA() || m_bodyClicked || input.PSpecialKey(boo::ESpecialKey::Enter)) && lastPage);
      x1ad_textViewing = !x1ac_textLeaveRequested;
    } else {
      x198_29_pulseTextArrowBottom = false;
      x198_28_pulseTextArrowTop = false;
    }

    if (x1a8_state != EState::Active)
      x1ad_textViewing = false;

    CPauseScreenBase::ProcessMouseInput(input, absViewInterp);
    CPauseScreenBase::ProcessControllerInput(input);
  }
}

void CInventoryScreen::Draw(float transInterp, float totalAlpha, float yOff) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CInventoryScreen::Draw", zeus::skPurple);
  CPauseScreenBase::Draw(transInterp, totalAlpha, std::fabs(x19c_samusDoll->GetViewInterpolation()));
  x19c_samusDoll->Draw(x4_mgr, transInterp * (1.f - x1a4_textBodyAlpha));
}

float CInventoryScreen::GetCameraYBias() const { return std::fabs(x19c_samusDoll->GetViewInterpolation()); }

bool CInventoryScreen::VReady() const { return true; }

bool CInventoryScreen::HasLeftInventoryItem(int idx) const {
  CPlayerState& playerState = *x4_mgr.GetPlayerState();
  switch (idx) {
  case 0: // Arm Cannon
    return true;
  case 1: // Morphball
    return playerState.HasPowerUp(CPlayerState::EItemType::MorphBall);
  case 2: // Suit
    return true;
  case 3: // Visor
    return true;
  case 4: // Secondary
    return playerState.HasPowerUp(CPlayerState::EItemType::SpaceJumpBoots) ||
           playerState.HasPowerUp(CPlayerState::EItemType::GrappleBeam) ||
           playerState.HasPowerUp(CPlayerState::EItemType::Missiles) ||
           playerState.HasPowerUp(CPlayerState::EItemType::ChargeBeam) ||
           playerState.HasPowerUp(CPlayerState::EItemType::SuperMissile) ||
           playerState.HasPowerUp(CPlayerState::EItemType::IceSpreader) ||
           playerState.HasPowerUp(CPlayerState::EItemType::Wavebuster) ||
           playerState.HasPowerUp(CPlayerState::EItemType::Flamethrower);
  default:
    return false;
  }
}

void CInventoryScreen::VActivate() {
  for (int i = 0; i < 5; ++i) {
    if (HasLeftInventoryItem(i)) {
      if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
        xa8_textpane_categories[i]->TextSupport().SetText(xc_pauseStrg.GetString(i + 10));
      } else {
        xa8_textpane_categories[i]->TextSupport().SetText(xc_pauseStrg.GetString(i + 12));
      }
    } else {
      xa8_textpane_categories[i]->TextSupport().SetText(u"??????");
      x70_tablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
    }
  }

  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
    x178_textpane_title->TextSupport().SetText(xc_pauseStrg.GetString(9));
  } else {
    x178_textpane_title->TextSupport().SetText(xc_pauseStrg.GetString(11));
  }
  x180_basewidget_yicon->SetVisibility(true, ETraversalMode::Children);
}

void CInventoryScreen::RightTableSelectionChanged(int oldSel, int newSel) {}

void CInventoryScreen::UpdateTextBody() {
  x1ac_textLeaveRequested = false;

  const SInventoryItem& sel = InventoryRegistry[x70_tablegroup_leftlog->GetUserSelection()].second[x1c_rightSel];
  std::u16string entryText = xc_pauseStrg.GetString((g_Main->IsUSA() && !g_Main->IsTrilogy()) ? sel.entryStrIdx : sel.entryStrIdx + 3);

  if (sel.idx == 23) // Beam combo
  {
    CPlayerState& playerState = *x4_mgr.GetPlayerState();
    if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
      entryText += xc_pauseStrg.GetString(playerState.HasPowerUp(CPlayerState::EItemType::SuperMissile) ? 71 : 65);
      entryText += xc_pauseStrg.GetString(playerState.HasPowerUp(CPlayerState::EItemType::IceSpreader) ? 73 : 65);
      entryText += xc_pauseStrg.GetString(playerState.HasPowerUp(CPlayerState::EItemType::Wavebuster) ? 75 : 65);
      entryText += xc_pauseStrg.GetString(playerState.HasPowerUp(CPlayerState::EItemType::Flamethrower) ? 77 : 65);
    } else {
      entryText += xc_pauseStrg.GetString(playerState.HasPowerUp(CPlayerState::EItemType::SuperMissile) ? 73 : 68);
      entryText += xc_pauseStrg.GetString(playerState.HasPowerUp(CPlayerState::EItemType::IceSpreader) ? 75 : 68);
      entryText += xc_pauseStrg.GetString(playerState.HasPowerUp(CPlayerState::EItemType::Wavebuster) ? 78 : 68);
      entryText += xc_pauseStrg.GetString(playerState.HasPowerUp(CPlayerState::EItemType::Flamethrower) ? 79 : 68);
    }
  }

  x174_textpane_body->TextSupport().SetText(entryText, true);
  x174_textpane_body->TextSupport().SetPage(0);
}

void CInventoryScreen::ChangedMode(EMode oldMode) {
  if (x10_mode == EMode::TextScroll) {
    x1ad_textViewing = true;
    UpdateTextBody();
  }
}

bool CInventoryScreen::HasRightInventoryItem(int idx) const {
  CPlayerState& playerState = *x4_mgr.GetPlayerState();
  switch (idx) {
  case 0: // Power Beam
    return true;
  case 1: // Ice Beam
    return playerState.HasPowerUp(CPlayerState::EItemType::IceBeam);
  case 2: // Wave Beam
    return playerState.HasPowerUp(CPlayerState::EItemType::WaveBeam);
  case 3: // Plasma Beam
    return playerState.HasPowerUp(CPlayerState::EItemType::PlasmaBeam);
  case 4: // Phazon Beam
    return playerState.HasPowerUp(CPlayerState::EItemType::PhazonSuit);
  case 5: // Morph Ball
    return playerState.HasPowerUp(CPlayerState::EItemType::MorphBall);
  case 6: // Boost Ball
    return playerState.HasPowerUp(CPlayerState::EItemType::BoostBall);
  case 7: // Spider Ball
    return playerState.HasPowerUp(CPlayerState::EItemType::SpiderBall);
  case 8: // Morph Ball Bomb
    return playerState.HasPowerUp(CPlayerState::EItemType::MorphBallBombs);
  case 9: // Power Bomb
    return playerState.HasPowerUp(CPlayerState::EItemType::PowerBombs);
  case 10: // Power Suit
    return true;
  case 11: // Varia Suit
    return playerState.HasPowerUp(CPlayerState::EItemType::VariaSuit);
  case 12: // Gravity Suit
    return playerState.HasPowerUp(CPlayerState::EItemType::GravitySuit);
  case 13: // Phazon Suit
    return playerState.HasPowerUp(CPlayerState::EItemType::PhazonSuit);
  case 14: // Energy Tank
    return playerState.HasPowerUp(CPlayerState::EItemType::EnergyTanks);
  case 15: // Combat Visor
    return true;
  case 16: // Scan Visor
    return playerState.HasPowerUp(CPlayerState::EItemType::ScanVisor);
  case 17: // X-Ray Visor
    return playerState.HasPowerUp(CPlayerState::EItemType::XRayVisor);
  case 18: // Thermal Visor
    return playerState.HasPowerUp(CPlayerState::EItemType::ThermalVisor);
  case 19: // Space Jump Boots
    return playerState.HasPowerUp(CPlayerState::EItemType::SpaceJumpBoots);
  case 20: // Grapple Beam
    return playerState.HasPowerUp(CPlayerState::EItemType::GrappleBeam);
  case 21: // Missile Launcher
    return playerState.HasPowerUp(CPlayerState::EItemType::Missiles);
  case 22: // Charge Beam
    return playerState.HasPowerUp(CPlayerState::EItemType::ChargeBeam);
  case 23: // Beam Combo
    return playerState.HasPowerUp(CPlayerState::EItemType::SuperMissile) ||
           playerState.HasPowerUp(CPlayerState::EItemType::IceSpreader) ||
           playerState.HasPowerUp(CPlayerState::EItemType::Wavebuster) ||
           playerState.HasPowerUp(CPlayerState::EItemType::Flamethrower);
  default:
    return false;
  }
}

bool CInventoryScreen::IsRightInventoryItemEquipped(int idx) const {
  CPlayerState& playerState = *x4_mgr.GetPlayerState();
  switch (idx) {
  case 0: // Power Beam
    return playerState.GetCurrentBeam() == CPlayerState::EBeamId::Power;
  case 1: // Ice Beam
    return playerState.GetCurrentBeam() == CPlayerState::EBeamId::Ice;
  case 2: // Wave Beam
    return playerState.GetCurrentBeam() == CPlayerState::EBeamId::Wave;
  case 3: // Plasma Beam
    return playerState.GetCurrentBeam() == CPlayerState::EBeamId::Plasma;
  case 4: // Phazon Beam
    return playerState.GetCurrentBeam() == CPlayerState::EBeamId::Phazon2;
  case 5: // Morph Ball
    return playerState.HasPowerUp(CPlayerState::EItemType::MorphBall);
  case 6: // Boost Ball
    return playerState.HasPowerUp(CPlayerState::EItemType::BoostBall);
  case 7: // Spider Ball
    return playerState.HasPowerUp(CPlayerState::EItemType::SpiderBall);
  case 8: // Morph Ball Bomb
    return playerState.HasPowerUp(CPlayerState::EItemType::MorphBallBombs);
  case 9: // Power Bomb
    return playerState.HasPowerUp(CPlayerState::EItemType::PowerBombs);
  case 10: // Power Suit
    return playerState.GetCurrentSuit() == CPlayerState::EPlayerSuit::Power;
  case 11: // Varia Suit
    return playerState.GetCurrentSuit() == CPlayerState::EPlayerSuit::Varia;
  case 12: // Gravity Suit
    return playerState.GetCurrentSuit() == CPlayerState::EPlayerSuit::Gravity;
  case 13: // Phazon Suit
    return playerState.GetCurrentSuit() == CPlayerState::EPlayerSuit::Phazon;
  case 14: // Energy Tank
    return playerState.HasPowerUp(CPlayerState::EItemType::EnergyTanks);
  case 15: // Combat Visor
    return playerState.GetCurrentVisor() == CPlayerState::EPlayerVisor::Combat;
  case 16: // Scan Visor
    return playerState.GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan;
  case 17: // X-Ray Visor
    return playerState.GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay;
  case 18: // Thermal Visor
    return playerState.GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal;
  case 19: // Space Jump Boots
    return playerState.HasPowerUp(CPlayerState::EItemType::SpaceJumpBoots);
  case 20: // Grapple Beam
    return playerState.HasPowerUp(CPlayerState::EItemType::GrappleBeam);
  case 21: // Missile Launcher
    return playerState.HasPowerUp(CPlayerState::EItemType::Missiles);
  case 22: // Charge Beam
    return playerState.HasPowerUp(CPlayerState::EItemType::ChargeBeam);
  case 23: // Beam Combo
    return playerState.HasPowerUp(CPlayerState::EItemType::SuperMissile) ||
           playerState.HasPowerUp(CPlayerState::EItemType::IceSpreader) ||
           playerState.HasPowerUp(CPlayerState::EItemType::Wavebuster) ||
           playerState.HasPowerUp(CPlayerState::EItemType::Flamethrower);
  default:
    return false;
  }
}

void CInventoryScreen::UpdateRightTable() {
  CPauseScreenBase::UpdateRightTable();
  const auto& [size, data] = InventoryRegistry[x70_tablegroup_leftlog->GetUserSelection()];

  int minSel = INT_MAX;
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* title = xd8_textpane_titles[i];
    if (i < int(size)) {
      if (HasRightInventoryItem(data[i].idx)) {
        title->TextSupport().SetText(xc_pauseStrg.GetString((g_Main->IsUSA() && !g_Main->IsTrilogy()) ? data[i].nameStrIdx : data[i].nameStrIdx + 3));
        x84_tablegroup_rightlog->GetWorkerWidget(i + 1)->SetIsSelectable(true);
        if (i < minSel)
          minSel = i;
      } else {
        title->TextSupport().SetText(u"??????");
        x84_tablegroup_rightlog->GetWorkerWidget(i + 1)->SetIsSelectable(false);
      }
    } else {
      title->TextSupport().SetText(u"??????");
    }
  }

  if (minSel != INT_MAX) {
    x1c_rightSel = minSel;
    SetRightTableSelection(x1c_rightSel, x1c_rightSel);
  }

  x84_tablegroup_rightlog->GetWorkerWidget(0)->SetIsSelectable(false);
  x84_tablegroup_rightlog->GetWorkerWidget(x84_tablegroup_rightlog->GetElementCount() - 1)->SetIsSelectable(false);
  zeus::CColor inactiveColor = g_tweakGuiColors->GetPauseItemAmberColor();
  inactiveColor.a() = 0.5f;
  UpdateRightLogColors(false, g_tweakGuiColors->GetPauseItemAmberColor(), inactiveColor);
}

bool CInventoryScreen::ShouldLeftTableAdvance() const { return x19c_samusDoll->IsLoaded(); }

bool CInventoryScreen::ShouldRightTableAdvance() const {
  return std::fabs(x19c_samusDoll->GetViewInterpolation()) == 0.f;
}

u32 CInventoryScreen::GetRightTableCount() const {
  return u32(InventoryRegistry[x70_tablegroup_leftlog->GetUserSelection()].first);
}

bool CInventoryScreen::IsRightLogDynamic() const { return true; }

void CInventoryScreen::UpdateRightLogColors(bool active, const zeus::CColor& activeColor,
                                            const zeus::CColor& inactiveColor) {
  x80_basewidget_rightlog->SetColor(active ? zeus::skWhite : zeus::CColor(1.f, 0.71f));
  const auto& [size, data] = InventoryRegistry[x70_tablegroup_leftlog->GetUserSelection()];
  for (u32 i = 0; i < 5; ++i) {
    if (i < size && IsRightInventoryItemEquipped(data[i].idx)) {
      x15c_model_righttitledecos[i]->SetColor(g_tweakGuiColors->GetPauseItemBlueColor());
      xd8_textpane_titles[i]->TextSupport().SetFontColor(g_tweakGuiColors->GetPauseItemBlueColor());
    } else {
      x15c_model_righttitledecos[i]->SetColor(activeColor);
      xd8_textpane_titles[i]->TextSupport().SetFontColor(activeColor);
    }
  }
}

void CInventoryScreen::UpdateRightLogHighlight(bool active, int idx, const zeus::CColor& activeColor,
                                               const zeus::CColor& inactiveColor) {
  const zeus::CColor actColor = g_tweakGuiColors->GetPauseItemAmberColor() * activeColor;
  const zeus::CColor inactColor = g_tweakGuiColors->GetPauseItemAmberColor() * inactiveColor;

  const auto& [size, data] = InventoryRegistry[x70_tablegroup_leftlog->GetUserSelection()];
  for (s32 i = 0; i < 5; ++i) {
    const bool act = i == idx && active;
    if (i < int(size) && IsRightInventoryItemEquipped(data[i].idx) && act) {
      x8c_model_righthighlight->SetColor(g_tweakGuiColors->GetPauseItemBlueColor());
    } else if (act) {
      x8c_model_righthighlight->SetColor(actColor);
    }
    x144_model_titles[i]->SetColor(act ? actColor : inactColor);
  }
}

void CInventoryScreen::UpdateSamusDollPulses() {
  bool pulseSuit = false;
  bool pulseBeam = false;
  bool pulseGrapple = false;
  bool pulseBoots = false;
  bool pulseVisor = false;
  int userSel = x70_tablegroup_leftlog->GetUserSelection();

  if (x10_mode == EMode::RightTable) {
    if (userSel == 2)
      pulseSuit = true;
    else if (userSel == 0)
      pulseBeam = true;
    else if (userSel == 3)
      pulseVisor = true;
    else if (userSel == 4) {
      pulseGrapple = SecondaryItems[x1c_rightSel].idx == 20;
      pulseBoots = SecondaryItems[x1c_rightSel].idx == 19;
      if (SecondaryItems[x1c_rightSel].idx == 21)
        pulseBeam = true;
    }
  }

  x19c_samusDoll->SetPulseSuit(pulseSuit);
  x19c_samusDoll->SetPulseBeam(pulseBeam);
  x19c_samusDoll->SetPulseGrapple(pulseGrapple);
  x19c_samusDoll->SetPulseBoots(pulseBoots);
  x19c_samusDoll->SetPulseVisor(pulseVisor);
}

} // namespace urde::MP1
