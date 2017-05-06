#include "CInventoryScreen.hpp"
#include "GuiSys/CGuiTableGroup.hpp"

namespace urde
{
namespace MP1
{

struct SInventoryItem
{
    u32 idx;
    u32 nameStrIdx;
    u32 entryStrIdx;
};

static const SInventoryItem ArmCannonItems[] =
{
    {0, 0x24, 0x46}, // Power Beam
    {1, 0x25, 0x48}, // Ice Beam
    {2, 0x26, 0x4a}, // Wave Beam
    {3, 0x27, 0x4c}, // Plasma Beam
    {4, 0x28, 0x4e}, // Phazon Beam
};

static const SInventoryItem MorphballItems[] =
{
    {5, 0x2e, 0x57}, // Morph Ball
    {6, 0x2f, 0x58}, // Boost Ball
    {7, 0x30, 0x59}, // Spider Ball
    {8, 0x31, 0x5a}, // Morph Ball Bomb
    {9, 0x32, 0x5b}, // Power Bomb
};

static const SInventoryItem SuitItems[] =
{
    {10, 0x33, 0x52}, // Power Suit
    {11, 0x34, 0x53}, // Varia Suit
    {12, 0x35, 0x54}, // Gravity Suit
    {13, 0x36, 0x55}, // Phazon Suit
    {14, 0x37, 0x56}, // Energy Tank
};

static const SInventoryItem VisorItems[] =
{
    {15, 0x38, 0x42}, // Combat Visor
    {16, 0x39, 0x43}, // Scan Visor
    {17, 0x3a, 0x44}, // X-Ray Visor
    {18, 0x3b, 0x45}, // Thermal Visor
};

static const SInventoryItem SecondaryItems[] =
{
    {19, 0x3c, 0x4f}, // Space Jump Boots
    {20, 0x3d, 0x50}, // Grapple Beam
    {21, 0x3e, 0x51}, // Missile Launcher
    {22, 0x3f, 0x5c}, // Charge Beam
    {23, 0x40, 0x5d}, // Beam Combo
};

static const std::pair<u32, const SInventoryItem*> InventoryRegistry[] =
{
    {5, ArmCannonItems},
    {5, MorphballItems},
    {5, SuitItems},
    {4, VisorItems},
    {5, SecondaryItems},
};

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

void CInventoryScreen::UpdateSamusDollPulses()
{
    bool pulseSuit = false;
    bool pulseBeam = false;
    bool pulseGrapple = false;
    bool pulseBoots = false;
    bool pulseVisor = false;
    int userSel = x70_tablegroup_leftlog->GetUserSelection();

    if (x10_mode == EMode::RightTable)
    {
        if (userSel == 2)
            pulseSuit = true;
        else if (userSel == 0)
            pulseBeam = true;
        else if (userSel == 3)
            pulseVisor = true;
        else if (userSel == 4)
        {
            pulseGrapple = SecondaryItems[x1c_rightSel].idx == 20;
            pulseBoots = SecondaryItems[x1c_rightSel].idx == 19;
            if (SecondaryItems[x1c_rightSel].idx == 21)
                pulseBeam = true;
        }
    }

    x19c_samusDoll->SetPulseSuit(pulseSuit);
    x19c_samusDoll->SetPulseBeam(pulseBeam);
    x19c_samusDoll->SetPulseGrapple(pulseGrapple);
    x19c_samusDoll->SetPulseBoots(pulseBoots);
    x19c_samusDoll->SetPulseVisor(pulseVisor);
}

}
}
