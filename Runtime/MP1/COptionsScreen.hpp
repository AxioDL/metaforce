#ifndef __URDE_COPTIONSSCREEN_HPP__
#define __URDE_COPTIONSSCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "CPauseScreenBase.hpp"
#include "CGameCubeDoll.hpp"
#include "CQuitGameScreen.hpp"

namespace urde
{
namespace MP1
{

class COptionsScreen : public CPauseScreenBase
{
    std::unique_ptr<CQuitGameScreen> x19c_quitGame;
    std::unique_ptr<CGameCubeDoll> x1a0_gameCube;
    CSfxHandle x1a4_sliderSfx;
    CRumbleGenerator x1a8_rumble;
    float x29c_optionAlpha = 0.f;
    bool x2a0_24_inOptionBody : 1;

    void UpdateOptionView();
    void ResetOptionWidgetVisibility();
    void OnSliderChanged(CGuiSliderGroup* caller, float val);
    void OnEnumChanged(CGuiTableGroup* caller, int sel);

public:
    COptionsScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);
    ~COptionsScreen();

    bool InputDisabled() const;
    void Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue);
    void Touch();
    void ProcessControllerInput(const CFinalInput& input);
    void Draw(float transInterp, float totalAlpha, float yOff);
    bool VReady() const;
    void VActivate();
    void RightTableSelectionChanged(int selBegin, int selEnd);
    void ChangedMode();
    void UpdateRightTable();
    bool ShouldLeftTableAdvance() const;
    bool ShouldRightTableAdvance() const;
    u32 GetRightTableCount() const;
};

}
}

#endif // __URDE_COPTIONSSCREEN_HPP__
