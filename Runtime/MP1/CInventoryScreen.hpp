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
    std::unique_ptr<CSamusDoll> x19c_samusDoll;
    float x1a0_ = 0.f;
    float x1a4_ = 0.f;
    u32 x1a8_ = 0;
    bool x1ac_ = false;
public:
    CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg,
                     const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);

    bool InputDisabled() const;
    void TransitioningAway();
    void Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue);
    void Touch();
    void ProcessControllerInput(const CFinalInput& input);
    void Draw(float transInterp, float totalAlpha, float yOff);
    float GetCameraYBias() const;
    bool VReady() const;
    void VActivate() const;
    void ChangedMode();
    void UpdateRightTable();
    u32 GetRightTableCount() const;
    bool IsRightLogDynamic() const;
    void UpdateRightLogColors(bool active, const zeus::CColor& activeColor, const zeus::CColor& inactiveColor);
    void UpdateRightLogHighlight(bool active, int idx, const zeus::CColor& activeColor, const zeus::CColor& inactiveColor);
    void UpdateSamusDollPulses();
};

}
}

#endif // __URDE_CINVENTORYSCREEN_HPP__
