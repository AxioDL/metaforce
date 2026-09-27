#ifndef _CINVENTORYSCREEN
#define _CINVENTORYSCREEN

#include "MetroidPrime/CPauseScreenBase.hpp"
#include "rstl/single_ptr.hpp"

class CDependencyGroup;
class CSamusDoll;

class CInventoryScreen : public CPauseScreenBase {
public:
  ~CInventoryScreen() override;
  bool InputDisabled() const override;
  void TransitioningAway() override;
  void Update(float dt, CRandom16& rand, CArchitectureQueue& queue) override;
  void Touch() override;
  void ProcessInput(const CFinalInput& input) override;
  void Draw(float transInterp, float totalAlpha, float yOff) const override;
  float GetCameraYBias() const override;
  bool VReady() const override;
  void VActivate() override;
  void RightTableSelectionChanged(int oldSel, int newSel) override;
  void ChangedMode(EMode oldMode) override;
  void UpdateRightTable() override;
  bool ShouldLeftTableAdvance() override;
  bool ShouldRightTableAdvance() override;
  uint GetRightTableCount() const override;
  bool IsRightLogDynamic() const override;
  void UpdateRightLogColors(bool active, const CColor& activeColor,
                            const CColor& inactiveColor) override;
  void UpdateRightLogHighlight(bool active, int idx, const CColor& activeColor,
                               const CColor& inactiveColor) override;

  CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg,
                   const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);

private:
  void UpdateTextBody();
  void UpdateSamusDollPulses();
  bool HasLeftInventoryItem(int idx) const;
  bool HasRightInventoryItem(int idx) const;
  bool IsRightInventoryItemEquipped(int idx) const;

  enum EState { kS_Active, kS_Leaving, kS_Inactive };

  rstl::single_ptr< CSamusDoll > mSamusDoll;
  float x1a0_;
  float mTextBodyAlpha;
  EState mState;
  bool mTextLeaveRequested;
  bool mTextViewing;
};
CHECK_SIZEOF(CInventoryScreen, 0x1b0)

#endif // _CINVENTORYSCREEN
