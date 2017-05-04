#include "CInventoryScreen.hpp"

namespace urde
{
namespace MP1
{

CInventoryScreen::CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg,
                                   const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp)
: CPauseScreenBase(mgr, frame, pauseStrg)
{
    CPlayerState& playerState = *mgr.GetPlayerState();
    x19c_samusDoll = std::make_unique<CSamusDoll>(suitDgrp, ballDgrp,
                                                  CPlayerState::EPlayerSuit(int(playerState.GetCurrentSuit()) +
                                                                            playerState.IsFusionEnabled() * 4),
                                                  playerState.GetCurrentBeam(),
                                                  playerState.HasPowerUp(CPlayerState::EItemType::SpiderBall),
                                                  playerState.HasPowerUp(CPlayerState::EItemType::GrappleBeam));
}

bool CInventoryScreen::InputDisabled() const
{
    return false;
}

void CInventoryScreen::TransitioningAway()
{

}

void CInventoryScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue)
{

}

void CInventoryScreen::Touch()
{

}

void CInventoryScreen::ProcessControllerInput(const CFinalInput& input)
{

}

void CInventoryScreen::Draw(float transInterp, float totalAlpha, float yOff)
{

}

float CInventoryScreen::GetCameraYBias() const
{
    return 0.f;
}

bool CInventoryScreen::VReady() const
{
    return false;
}

void CInventoryScreen::VActivate() const
{

}

void CInventoryScreen::ChangedMode()
{

}

void CInventoryScreen::UpdateRightTable()
{

}

u32 CInventoryScreen::GetRightTableCount() const
{
    return 0;
}

bool CInventoryScreen::IsRightLogDynamic() const
{
    return false;
}

void CInventoryScreen::UpdateRightLogColors(bool active, const zeus::CColor& activeColor,
                                            const zeus::CColor& inactiveColor)
{

}

void CInventoryScreen::UpdateRightLogHighlight(bool active, int idx,
                                               const zeus::CColor& activeColor,
                                               const zeus::CColor& inactiveColor)
{

}

}
}
