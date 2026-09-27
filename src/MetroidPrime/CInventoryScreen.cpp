#include "MetroidPrime/CInventoryScreen.hpp"

#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CSamusDoll.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/math.hpp"
#include <limits.h>

namespace {
struct SInventoryItem {
  int idx;
  int nameStrIdx;
  int entryStrIdx;
};

struct SInventoryCategory {
  int count;
  const SInventoryItem* items;
};

const SInventoryItem skArmCannonItems[] = {
    {0, 0x24, 0x46}, {1, 0x25, 0x48}, {2, 0x26, 0x4a}, {3, 0x27, 0x4c}, {4, 0x28, 0x4e}};

const SInventoryItem skMorphballItems[] = {
    {5, 0x2e, 0x57}, {6, 0x2f, 0x58}, {7, 0x30, 0x59}, {8, 0x31, 0x5a}, {9, 0x32, 0x5b}};

const SInventoryItem skSuitItems[] = {
    {10, 0x33, 0x52}, {11, 0x34, 0x53}, {12, 0x35, 0x54}, {13, 0x36, 0x55}, {14, 0x37, 0x56}};

const SInventoryItem skVisorItems[] = {
    {15, 0x38, 0x42}, {16, 0x39, 0x43}, {17, 0x3a, 0x44}, {18, 0x3b, 0x45}};

const SInventoryItem skSecondaryItems[] = {
    {19, 0x3c, 0x4f}, {20, 0x3d, 0x50}, {21, 0x3e, 0x51}, {22, 0x3f, 0x5c}, {23, 0x40, 0x5d}};

const SInventoryCategory skInventoryRegistry[] = {{5, skArmCannonItems},
                                                  {5, skMorphballItems},
                                                  {5, skSuitItems},
                                                  {4, skVisorItems},
                                                  {5, skSecondaryItems}};

const wchar_t* const skUnknownItem = L"??????";
} // namespace

CInventoryScreen::CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame,
                                   const CStringTable& pauseStrg, const CDependencyGroup& suitDgrp,
                                   const CDependencyGroup& ballDgrp)
: CPauseScreenBase(mgr, frame, pauseStrg)
, mSamusDoll(rs_new CSamusDoll(
      suitDgrp, ballDgrp,
      static_cast< CPlayerState::EPlayerSuit >(mgr.GetPlayerState()->GetCurrentSuitRaw() +
                                               4 * mgr.GetPlayerState()->GetIsFusionEnabled()),
      mgr.GetPlayerState()->GetCurrentBeam(),
      mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall),
      mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GrappleBeam)))
, x1a0_(0.f)
, mTextBodyAlpha(0.f)
, mState(kS_Active)
, mTextLeaveRequested(false) {}

CInventoryScreen::~CInventoryScreen() {
  for (int i = 0; i < 5; ++i) {
    mTextpane_titles[i]->TextSupport().SetFontColor(CColor::White());
    mModel_righttitledecos[i]->SetColor(CColor::White());
    mModel_titles[i]->SetColor(CColor::White());
  }

  mModel_righthighlight->SetColor(CColor::White());
}

float CInventoryScreen::GetCameraYBias() const {
  return CMath::AbsF(mSamusDoll->GetViewInterpolation());
}

void CInventoryScreen::ProcessInput(const CFinalInput& input) {
  if (mState == kS_Inactive) {
    return;
  }

  if (mSamusDoll->IsViewInterpolating()) {
    return;
  }

  float viewInterp = CMath::AbsF(mSamusDoll->GetViewInterpolation());
  if (input.PY() && mSamusDoll->IsLoaded() && (viewInterp > 0.f || mMode != kM_TextScroll)) {
    mSamusDoll->BeginViewInterpolate(viewInterp == 0.f);
  }

  if (viewInterp == 1.f) {
    if (input.PStart()) {
      mSamusDoll->BeginViewInterpolate(false);
      mExitPauseScreen = true;
    } else if (input.PB()) {
      mSamusDoll->BeginViewInterpolate(false);
    }
  }

  if (CMath::AbsF(mSamusDoll->GetViewInterpolation()) > 0.f) {
    float motionAmt = 6.f * input.Time();
    float circleUp = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleUp, input);
    float circleDown = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleDown, input);
    float circleLeft = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleLeft, input);
    float circleRight = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleRight, input);
    float moveForward = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveForward, input);
    float moveBack = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveBack, input);
    float moveLeft = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveLeft, input);
    float moveRight = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveRight, input);
    float zoomIn = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomIn, input);
    float zoomOut = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomOut, input);

    CVector3f moveVec(0.25f * motionAmt * (moveRight - moveLeft),
                      0.5f * motionAmt * (zoomIn - zoomOut),
                      0.25f * motionAmt * (moveForward - moveBack));
    mSamusDoll->SetOffset(moveVec, input.Time());
    mSamusDoll->SetRotation(0.5f * motionAmt * (circleDown - circleUp),
                                0.5f * motionAmt * (circleRight - circleLeft), input.Time());
  } else {
    mTextViewing = false;

    if (mMode == kM_TextScroll) {
      int oldPage = mTextpane_body->TextSupport().GetPageCounter();
      int newPage = oldPage;
      int totalCount = mTextpane_body->TextSupport().GetTotalPageCount();
      bool lastPage = oldPage == totalCount - 1;

      if (totalCount != -1) {
        if (input.PLAUp()) {
          newPage = rstl::max_val(0, oldPage - 1);
        } else if (input.PLADown() || (input.PA() && !lastPage)) {
          newPage = rstl::min_val(totalCount - 1, oldPage + 1);
        }

        mTextpane_body->TextSupport().SetPage(newPage);
        if (oldPage != newPage) {
          CSfxManager::SfxStart(0x5a4, 0x7f, 0x40, false);
        }

        mPulseTextArrowTop = newPage > 0;
        mPulseTextArrowBottom = !lastPage;
      } else {
        mPulseTextArrowTop = mPulseTextArrowBottom = false;
      }

      if (!mTextLeaveRequested) {
        mTextLeaveRequested = input.PB() || ((input.PA() && lastPage) ? true : false);
      }
      mTextViewing = !mTextLeaveRequested;
    } else {
      mPulseTextArrowTop = mPulseTextArrowBottom = false;
    }

    if (mState != kS_Active) {
      mTextViewing = false;
    }

    CPauseScreenBase::ProcessInput(input);
  }
}

void CInventoryScreen::ChangedMode(EMode oldMode) {
  if (mMode == kM_TextScroll) {
    mTextViewing = true;
    UpdateTextBody();
  }
}

void CInventoryScreen::UpdateTextBody() {
  mTextLeaveRequested = false;

  int leftSel = mTablegroup_leftlog->GetUserSelection();
  const SInventoryCategory& category = skInventoryRegistry[leftSel];
  const SInventoryItem& item = category.items[mRightSel];

  rstl::wstring text = mPauseStrg.GetString(item.entryStrIdx);
  if (item.idx == 23) {
    const CPlayerState& playerState = *mMgr.GetPlayerState();
    text.append(
        mPauseStrg.GetString(playerState.HasPowerUp(CPlayerState::kIT_SuperMissile) ? 71 : 65),
        -1);
    text.append(
        mPauseStrg.GetString(playerState.HasPowerUp(CPlayerState::kIT_IceSpreader) ? 73 : 65),
        -1);
    text.append(
        mPauseStrg.GetString(playerState.HasPowerUp(CPlayerState::kIT_Wavebuster) ? 75 : 65), -1);
    text.append(
        mPauseStrg.GetString(playerState.HasPowerUp(CPlayerState::kIT_Flamethrower) ? 77 : 65),
        -1);
  }

  mTextpane_body->TextSupport().SetText(text, true);
  mTextpane_body->TextSupport().SetPage(0);
}

bool CInventoryScreen::VReady() const { return true; }

void CInventoryScreen::VActivate() {
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* category = mTextpane_categories[i];
    if (HasLeftInventoryItem(i)) {
      category->TextSupport().SetText(mPauseStrg.GetString(i + 10));
    } else {
      category->TextSupport().SetText(rstl::wstring_l(skUnknownItem));
      mTablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
    }
  }

  mTextpane_title->TextSupport().SetText(mPauseStrg.GetString(9));
  mBasewidget_yicon->SetVisibility(true, kTM_Children);

  for (int i = 5; i < 5; ++i) {
    mTablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
  }
}

bool CInventoryScreen::ShouldLeftTableAdvance() { return mSamusDoll->IsLoaded(); }

bool CInventoryScreen::ShouldRightTableAdvance() {
  return CMath::AbsF(mSamusDoll->GetViewInterpolation()) == 0.f;
}

uint CInventoryScreen::GetRightTableCount() const {
  return skInventoryRegistry[mTablegroup_leftlog->GetUserSelection()].count;
}

void CInventoryScreen::Draw(float transInterp, float totalAlpha, float yOff) const {
  CPauseScreenBase::Draw(transInterp, totalAlpha,
                         CMath::AbsF(mSamusDoll->GetViewInterpolation()));
  mSamusDoll->Draw(mMgr, transInterp * (1.f - mTextBodyAlpha));
}

void CInventoryScreen::Touch() {
  CPauseScreenBase::Touch();
  mSamusDoll->Touch();
}

void CInventoryScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& queue) {
  CPauseScreenBase::Update(dt, rand, queue);
  mSamusDoll->Update(dt, rand);

  if (mMode == kM_TextScroll) {
    if (mTextViewing) {
      mTextBodyAlpha = rstl::min_val(1.f, 4.f * dt + mTextBodyAlpha);
    } else {
      mTextBodyAlpha = rstl::max_val(0.f, mTextBodyAlpha - 4.f * dt);
    }

    CColor bodyColor = CColor::White().WithAlphaOf(mTextBodyAlpha);
    mTextpane_body->SetColor(bodyColor);
    mBasewidget_yicon->SetColor(CColor::White().WithAlphaOf(1.f - mTextBodyAlpha));

    if (mTextBodyAlpha == 0.f && mState == kS_Active) {
      ChangeMode(kM_RightTable);
    }
  }

  bool morphball = mTablegroup_leftlog->GetUserSelection() == 1 && mMode != kM_LeftTable;
  mSamusDoll->CheckTransition(morphball);
  UpdateSamusDollPulses();

  if (mState == kS_Leaving && mTextBodyAlpha == 0.f) {
    mState = kS_Inactive;
  }
}

void CInventoryScreen::UpdateRightTable() {
  CPauseScreenBase::UpdateRightTable();

  int minSel = INT_MAX;
  int leftSel = mTablegroup_leftlog->GetUserSelection();
  const SInventoryCategory& category = skInventoryRegistry[leftSel];

  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* title = mTextpane_titles[i];
    if (i < category.count) {
      const SInventoryItem& item = category.items[i];
      if (HasRightInventoryItem(item.idx)) {
        title->TextSupport().SetText(mPauseStrg.GetString(item.nameStrIdx));
        mTablegroup_rightlog->GetWorkerWidget(i + 1)->SetIsSelectable(true);
        if (i < minSel) {
          minSel = i;
        }
      } else {
        title->TextSupport().SetText(rstl::wstring_l(skUnknownItem));
        mTablegroup_rightlog->GetWorkerWidget(i + 1)->SetIsSelectable(false);
      }
    } else {
      title->TextSupport().SetText(rstl::wstring_l(L""));
    }
  }

  if (minSel != INT_MAX) {
    mRightSel = minSel;
    SetRightTableSelection(mRightSel, mRightSel);
  }

  mTablegroup_rightlog->GetWorkerWidget(0)->SetIsSelectable(false);
  mTablegroup_rightlog->GetWorkerWidget(mTablegroup_rightlog->GetElementCount() - 1)
      ->SetIsSelectable(false);

  UpdateRightLogColors(false, gpTweakGuiColors->GetPauseItemAmberColor(),
                       gpTweakGuiColors->GetPauseItemAmberColor().WithAlphaOf(0.5f));
}

void CInventoryScreen::UpdateRightLogColors(bool active, const CColor& activeColor,
                                            const CColor& inactiveColor) {
  mBasewidget_rightlog->SetColor(active ? CColor::White()
                                           : CColor::White().WithAlphaOf(0.712291f));

  int leftSel = mTablegroup_leftlog->GetUserSelection();
  const SInventoryCategory& category = skInventoryRegistry[leftSel];

  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* title = mTextpane_titles[i];
    bool useActiveColor = true;
    if (i < category.count && IsRightInventoryItemEquipped(category.items[i].idx)) {
      mModel_righttitledecos[i]->SetColor(gpTweakGuiColors->GetPauseItemBlueColor());
      title->TextSupport().SetFontColor(gpTweakGuiColors->GetPauseItemBlueColor());
      useActiveColor = false;
    }

    if (useActiveColor) {
      mModel_righttitledecos[i]->SetColor(activeColor);
      title->TextSupport().SetFontColor(activeColor);
    }
  }
}

void CInventoryScreen::UpdateRightLogHighlight(bool active, int idx, const CColor& activeColor,
                                               const CColor& inactiveColor) {
  CColor activeBlue = CColor::Modulate(gpTweakGuiColors->GetPauseItemBlueColor(), activeColor);
  CColor inactiveBlue = CColor::Modulate(gpTweakGuiColors->GetPauseItemBlueColor(), inactiveColor);
  CColor activeAmber = CColor::Modulate(gpTweakGuiColors->GetPauseItemAmberColor(), activeColor);
  CColor inactiveAmber =
      CColor::Modulate(gpTweakGuiColors->GetPauseItemAmberColor(), inactiveColor);

  int leftSel = mTablegroup_leftlog->GetUserSelection();
  const SInventoryCategory& category = skInventoryRegistry[leftSel];

  for (int i = 0; i < 5; ++i) {
    bool selected = idx == i && active;
    bool useAmber = true;
    if (i < category.count && IsRightInventoryItemEquipped(category.items[i].idx) && selected) {
      useAmber = false;
      mModel_righthighlight->SetColor(gpTweakGuiColors->GetPauseItemBlueColor());
    }

    mModel_titles[i]->SetColor(selected ? activeAmber : inactiveAmber);
    if (useAmber && selected) {
      mModel_righthighlight->SetColor(activeAmber);
    }
  }
}

void CInventoryScreen::TransitioningAway() { mState = kS_Leaving; }

bool CInventoryScreen::InputDisabled() const {
  if (CMath::AbsF(mSamusDoll->GetViewInterpolation()) > 0.f) {
    return true;
  }

  return mState == kS_Leaving;
}

void CInventoryScreen::RightTableSelectionChanged(int oldSel, int newSel) {}

void CInventoryScreen::UpdateSamusDollPulses() {
  bool pulseSuit = false;
  bool pulseBeam = false;
  bool pulseGrapple = false;
  bool pulseBoots = false;
  bool pulseVisor = false;
  int userSel = mTablegroup_leftlog->GetUserSelection();

  if (mMode == kM_RightTable) {
    if (userSel == 2) {
      pulseSuit = true;
    } else if (userSel == 0) {
      pulseBeam = true;
    } else if (userSel == 3) {
      pulseVisor = true;
    } else if (userSel == 4) {
      pulseGrapple = skSecondaryItems[mRightSel].idx == 20;
      pulseBoots = skSecondaryItems[mRightSel].idx == 19;
      if (skSecondaryItems[mRightSel].idx == 21) {
        pulseBeam = true;
      }
    }
  }

  mSamusDoll->SetPulseSuit(pulseSuit);
  mSamusDoll->SetPulseBeam(pulseBeam);
  mSamusDoll->SetPulseGrapple(pulseGrapple);
  mSamusDoll->SetPulseBoots(pulseBoots);
  mSamusDoll->SetPulseVisor(pulseVisor);
}

bool CInventoryScreen::HasLeftInventoryItem(int idx) const {
  const CPlayerState& playerState = *mMgr.GetPlayerState();

  switch (idx) {
  case 0:
    return true;
  case 1:
    return playerState.HasPowerUp(CPlayerState::kIT_MorphBall);
  case 2:
    return true;
  case 3:
    return true;
  case 4:
    return playerState.HasPowerUp(CPlayerState::kIT_SpaceJumpBoots) ||
           playerState.HasPowerUp(CPlayerState::kIT_GrappleBeam) ||
           playerState.HasPowerUp(CPlayerState::kIT_Missiles) ||
           playerState.HasPowerUp(CPlayerState::kIT_ChargeBeam) ||
           playerState.HasPowerUp(CPlayerState::kIT_SuperMissile) ||
           playerState.HasPowerUp(CPlayerState::kIT_IceSpreader) ||
           playerState.HasPowerUp(CPlayerState::kIT_Wavebuster) ||
           playerState.HasPowerUp(CPlayerState::kIT_Flamethrower);
  default:
    return false;
  }
}

bool CInventoryScreen::HasRightInventoryItem(int idx) const {
  const CPlayerState& playerState = *mMgr.GetPlayerState();

  switch (idx) {
  case 0:
    return true;
  case 1:
    return playerState.HasPowerUp(CPlayerState::kIT_IceBeam);
  case 2:
    return playerState.HasPowerUp(CPlayerState::kIT_WaveBeam);
  case 3:
    return playerState.HasPowerUp(CPlayerState::kIT_PlasmaBeam);
  case 4:
    return playerState.HasPowerUp(CPlayerState::kIT_PhazonSuit);
  case 5:
    return playerState.HasPowerUp(CPlayerState::kIT_MorphBall);
  case 6:
    return playerState.HasPowerUp(CPlayerState::kIT_BoostBall);
  case 7:
    return playerState.HasPowerUp(CPlayerState::kIT_SpiderBall);
  case 8:
    return playerState.HasPowerUp(CPlayerState::kIT_MorphBallBombs);
  case 9:
    return playerState.HasPowerUp(CPlayerState::kIT_PowerBombs);
  case 10:
    return true;
  case 11:
    return playerState.HasPowerUp(CPlayerState::kIT_VariaSuit);
  case 12:
    return playerState.HasPowerUp(CPlayerState::kIT_GravitySuit);
  case 13:
    return playerState.HasPowerUp(CPlayerState::kIT_PhazonSuit);
  case 14:
    return playerState.HasPowerUp(CPlayerState::kIT_EnergyTanks);
  case 15:
    return true;
  case 16:
    return playerState.HasPowerUp(CPlayerState::kIT_ScanVisor);
  case 17:
    return playerState.HasPowerUp(CPlayerState::kIT_XRayVisor);
  case 18:
    return playerState.HasPowerUp(CPlayerState::kIT_ThermalVisor);
  case 19:
    return playerState.HasPowerUp(CPlayerState::kIT_SpaceJumpBoots);
  case 20:
    return playerState.HasPowerUp(CPlayerState::kIT_GrappleBeam);
  case 21:
    return playerState.HasPowerUp(CPlayerState::kIT_Missiles);
  case 22:
    return playerState.HasPowerUp(CPlayerState::kIT_ChargeBeam);
  case 23:
    return playerState.HasPowerUp(CPlayerState::kIT_SuperMissile) ||
           playerState.HasPowerUp(CPlayerState::kIT_IceSpreader) ||
           playerState.HasPowerUp(CPlayerState::kIT_Wavebuster) ||
           playerState.HasPowerUp(CPlayerState::kIT_Flamethrower);
  default:
    return false;
  }
}

bool CInventoryScreen::IsRightInventoryItemEquipped(int idx) const {
  const CPlayerState& playerState = *mMgr.GetPlayerState();

  switch (idx) {
  case 0:
    return playerState.GetCurrentBeam() == CPlayerState::kBI_Power;
  case 1:
    return playerState.GetCurrentBeam() == CPlayerState::kBI_Ice;
  case 2:
    return playerState.GetCurrentBeam() == CPlayerState::kBI_Wave;
  case 3:
    return playerState.GetCurrentBeam() == CPlayerState::kBI_Plasma;
  case 4:
    return playerState.GetCurrentBeam() == CPlayerState::kBI_Phazon2;
  case 5:
    return playerState.HasPowerUp(CPlayerState::kIT_MorphBall);
  case 6:
    return playerState.HasPowerUp(CPlayerState::kIT_BoostBall);
  case 7:
    return playerState.HasPowerUp(CPlayerState::kIT_SpiderBall);
  case 8:
    return playerState.HasPowerUp(CPlayerState::kIT_MorphBallBombs);
  case 9:
    return playerState.HasPowerUp(CPlayerState::kIT_PowerBombs);
  case 10:
    return playerState.GetCurrentSuitRaw() == CPlayerState::kPS_Power;
  case 11:
    return playerState.GetCurrentSuitRaw() == CPlayerState::kPS_Varia;
  case 12:
    return playerState.GetCurrentSuitRaw() == CPlayerState::kPS_Gravity;
  case 13:
    return playerState.GetCurrentSuitRaw() == CPlayerState::kPS_Phazon;
  case 14:
    return playerState.HasPowerUp(CPlayerState::kIT_EnergyTanks);
  case 15:
    return playerState.GetCurrentVisor() == CPlayerState::kPV_Combat;
  case 16:
    return playerState.GetCurrentVisor() == CPlayerState::kPV_Scan;
  case 17:
    return playerState.GetCurrentVisor() == CPlayerState::kPV_XRay;
  case 18:
    return playerState.GetCurrentVisor() == CPlayerState::kPV_Thermal;
  case 19:
    return playerState.HasPowerUp(CPlayerState::kIT_SpaceJumpBoots);
  case 20:
    return playerState.HasPowerUp(CPlayerState::kIT_GrappleBeam);
  case 21:
    return playerState.HasPowerUp(CPlayerState::kIT_Missiles);
  case 22:
    return playerState.HasPowerUp(CPlayerState::kIT_ChargeBeam);
  case 23:
    return playerState.HasPowerUp(CPlayerState::kIT_SuperMissile) ||
           playerState.HasPowerUp(CPlayerState::kIT_IceSpreader) ||
           playerState.HasPowerUp(CPlayerState::kIT_Wavebuster) ||
           playerState.HasPowerUp(CPlayerState::kIT_Flamethrower);
  default:
    return false;
  }
}

bool CInventoryScreen::IsRightLogDynamic() const { return true; }
