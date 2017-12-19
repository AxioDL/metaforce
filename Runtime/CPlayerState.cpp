#include "CPlayerState.hpp"
#include "IOStreams.hpp"
#include "zeus/Math.hpp"
#include "CStateManager.hpp"
#include "Camera/CCameraManager.hpp"
#include "Camera/CFirstPersonCamera.hpp"
#include "CMemoryCardSys.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
const u32 CPlayerState::PowerUpMaxValues[41] =
{   1, 1, 1, 1, 250, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 14,  1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

const char* PowerUpNames[41]=
{
    "Power Beam",
    "Ice Beam",
    "Wave Beam",
    "Plasma Beam",
    "Missiles",
    "Scan Visor",
    "Morph Ball Bombs",
    "Power Bombs",
    "Flamethrower",
    "Thermal Visor",
    "Charge Beam",
    "Super Missile",
    "GrappleBeam",
    "X-Ray Visor",
    "Ice Spreader",
    "Space Jump Boots",
    "Morph Ball",
    "Combat Visor",
    "Boost Ball",
    "Spider Ball",
    "Power Suit",
    "Gravity Suit",
    "Varia Suit",
    "Phazon Suit",
    "Energy Tanks",
    "Unknown Item 1",
    "Health Refill",
    "Unknown Item 2",
    "Wavebuster",
    "Artifact of Truth",
    "Artifact of Strength",
    "Artifact of Elder",
    "Artifact of Wild",
    "Artifact of Lifegiver",
    "Artifact of Warrior",
    "Artifact of Chozo",
    "Artifact of Nature",
    "Artifact of Sun",
    "Artifact of World",
    "Artifact of Spirit",
    "Artifact of Newborn",
};

CPlayerState::CPlayerState()
: x188_staticIntf(5)
{
    x0_24_alive = true;
    x24_powerups.set_size(41);
}

CPlayerState::CPlayerState(CBitStreamReader& stream)
: x188_staticIntf(5)
{
    x4_enabledItems = stream.ReadEncoded(0x20);
    u32 tmp = stream.ReadEncoded(0x20);
    xc_health.SetHP(*reinterpret_cast<float*>(&tmp));
    x8_currentBeam = EBeamId(stream.ReadEncoded(CBitStreamReader::GetBitCount(5)));
    x20_currentSuit = EPlayerSuit(stream.ReadEncoded(CBitStreamReader::GetBitCount(4)));
    x24_powerups.set_size(41);
    for (u32 i = 0; i < x24_powerups.size(); ++i)
    {
        if (PowerUpMaxValues[i] == 0)
            continue;

        u32 a = stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxValues[i]));
        u32 b = stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxValues[i]));
        x24_powerups[i] = CPowerUp(a, b);
    }

    const auto& scanStates = g_MemoryCardSys->GetScanStates();
    for (const auto& state : scanStates)
    {
        float time = stream.ReadEncoded(1) ? 1.f : 0.f;
        x170_scanTimes.emplace_back(state.first, time);
    }

    x180_scanCompletionRate.first = stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100));
    x180_scanCompletionRate.second = stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100));
}

void CPlayerState::PutTo(CBitStreamWriter& stream)
{
    stream.WriteEncoded(x4_enabledItems, 32);
    float hp = xc_health.GetHP();
    stream.WriteEncoded(*reinterpret_cast<u32*>(&hp), 32);
    stream.WriteEncoded(u32(x8_currentBeam), CBitStreamWriter::GetBitCount(5));
    stream.WriteEncoded(u32(x20_currentSuit), CBitStreamWriter::GetBitCount(4));
    for (u32 i = 0; i < x24_powerups.size(); ++i)
    {
        const CPowerUp& pup = x24_powerups[i];
        stream.WriteEncoded(pup.x0_amount, CBitStreamWriter::GetBitCount(PowerUpMaxValues[i]));
        stream.WriteEncoded(pup.x4_capacity, CBitStreamWriter::GetBitCount(PowerUpMaxValues[i]));
    }

    for (const auto& scanTime : x170_scanTimes)
    {
        if (scanTime.second >= 1.f)
            stream.WriteEncoded(true, 1);
        else
            stream.WriteEncoded(false, 1);
    }

    stream.WriteEncoded(x180_scanCompletionRate.first, CBitStreamWriter::GetBitCount(0x100));
    stream.WriteEncoded(x180_scanCompletionRate.second, CBitStreamWriter::GetBitCount(0x100));
}

static const u32 costs[] =
{
    5, 10, 10, 10, 1
};

u32 CPlayerState::GetMissileCostForAltAttack() const
{
    return costs[u32(x8_currentBeam)];
}

static const float ComboAmmoPeriods[] =
{
    0.2f, 0.1f, 0.2f, 0.2f, 1.f
};

float CPlayerState::GetComboFireAmmoPeriod() const
{
    return ComboAmmoPeriods[u32(x8_currentBeam)];
}

u32 CPlayerState::CalculateItemCollectionRate() const
{
    u32 total = GetItemCapacity(EItemType::PowerBombs);

    if (total >= 4)
        total -= 3;
    total += GetItemCapacity(EItemType::WaveBeam);
    total += GetItemCapacity(EItemType::IceBeam);
    total += GetItemCapacity(EItemType::PlasmaBeam);
    total += GetItemCapacity(EItemType::Missiles) / 5;
    total += GetItemCapacity(EItemType::MorphBallBombs);
    total += GetItemCapacity(EItemType::Flamethrower);
    total += GetItemCapacity(EItemType::ThermalVisor);
    total += GetItemCapacity(EItemType::ChargeBeam);
    total += GetItemCapacity(EItemType::SuperMissile);
    total += GetItemCapacity(EItemType::GrappleBeam);
    total += GetItemCapacity(EItemType::XRayVisor);
    total += GetItemCapacity(EItemType::IceSpreader);
    total += GetItemCapacity(EItemType::SpaceJumpBoots);
    total += GetItemCapacity(EItemType::MorphBall);
    total += GetItemCapacity(EItemType::BoostBall);
    total += GetItemCapacity(EItemType::SpiderBall);
    total += GetItemCapacity(EItemType::GravitySuit);
    total += GetItemCapacity(EItemType::VariaSuit);
    total += GetItemCapacity(EItemType::EnergyTanks);
    total += GetItemCapacity(EItemType::Truth);
    total += GetItemCapacity(EItemType::Strength);
    total += GetItemCapacity(EItemType::Elder);
    total += GetItemCapacity(EItemType::Wild);
    total += GetItemCapacity(EItemType::Lifegiver);
    total += GetItemCapacity(EItemType::Warrior);
    total += GetItemCapacity(EItemType::Chozo);
    total += GetItemCapacity(EItemType::Nature);
    total += GetItemCapacity(EItemType::Sun);
    total += GetItemCapacity(EItemType::World);
    total += GetItemCapacity(EItemType::Spirit);
    total += GetItemCapacity(EItemType::Newborn);
    return total + GetItemCapacity(EItemType::Wavebuster);
}

CHealthInfo& CPlayerState::HealthInfo()
{
    return xc_health;
}

const CHealthInfo& CPlayerState::GetHealthInfo() const
{
    return xc_health;
}

CPlayerState::EPlayerSuit CPlayerState::GetCurrentSuit() const
{
    if (IsFusionEnabled())
        return EPlayerSuit::FusionPower;

    return x20_currentSuit;
}

bool CPlayerState::CanVisorSeeFog(const CStateManager& stateMgr) const
{
    EPlayerVisor activeVisor = GetActiveVisor(stateMgr);
    if (activeVisor == EPlayerVisor::Combat || activeVisor == EPlayerVisor::Scan)
        return true;
    return true;
}

CPlayerState::EPlayerVisor CPlayerState::GetActiveVisor(const CStateManager& stateMgr) const
{
    const CFirstPersonCamera* cam =
        static_cast<const CFirstPersonCamera*>(
            stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr));
    return (cam ? x14_currentVisor : EPlayerVisor::Combat);
}

void CPlayerState::UpdateStaticInterference(CStateManager& stateMgr, float dt)
{
    x188_staticIntf.Update(stateMgr, dt);
}

void CPlayerState::SetScanTime(CAssetId res, float time)
{
    auto it = std::find_if(x170_scanTimes.begin(), x170_scanTimes.end(), [&](const auto& test) -> bool{
        return test.first == res;
    });

    if (it != x170_scanTimes.end())
        it->second = time;
}

float CPlayerState::GetScanTime(CAssetId res) const
{
    const auto it = std::find_if(x170_scanTimes.cbegin(), x170_scanTimes.cend(), [&](const auto& test) -> bool{
        return test.first == res;
    });

    if (it == x170_scanTimes.end())
        return 0.f;

    return it->second;
}

bool CPlayerState::GetIsVisorTransitioning() const
{
    if (x14_currentVisor != x18_transitioningVisor || x1c_visorTransitionFactor < 0.2f)
        return true;

    return false;
}

float CPlayerState::GetVisorTransitionFactor() const
{
    return x1c_visorTransitionFactor;
}

void CPlayerState::UpdateVisorTransition(float dt)
{
    if (!GetIsVisorTransitioning())
        return;

    if (x14_currentVisor == x18_transitioningVisor)
    {
        x1c_visorTransitionFactor += dt;
        if (x1c_visorTransitionFactor > 0.2)
            x1c_visorTransitionFactor = 0.2;
    }
    else
    {
        x1c_visorTransitionFactor -= dt;
        if (x1c_visorTransitionFactor < 0.f)
        {
            x14_currentVisor = x18_transitioningVisor;
            x1c_visorTransitionFactor = fabs(x1c_visorTransitionFactor);
            if (x1c_visorTransitionFactor < 0.19f)
                x1c_visorTransitionFactor = 0.19f;
        }
    }
}

bool CPlayerState::StartVisorTransition(CPlayerState::EPlayerVisor visor)
{
    x18_transitioningVisor = visor;
    return x14_currentVisor == x18_transitioningVisor;
}

void CPlayerState::ResetVisor()
{
    x18_transitioningVisor = x14_currentVisor = EPlayerVisor::Combat;
    x1c_visorTransitionFactor = 0.0f;
}

bool CPlayerState::ItemEnabled(CPlayerState::EItemType type) const
{
    if (HasPowerUp(type))
        return (x4_enabledItems & (1 << u32(type)));
    return false;
}

void CPlayerState::EnableItem(CPlayerState::EItemType type)
{
    if (HasPowerUp(type))
        x4_enabledItems |= (1 << u32(type));
}

void CPlayerState::DisableItem(CPlayerState::EItemType type)
{
    if (HasPowerUp(type))
        x4_enabledItems &= ~(1 << u32(type));
}

bool CPlayerState::HasPowerUp(CPlayerState::EItemType type) const
{
    if (type < EItemType::Max)
        return x24_powerups[u32(type)].x4_capacity != 0;
    return false;
}

u32 CPlayerState::GetItemCapacity(CPlayerState::EItemType type) const
{
    if (type < EItemType::Max)
        return x24_powerups[u32(type)].x4_capacity;
    return 0;
}

u32 CPlayerState::GetItemAmount(CPlayerState::EItemType type) const
{
    if (type == EItemType::SpaceJumpBoots ||
        type == EItemType::PowerBombs     ||
        type == EItemType::Flamethrower   ||
        type == EItemType::EnergyTanks    ||
        type == EItemType::Missiles       ||
        (type >= EItemType::Truth && type <= EItemType::Newborn))
    {
        return x24_powerups[u32(type)].x0_amount;
    }

    return 0;
}

void CPlayerState::DecrPickup(CPlayerState::EItemType type, s32 amount)
{
    if (type >= EItemType::Max)
        return;

    if ((type == EItemType::Missiles || type >= EItemType::PowerBombs) && type < EItemType::ThermalVisor)
        x24_powerups[u32(type)].x0_amount -= amount;
}

void CPlayerState::IncrPickup(EItemType type, s32 amount)
{
    if (type >= EItemType::Max)
        return;

    if (amount < 0)
        return;

    switch(type)
    {
    case EItemType::Missiles:
    case EItemType::PowerBombs:
    case EItemType::ChargeBeam:
    case EItemType::SpaceJumpBoots:
    case EItemType::EnergyTanks:
    case EItemType::Truth:
    case EItemType::Strength:
    case EItemType::Elder:
    case EItemType::Wild:
    case EItemType::Lifegiver:
    case EItemType::Warrior:
    case EItemType::Chozo:
    case EItemType::Nature:
    case EItemType::Sun:
    case EItemType::World:
    case EItemType::Spirit:
    case EItemType::Newborn:
    {
        CPowerUp& pup = x24_powerups[u32(type)];
        pup.x0_amount = std::min(pup.x0_amount + amount, pup.x4_capacity);
        if (type == EItemType::EnergyTanks)
            IncrPickup(EItemType::HealthRefill, 9999);
        break;
    }
    case EItemType::HealthRefill:
    {
        float health = CalculateHealth(amount);
        xc_health.SetHP(std::min(health, xc_health.GetHP() + amount));
    }
    default:
        break;
    }
}

void CPlayerState::ResetAndIncrPickUp(CPlayerState::EItemType type, s32 amount)
{
    x24_powerups[u32(type)].x0_amount = 0;
    IncrPickup(type, amount);
}

float CPlayerState::CalculateHealth(u32 health)
{
    return (GetBaseHealthCapacity() + (health * GetEnergyTankCapacity()));
}

void CPlayerState::InitializePowerUp(CPlayerState::EItemType type, u32 capacity)
{
    if (type >= EItemType::Max)
        return;

    CPowerUp& pup = x24_powerups[(u32)type];
    pup.x4_capacity = zeus::clamp(u32(0), pup.x4_capacity + capacity, PowerUpMaxValues[u32(type)]);
    pup.x0_amount = std::min(pup.x0_amount, pup.x4_capacity);
    if (type >= EItemType::PowerSuit && type <= EItemType::PhazonSuit)
    {
        if (HasPowerUp(EItemType::PhazonSuit))
            x20_currentSuit = EPlayerSuit::Phazon;
        else if (HasPowerUp(EItemType::GravitySuit))
            x20_currentSuit = EPlayerSuit::Gravity;
        else if (HasPowerUp(EItemType::VariaSuit))
            x20_currentSuit = EPlayerSuit::Varia;
        else
            x20_currentSuit = EPlayerSuit::Power;
    }
}

void CPlayerState::ReInitalizePowerUp(CPlayerState::EItemType type, u32 capacity)
{
    x24_powerups[u32(type)].x4_capacity = 0;
    InitializePowerUp(type, capacity);
}

void CPlayerState::InitializeScanTimes()
{
    if (x170_scanTimes.size())
        return;

    const auto& scanStates = g_MemoryCardSys->GetScanStates();
    for (const auto& state : scanStates)
        x170_scanTimes.emplace_back(state.first, 0.f);
}

}
