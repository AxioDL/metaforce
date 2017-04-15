#ifndef __URDE_CINVENTORYSCREEN_HPP__
#define __URDE_CINVENTORYSCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "Editor/ProjectResourceFactoryBase.hpp"

namespace urde
{
class CDependencyGroup;

namespace MP1
{

class CInventoryScreen
{
    u32 x0_w1;
    TLockedToken<CStringTable> x14_strgPauseScreen;
    const CDependencyGroup& x20_suitDgrp;
    const CDependencyGroup& x24_ballDgrp;
    TLockedToken<CGuiFrame> x28_pauseScreenInstructions;
    u32 x34_ = 0;
    u32 x38_ = 0;
    u32 x3c_ = 0;
    u32 x40_ = 0;
    u32 x48_ = 0;
    u32 x4c_ = 0;
    u32 x50_ = 0;
    ResId x54_frmePauseScreenId;
    u32 x58_frmePauseScreenBufSz;
    std::unique_ptr<u8[]> x5c_frmePauseScreenBuf;
    std::shared_ptr<ProjectResourceFactoryBase::AsyncTask> x60_loadTok;
    u32 x64_ = 0;
    u32 x78_ = 0;
    rstl::reserved_vector<CToken, 2> x7c_;

public:
    CInventoryScreen(u32 w1, const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);
    void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input);
};

}
}

#endif // __URDE_CINVENTORYSCREEN_HPP__
