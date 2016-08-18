#ifndef __URDE_CPLAYERSTATE_HPP__
#define __URDE_CPLAYERSTATE_HPP__

#include "RetroTypes.hpp"
#include "CBasics.hpp"
#include "CStaticInterference.hpp"
#include "IOStreams.hpp"
#include "rstl.hpp"
#include "World/CHealthInfo.hpp"

namespace urde
{

class CPlayerState
{
    friend class CWorldTransManager;
public:
    enum class EItemType : u32
    {
        PowerBeam = 0,
        IceBeam = 1,
        WaveBeam = 2,
        PlasmaBeam = 3,
        Missiles = 4,
        ScanVisor = 5,
        MorphBallBombs = 6,
        PowerBombs = 7,
        Flamethrower = 8,
        ThermalVisor = 9,
        ChargeBeam = 10,
        SuperMissile = 11,
        GrappleBeam = 12,
        XRayVisor = 13,
        IceSpreader = 14,
        SpaceJumpBoots = 15,
        MorphBall = 16,
        CombatVisor = 17,
        BoostBall = 18,
        SpiderBall = 19,
        PowerSuit = 20,
        GravitySuit = 21,
        VariaSuit = 22,
        PhazonSuit = 23,
        EnergyTanks = 24,
        UnknownItem1 = 25,
        HealthRefill = 26,
        UnknownItem2 = 27,
        Wavebuster = 28,
        ArtifactOfTruth = 29,
        ArtifactOfStrength = 30,
        ArtifactOfElder = 31,
        ArtifactOfWild = 32,
        ArtifactOfLifegiver = 33,
        ArtifactOfWarrior = 34,
        ArtifactOfChozo = 35,
        ArtifactOfNature = 36,
        ArtifactOfSun = 37,
        ArtifactOfWorld = 38,
        ArtifactOfSpirit = 39,
        ArtifactOfNewborn = 40,

        /* This must remain at the end of the list */
        Max
    };

    enum class EPlayerVisor : u32
    {
        Combat,
        XRay,
        Scan,
        Thermal,

        /* This must remain at the end of the list */
        Max
    };

    enum class EPlayerSuit : u32
    {
        Power,
        Gravity,
        Varia,
        Phazon,
        FusionPower,
        FusionGravity,
        FusionVaria,
        FusionPhazon
    };

    enum class EBeamId : u32
    {
        Power,
        Ice,
        Plasma,
        Wave,
        Phazon
    };

private:

    static const u32 PowerUpMaxValues[41];
    struct CPowerUp
    {
        int x0_amount = 0;
        int x4_capacity = 0;
        CPowerUp() {}
        CPowerUp(int amount, int capacity) : x0_amount(amount), x4_capacity(capacity) {}
    };
    union
    {
        struct { bool x0_24_ : 1; bool x0_25_ : 1; bool x0_26_fusion; };
        u32 dummy = 0;
    };

    u32 x4_ = 0;
    EBeamId x8_currentBeam = EBeamId::Power;
    CHealthInfo xc_health = {99.f, 50.f};
    EPlayerVisor x14_currentVisor = EPlayerVisor::Combat;
    EPlayerVisor x18_transitioningVisor = x14_currentVisor;
    float x1c_visorTransitionFactor = 0.2f;
    EPlayerSuit x20_currentSuit = EPlayerSuit::Power;
    rstl::reserved_vector<CPowerUp, 41> x24_powerups;
    rstl::reserved_vector<std::pair<u32, float>, 846> x170_scanTimes;
    u32 x180_ = 0;
    u32 x184_ = 0;
    CStaticInterference x188_staticIntf;
public:

    float sub_80091204() const;
    u32 GetMissileCostForAltAttack() const;
    u32 CalculateItemCollectionRate() const;

    CHealthInfo& HealthInfo();
    CHealthInfo GetHealthInfo() const;
    u32 GetPickupTotal() { return 99; }
    void SetFusion(bool val) { x0_26_fusion = val; }
    bool GetFusion() const { return x0_26_fusion; }
    EPlayerSuit GetCurrentSuit() const;
    EBeamId GetCurrentBeam() const { return x8_currentBeam; }
    bool CanVisorSeeFog(const CStateManager& stateMgr) const;
    EPlayerVisor GetActiveVisor(const CStateManager& stateMgr) const;
    void UpdateStaticInterference(CStateManager& stateMgr, const float& dt);
    void IncreaseScanTime(u32 time, float val);
    void SetScanTime(u32 time, float val);
    float GetScanTime(u32 time, float val);
    bool GetIsVisorTransitioning() const;
    float GetVisorTransitionFactor() const;
    void UpdateVisorTransition(float dt);
    bool StartVisorTransition(EPlayerVisor visor);
    void ResetVisor();
    bool ItemEnabled(EItemType type);
    void DisableItem(EItemType type);
    void EnableItem(EItemType type);
    bool HasPowerUp(EItemType type);
    u32 GetItemCapacity(EItemType type) const;
    u32 GetItemAmount(EItemType type) const;
    void DecrPickup(EItemType type, s32 amount);
    void IncrPickup(EItemType type, s32 amount);
    void ResetAndIncrPickUp(EItemType type, s32 amount);
    float GetEnergyTankCapacity() const { return 100.f; }
    float GetBaseHealthCapacity() const { return 99.f; }
    float CalculateHealth(u32 health);
    void ReInitalizePowerUp(EItemType type, u32 capacity);
    void InitializePowerUp(EItemType type, u32 capacity);
    CPlayerState() : x188_staticIntf(5) { x0_24_ = true; }
    CPlayerState(CBitStreamReader& stream);
    void PutTo(CBitStreamWriter& stream);

};
}

#endif // __URDE_CPLAYERSTATE_HPP__
