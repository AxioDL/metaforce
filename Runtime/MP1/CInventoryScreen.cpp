#include "CInventoryScreen.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde
{
namespace MP1
{

CInventoryScreen::CInventoryScreen(u32 w1,
                                   const CDependencyGroup& suitDgrp,
                                   const CDependencyGroup& ballDgrp)
: x0_w1(w1), x14_strgPauseScreen(g_SimplePool->GetObj("STRG_PauseScreen")),
  x20_suitDgrp(suitDgrp), x24_ballDgrp(ballDgrp),
  x28_pauseScreenInstructions(g_SimplePool->GetObj("FRME_PauseScreenInstructions")),
  x54_frmePauseScreenId(g_ResFactory->GetResourceIdByName("FRME_PauseScreen")->id)
{
    SObjectTag frmeTag(FOURCC('FRME'), x54_frmePauseScreenId);
    x58_frmePauseScreenBufSz = g_ResFactory->ResourceSize(frmeTag);
    ProjectResourceFactoryBase& resFac = static_cast<ProjectResourceFactoryBase&>(*g_ResFactory);
    x60_loadTok = resFac.LoadResourceAsync(frmeTag, x5c_frmePauseScreenBuf);
    CSfxManager::SfxStart(1435, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

}
}
