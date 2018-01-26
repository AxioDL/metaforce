#ifndef __URDE_CINVENTORYSCREEN_HPP__
#define __URDE_CINVENTORYSCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "CPauseScreenBase.hpp"
#include "CSamusDoll.hpp"

namespace urde
{
class CDependencyGroup;

namespace MP1
{

class CInventoryScreen : public CPauseScreenBase
{
    enum class EState
    {
        Active,
        Leaving,
        Inactive
    };

    std::unique_ptr<CSamusDoll> x19c_samusDoll;
    float x1a0_ = 0.f;
    float x1a4_textBodyAlpha = 0.f;
    EState x1a8_state = EState::Active;
    bool x1ac_textLeaveRequested = false;
    bool x1ad_textViewing;

    void UpdateSamusDollPulses();
    bool HasLeftInventoryItem(int idx) const;
    bool HasRightInventoryItem(int idx) const;
    bool IsRightInventoryItemEquipped(int idx) const;
    void UpdateTextBody();
public:
    CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg,
                     const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);
    ~CInventoryScreen();

    bool InputDisabled() const;
    void TransitioningAway();
    void Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue);
    void Touch();
    void ProcessControllerInput(const CFinalInput& input);
    void Draw(float transInterp, float totalAlpha, float yOff);
    float GetCameraYBias() const;
    bool VReady() const;
    void VActivate();
    void RightTableSelectionChanged(int oldSel, int newSel);
    void ChangedMode(EMode oldMode);
    void UpdateRightTable();
    bool ShouldLeftTableAdvance() const;
    bool ShouldRightTableAdvance() const;
    u32 GetRightTableCount() const;
    bool IsRightLogDynamic() const;
    void UpdateRightLogColors(bool active, const zeus::CColor& activeColor, const zeus::CColor& inactiveColor);
    void UpdateRightLogHighlight(bool active, int idx, const zeus::CColor& activeColor, const zeus::CColor& inactiveColor);
};

}
}

#endif // __URDE_CINVENTORYSCREEN_HPP__
