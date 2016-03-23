#ifndef __PSHAG_CPLAYERSTATE_HPP__
#define __PSHAG_CPLAYERSTATE_HPP__

#include "RetroTypes.hpp"
#include "CBasics.hpp"
#include "CStaticInterference.hpp"
#include "IOStreams.hpp"
#include "rstl.hpp"

namespace urde
{

class CPlayerState
{
public:
    enum class EItemType : u32
    {
        PowerBeam,
        IceBeam,
        WaveBeam,
        PlasmaBeam,
        Missiles,
        ScanVisor,
        MorphBallBombs,
        PowerBombs,
        Flamethrower,
        ThermalVisor,
        ChargeBeam,
        SuperMissile,
        GrappleBeam,
        XRayVisor,
        IceSpreader,
        SpaceJumpBoots,
        MorphBall,
        CombatVisor,
        BoostBall,
        SpiderBall,
        PowerSuit,
        GravitySuit,
        VariaSuit,
        PhazonSuit,
        EnergyTanks,
        UnknownItem1,
        HealthRefill,
        UnknownItem2,
        Wavebuster,
        ArtifactOfTruth,
        ArtifactOfStrength,
        ArtifactOfElder,
        ArtifactOfWild,
        ArtifactOfLifegiver,
        ArtifactOfWarrior,
        ArtifactOfChozo,
        ArtifactOfNature,
        ArtifactOfSun,
        ArtifactOfWorld,
        ArtifactOfSpirit,
        ArtifactOfNewborn,

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
    u32 x8_currentBeam = 0;
    float xc_currentHealth = 99.f;
    float x10_ = 50.f;
    EPlayerVisor x14_currentVisor = EPlayerVisor::Combat;
    EPlayerVisor x18_transitioningVisor = x14_currentVisor;
    float x1c_visorTransitionFactor = 0.2f;
    EPlayerSuit x20_currentSuit = EPlayerSuit::Power;
    rstl::reserved_vector<CPowerUp, 41> x24_powerups;

    CStaticInterference x188_staticIntf;
public:

    u32 CalculateItemCollectionRate() const;

    u32 GetBaseHealthCapacityInt32() { return 99; }
    void SetFusion(bool val) { x0_26_fusion = val; }
    bool GetFusion() const { return x0_26_fusion; }
    EPlayerSuit GetCurrentSuit() const;
    bool CanVisorSeeFog(const CStateManager& stateMgr) const;
    EPlayerVisor GetActiveVisor(const CStateManager& stateMgr) const;
    void UpdateStaticInterference(CStateManager& stateMgr, const float& dt);
    void IncreaseScanTime(u32 time, float val);
    void NewScanTime(u32 time);
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
};
}

#endif // __PSHAG_CPLAYERSTATE_HPP__
