#ifndef __URDE_CPAUSESCREEN_HPP__
#define __URDE_CPAUSESCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "Editor/ProjectResourceFactoryBase.hpp"
#include "CPauseScreenBase.hpp"

namespace urde
{
class CDependencyGroup;

namespace MP1
{

class CPauseScreen
{
public:
    enum class ESubScreen
    {
        LogBook,
        Options,
        Inventory,
        ToGame,
        ToMap
    };

private:
    ESubScreen x0_initialSubScreen;
    u32 x4_ = 2;
    ESubScreen x8_curSubscreen = ESubScreen::ToGame;
    ESubScreen xc_nextSubscreen = ESubScreen::ToGame;
    float x10_alphaInterp = 0.f;
    TLockedToken<CStringTable> x14_strgPauseScreen;
    const CDependencyGroup& x20_suitDgrp;
    const CDependencyGroup& x24_ballDgrp;
    TLockedToken<CGuiFrame> x28_pauseScreenInstructions;
    CGuiFrame* x34_loadedPauseScreenInstructions = nullptr;
    CGuiTextPane* x38_textpane_l1 = nullptr;
    CGuiTextPane* x3c_textpane_r = nullptr;
    CGuiTextPane* x40_textpane_a = nullptr;
    CGuiTextPane* x44_textpane_b = nullptr;
    CGuiTextPane* x48_textpane_return = nullptr;
    CGuiTextPane* x4c_textpane_next = nullptr;
    CGuiTextPane* x50_textpane_back = nullptr;
    CAssetId x54_frmePauseScreenId;
    u32 x58_frmePauseScreenBufSz;
    std::unique_ptr<u8[]> x5c_frmePauseScreenBuf;
    std::shared_ptr<IDvdRequest> x60_loadTok;
    rstl::reserved_vector<std::unique_ptr<CGuiFrame>, 2> x64_frameInsts;
    u32 x78_activeIdx = 0;
    rstl::reserved_vector<std::unique_ptr<CPauseScreenBase>, 2> x7c_screens;
    bool x90_resourcesLoaded = false;
    bool x91_initialTransition = true;

    std::unique_ptr<CPauseScreenBase> BuildPauseSubScreen(ESubScreen subscreen,
                                                          const CStateManager& mgr,
                                                          CGuiFrame& frame) const;
    void StartTransition(float time, const CStateManager& mgr, ESubScreen subscreen, int);
    bool CheckLoadComplete(const CStateManager& mgr);
    void InitializeFrameGlue();
    bool InputEnabled() const;
    static ESubScreen GetPreviousSubscreen(ESubScreen screen);
    static ESubScreen GetNextSubscreen(ESubScreen screen);
    void TransitionComplete();
public:
    CPauseScreen(ESubScreen subscreen, const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);
    ~CPauseScreen();
    void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input);
    void Update(float dt, const CStateManager& mgr, CRandom16& rand, CArchitectureQueue& archQueue);
    void PreDraw();
    void Draw();
    bool IsLoaded() const { return x90_resourcesLoaded; }
    bool ShouldSwitchToMapScreen() const;
    bool ShouldSwitchToInGame() const;
    bool IsTransitioning() const { return x8_curSubscreen != xc_nextSubscreen; }
    float GetHelmetCamYOff() const;
};

}
}

#endif // __URDE_CPAUSESCREEN_HPP__
