#pragma once

#include <memory>

#include "Runtime/MP1/CInGameGuiManager.hpp"
#include "Runtime/MP1/CPauseScreenBase.hpp"
#include "Runtime/MP1/CSamusDoll.hpp"

#include <zeus/CVector2f.hpp>

namespace urde {
class CDependencyGroup;

namespace MP1 {

class CInventoryScreen : public CPauseScreenBase {
  enum class EState { Active, Leaving, Inactive };

  std::unique_ptr<CSamusDoll> x19c_samusDoll;
  float x1a0_ = 0.f;
  float x1a4_textBodyAlpha = 0.f;
  EState x1a8_state = EState::Active;
  bool x1ac_textLeaveRequested = false;
  bool x1ad_textViewing;

  zeus::CVector2f m_lastMouseCoord;
  boo::SScrollDelta m_lastAccumScroll;
  boo::SScrollDelta m_dollScroll;

  void UpdateSamusDollPulses();
  bool HasLeftInventoryItem(int idx) const;
  bool HasRightInventoryItem(int idx) const;
  bool IsRightInventoryItemEquipped(int idx) const;
  void UpdateTextBody();

public:
  CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg,
                   const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);
  ~CInventoryScreen() override;

  bool InputDisabled() const override;
  void TransitioningAway() override;
  void Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue) override;
  void Touch() override;
  void ProcessControllerInput(const CFinalInput& input) override;
  void Draw(float transInterp, float totalAlpha, float yOff) override;
  float GetCameraYBias() const override;
  bool VReady() const override;
  void VActivate() override;
  void RightTableSelectionChanged(int oldSel, int newSel) override;
  void ChangedMode(EMode oldMode) override;
  void UpdateRightTable() override;
  bool ShouldLeftTableAdvance() const override;
  bool ShouldRightTableAdvance() const override;
  u32 GetRightTableCount() const override;
  bool IsRightLogDynamic() const override;
  void UpdateRightLogColors(bool active, const zeus::CColor& activeColor, const zeus::CColor& inactiveColor) override;
  void UpdateRightLogHighlight(bool active, int idx, const zeus::CColor& activeColor,
                               const zeus::CColor& inactiveColor) override;
};

} // namespace MP1
} // namespace urde
