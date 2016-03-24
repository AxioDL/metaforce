#include "CPlayerState.hpp"
#include "IOStreams.hpp"
#include "zeus/Math.hpp"

namespace urde
{
/* TODO: Implement this properly */
/* NOTE: This is only to be used as a reference,
 * and is not indicative of how the actual format is structured
 * a proper RE is still required!
 */

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

CPlayerState::CPlayerState(CBitStreamReader& in)
    : x188_staticIntf(5)
{
    x0_24_ = true;
    u32 bitCount = 0;
    std::for_each(std::cbegin(CPlayerState::PowerUpMaxValues), std::cend(CPlayerState::PowerUpMaxValues), [&bitCount](const u32& max){
        bitCount += CBitStreamReader::GetBitCount(max);
    });

#if 1
    in.readUint32Big();
    in.readBool();
    in.readBool();
    in.readBool();

    atInt8 data[0xAE];
    in.readBytesToBuf(data, 0xAE);
    for (u32 k = 0; k < 3; k++)
    {
        atInt8 save[0x3AC];
        in.readBytesToBuf(save, 0x3AC);
        {
            CBitStreamReader stream(save, 0x1000);
            std::string filename = athena::utility::sprintf("Game%i.dat", k + 1);
            std::string logFilename = athena::utility::sprintf("Game%i.txt", k + 1);
            FILE * f = fopen(logFilename.c_str(), "w");
            CBitStreamWriter w{filename};

            fprintf(f, "Game State\n");
            for (u32 i = 0; i < 0x80; i++)
            {
                u32 tmp = stream.ReadEncoded(8);
                if (!(i % 16))
                    fprintf(f, "\n");
                fprintf(f, "%.2i ", tmp);
                w.WriteEncoded(tmp, 8);
            }
            fprintf(f, "\n\n");

            s32 tmp = stream.ReadEncoded(32);
            w.WriteEncoded(tmp, 0x20);
            fprintf(f, "%i\n", tmp);
            tmp = stream.ReadEncoded(1);
            w.WriteEncoded(tmp, 1);
            fprintf(f, "%i\n", tmp);
            tmp = stream.ReadEncoded(1);
            w.WriteEncoded(tmp, 1);
            fprintf(f, "%i\n", tmp);
            tmp = stream.ReadEncoded(32);
            w.WriteEncoded(tmp, 0x20);
            fprintf(f, "%f\n", *reinterpret_cast<float*>(&tmp));
            tmp = stream.ReadEncoded(32);
            w.WriteEncoded(tmp, 0x20);
            fprintf(f, "%f\n", *reinterpret_cast<float*>(&tmp));
            tmp = stream.ReadEncoded(32);
            fprintf(f, "%x\n", tmp);
            w.WriteEncoded(tmp, 0x20);

            fprintf(f, "PlayerState\n");
            x4_ = stream.ReadEncoded(0x20);
            w.WriteEncoded(x4_, 0x20);
            fprintf(f, "%x\n", x4_);
            tmp = stream.ReadEncoded(0x20);
            fprintf(f, "Base health %f\n", *reinterpret_cast<float*>(&tmp));
            xc_currentHealth = *reinterpret_cast<float*>(&tmp);
            w.WriteEncoded(tmp, 0x20);
            x8_currentBeam = stream.ReadEncoded(CBitStreamReader::GetBitCount(5));
            fprintf(f, "%i\n", x8_currentBeam);
            w.WriteEncoded(x8_currentBeam, CBitStreamReader::GetBitCount(5));
            x20_currentSuit = EPlayerSuit(stream.ReadEncoded(CBitStreamReader::GetBitCount(4)));
            fprintf(f, "%i\n", x20_currentSuit);
            w.WriteEncoded(u32(x20_currentSuit), CBitStreamReader::GetBitCount(4));
            x24_powerups.resize(41);
            fprintf(f, "Powerups\n");
            for (u32 i = 0; i < x24_powerups.size(); ++i)
            {
                if (PowerUpMaxValues[i] == 0)
                    continue;

                u32 a = stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxValues[i]));
                u32 b = stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxValues[i]));
                w.WriteEncoded(a, CBitStreamReader::GetBitCount(PowerUpMaxValues[i]));
                w.WriteEncoded(b, CBitStreamReader::GetBitCount(PowerUpMaxValues[i]));
                x24_powerups[i] = CPowerUp(a, b);
                fprintf(f, "%2i(%21s): cur=%3i max=%3i\n", i, PowerUpNames[i], a, b);
            }

            for (u32 i = 0; i < 832; i++)
            {
                u32 tmp = stream.ReadEncoded(1);
                if (!(i % 32))
                    fprintf(f, "\n");

                fprintf(f, "%i ", tmp);
                w.WriteEncoded(tmp, 1);
            }
            fprintf(f, "\n\n");

            tmp = stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100));
            w.WriteEncoded(tmp, CBitStreamReader::GetBitCount(0x100));
            fprintf(f, "%i\n", tmp);
            tmp = stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100));
            w.WriteEncoded(tmp, CBitStreamReader::GetBitCount(0x100));
            fprintf(f, "%i\n", tmp);

            fprintf(f, "Final Offset %.8llx\n", stream.position());
            fprintf(f, "Completion: %.2i%%\n", CalculateItemCollectionRate());
            w.save();
        }
    }
#endif
}

float CPlayerState::GetBeamSwitchTime() const
{
    static const float switchTimes[4] {
        0.2, 0.1, 0.2, 0.2
    };

    return switchTimes[u32(x8_currentBeam)];
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
        return EVisorType::Zero;
#endif
    return x14_currentVisor;
}

void CPlayerState::UpdateStaticInterference(CStateManager& stateMgr, const float& dt)
{
    x188_staticIntf.Update(stateMgr, dt);
}

void CPlayerState::NewScanTime(u32 time)
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
