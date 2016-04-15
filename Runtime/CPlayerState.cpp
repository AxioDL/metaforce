#include "CPlayerState.hpp"
#include "IOStreams.hpp"
#include "zeus/Math.hpp"

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

CPlayerState::CPlayerState(CBitStreamReader& stream)
    : CPlayerState()
{
    x4_ = stream.ReadEncoded(0x20);
    u32 tmp = stream.ReadEncoded(0x20);
    xc_currentHealth = *reinterpret_cast<float*>(&tmp);
    x8_currentBeam = EBeamId(stream.ReadEncoded(CBitStreamReader::GetBitCount(5)));
    x20_currentSuit = EPlayerSuit(stream.ReadEncoded(CBitStreamReader::GetBitCount(4)));
    x24_powerups.resize(41);
    for (u32 i = 0; i < x24_powerups.size(); ++i)
    {
        if (PowerUpMaxValues[i] == 0)
            continue;

        u32 a = stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxValues[i]));
        u32 b = stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxValues[i]));
        x24_powerups[i] = CPowerUp(a, b);
    }

    x170_scanTimes.resize(846);
    for (u32 i = 0; i < x170_scanTimes.size(); i++)
    {
        x170_scanTimes[i].first = stream.ReadEncoded(1);
        if (x170_scanTimes[i].first)
            x170_scanTimes[i].second = 1.f;
        else
            x170_scanTimes[i].second = 0.f;
    }

    x180_ = stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100));
    x184_ = stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100));
}

void CPlayerState::PutTo(CBitStreamWriter &stream)
{
    stream.WriteEncoded(x4_, 32);
    u32 tmp = *reinterpret_cast<int*>(&xc_currentHealth);
    stream.WriteEncoded(tmp, 32);
    stream.WriteEncoded((u32)x8_currentBeam, CBitStreamWriter::GetBitCount(5));
    stream.WriteEncoded((u32)x20_currentSuit, CBitStreamWriter::GetBitCount(4));
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

    stream.WriteEncoded(x180_, CBitStreamWriter::GetBitCount(0x100));
    stream.WriteEncoded(x184_, CBitStreamWriter::GetBitCount(0x100));
}

static const float unk[]
{
    0.2f, 0.1f, 0.2f, 0.2f, 1.f
};

float CPlayerState::sub_80091204() const
{


    return unk[u32(x8_currentBeam)];
}

static const u32 costs[]
{
    5, 10, 10, 10, 1
};

u32 CPlayerState::GetMissileCostForAltAttack() const
{
    return costs[u32(x8_currentBeam)];
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
    total += GetItemCapacity(EItemType::ArtifactOfTruth);
    total += GetItemCapacity(EItemType::ArtifactOfStrength);
    total += GetItemCapacity(EItemType::ArtifactOfElder);
    total += GetItemCapacity(EItemType::ArtifactOfWild);
    total += GetItemCapacity(EItemType::ArtifactOfLifegiver);
    total += GetItemCapacity(EItemType::ArtifactOfWarrior);
    total += GetItemCapacity(EItemType::ArtifactOfChozo);
    total += GetItemCapacity(EItemType::ArtifactOfNature);
    total += GetItemCapacity(EItemType::ArtifactOfSun);
    total += GetItemCapacity(EItemType::ArtifactOfWorld);
    total += GetItemCapacity(EItemType::ArtifactOfSpirit);
    total += GetItemCapacity(EItemType::ArtifactOfNewborn);
    return total + GetItemCapacity(EItemType::Wavebuster);
}

CPlayerState::EPlayerSuit CPlayerState::GetCurrentSuit() const
{
    if (GetFusion())
        return EPlayerSuit::FusionPower;

    return x20_currentSuit;
}

bool CPlayerState::CanVisorSeeFog(const CStateManager& stateMgr) const
{
    u32 activeVisor = u32(GetActiveVisor(stateMgr));
    if (activeVisor == 0 || activeVisor == 2)
        return true;
    return true;
}

CPlayerState::EPlayerVisor CPlayerState::GetActiveVisor(const CStateManager& stateMgr) const
{
    /* TODO: We need CGameCamera, and gang in order to enable this */
#if 0
    CFirstPersionCamera* cam = dynamic_cast<CFirstPersonCamera*>(stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr));
    if (!cam)
        return EVisorType::Combat;
#endif
    return x14_currentVisor;
}

void CPlayerState::UpdateStaticInterference(CStateManager& stateMgr, const float& dt)
{
    x188_staticIntf.Update(stateMgr, dt);
}

void CPlayerState::IncreaseScanTime(u32 time, float val)
{

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

bool CPlayerState::ItemEnabled(CPlayerState::EItemType type)
{
    if (HasPowerUp(type))
        return x24_powerups[u32(type)].x4_capacity != 0;
    return false;
}

bool CPlayerState::HasPowerUp(CPlayerState::EItemType type)
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
    if (type != EItemType::ThermalVisor && type < EItemType::Max)
        return x24_powerups[u32(type)].x0_amount;
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
    case EItemType::ArtifactOfTruth:
    case EItemType::ArtifactOfStrength:
    case EItemType::ArtifactOfElder:
    case EItemType::ArtifactOfWild:
    case EItemType::ArtifactOfLifegiver:
    case EItemType::ArtifactOfWarrior:
    case EItemType::ArtifactOfChozo:
    case EItemType::ArtifactOfNature:
    case EItemType::ArtifactOfSun:
    case EItemType::ArtifactOfWorld:
    case EItemType::ArtifactOfSpirit:
    case EItemType::ArtifactOfNewborn:
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
        xc_currentHealth = std::min(health, xc_currentHealth + amount);
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
    pup.x0_amount = zeus::clamp(u32(0), pup.x4_capacity + capacity, PowerUpMaxValues[u32(type)]);
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

}
