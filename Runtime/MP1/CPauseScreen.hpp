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
        Invalid
    };

private:
    u32 x0_w1;
    u32 x4_ = 2;
    ESubScreen x8_ = ESubScreen::Invalid;
    ESubScreen xc_ = ESubScreen::Invalid;
    float x10_ = 0.f;
    TLockedToken<CStringTable> x14_strgPauseScreen;
    const CDependencyGroup& x20_suitDgrp;
    const CDependencyGroup& x24_ballDgrp;
    TLockedToken<CGuiFrame> x28_pauseScreenInstructions;
    CGuiFrame* x34_loadedPauseScreenInstructions = nullptr;
    CGuiTextPane* x38_textpane_l1 = nullptr;
    CGuiTextPane* x3c_textpane_r = nullptr;
    CGuiTextPane* x40_textpane_a = nullptr;
    CGuiTextPane* x48_textpane_return = nullptr;
    CGuiTextPane* x4c_textpane_next = nullptr;
    CGuiTextPane* x50_textpane_back = nullptr;
    ResId x54_frmePauseScreenId;
    u32 x58_frmePauseScreenBufSz;
    std::unique_ptr<u8[]> x5c_frmePauseScreenBuf;
    std::shared_ptr<ProjectResourceFactoryBase::AsyncTask> x60_loadTok;
    rstl::reserved_vector<std::unique_ptr<CGuiFrame>, 2> x64_frames;
    u32 x78_activeIdx = 0;
    rstl::reserved_vector<std::unique_ptr<CPauseScreenBase>, 2> x7c_screens;
    bool x90_resourcesLoaded = false;
    bool x91_inPauseScreen = true;

public:
    CPauseScreen(u32 w1, const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);
    void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input);
    void Update(float dt, CRandom16& rand, const CStateManager& mgr);
    void PreDraw();
};

}
}

#endif // __URDE_CPAUSESCREEN_HPP__
