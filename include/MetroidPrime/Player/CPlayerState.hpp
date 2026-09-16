#ifndef _CPLAYERSTATE
#define _CPLAYERSTATE

#include "Kyoto/SObjectTag.hpp"
#include "types.h"

#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/Player/CStaticInterference.hpp"

#include "Kyoto/IObjectStore.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CStateManager;

class CPlayerState {
public:
  enum EItemType {
    kIT_Invalid = -1,
    kIT_PowerBeam = 0,
    kIT_IceBeam = 1,
    kIT_WaveBeam = 2,
    kIT_PlasmaBeam = 3,
    kIT_Missiles = 4,
    kIT_ScanVisor = 5,
    kIT_MorphBallBombs = 6,
    kIT_PowerBombs = 7,
    kIT_Flamethrower = 8,
    kIT_ThermalVisor = 9,
    kIT_ChargeBeam = 10,
    kIT_SuperMissile = 11,
    kIT_GrappleBeam = 12,
    kIT_XRayVisor = 13,
    kIT_IceSpreader = 14,
    kIT_SpaceJumpBoots = 15,
    kIT_MorphBall = 16,
    kIT_CombatVisor = 17,
    kIT_BoostBall = 18,
    kIT_SpiderBall = 19,
    kIT_PowerSuit = 20,
    kIT_GravitySuit = 21,
    kIT_VariaSuit = 22,
    kIT_PhazonSuit = 23,
    kIT_EnergyTanks = 24,
    kIT_UnknownItem1 = 25,
    kIT_HealthRefill = 26,
    kIT_UnknownItem2 = 27,
    kIT_Wavebuster = 28,
    kIT_Truth = 29,
    kIT_Strength = 30,
    kIT_Elder = 31,
    kIT_Wild = 32,
    kIT_Lifegiver = 33,
    kIT_Warrior = 34,
    kIT_Chozo = 35,
    kIT_Nature = 36,
    kIT_Sun = 37,
    kIT_World = 38,
    kIT_Spirit = 39,
    kIT_Newborn = 40,

    /* This must remain at the end of the list */
    kIT_Max
  };

  enum EPlayerVisor {
    kPV_Combat,
    kPV_XRay,
    kPV_Scan,
    kPV_Thermal,
  };
  enum EPlayerSuit {
    kPS_Invalid = -1,
    kPS_Power,
    kPS_Gravity,
    kPS_Varia,
    kPS_Phazon,
    kPS_FusionPower,
    kPS_FusionGravity,
    kPS_FusionVaria,
    kPS_FusionPhazon,
  };
  enum EBeamId {
    kBI_Invalid = -1,
    kBI_Power,
    kBI_Ice,
    kBI_Wave,
    kBI_Plasma,
    kBI_Phazon,
    kBI_Phazon2 = 27,
  };
  enum EChargeStage { kCS_Normal, kCS_Charged };

  CPlayerState();
  explicit CPlayerState(CInputStream& stream);
  void PutTo(COutputStream& stream);

  int GetMissileCostForAltAttack() const;
  float GetComboFireAmmoPeriod() const;
  static float GetMissileComboChargeFactor();
  int CalculateItemCollectionRate() const;
  int CalculateItemCollectionPercentage() const;
  int GetTotalPickupCount() const;

  void SetIsFusionEnabled(bool v);
  bool GetIsFusionEnabled() const;

  EPlayerSuit GetCurrentSuit() const;
  EPlayerSuit GetCurrentSuitRaw() const { return x20_currentSuit; }
  void SetFiringComboBeam(bool firing) { x0_25_firingComboBeam = firing; }
  bool IsFiringComboBeam() const { return x0_25_firingComboBeam; }
  EBeamId GetCurrentBeam() const { return x8_currentBeam; }
  void SetCurrentBeam(EBeamId beam) { x8_currentBeam = beam; }
  bool CanVisorSeeFog(const CStateManager& stateMgr) const;
  EPlayerVisor GetCurrentVisor() const { return x14_currentVisor; }
  EPlayerVisor GetTransitioningVisor() const { return x18_transitioningVisor; }
  EPlayerVisor GetActiveVisor(const CStateManager& mgr) const;
  const bool IsXRayActive(const CStateManager& mgr) const {
    return GetActiveVisor(mgr) == kPV_XRay;
  }
  const bool IsStandardVisor(const CStateManager& mgr) const {
    EPlayerVisor activeVisor = GetActiveVisor(mgr);
    return (activeVisor == kPV_Combat || activeVisor == kPV_Scan);
  }

  void UpdateStaticInterference(CStateManager& stateMgr, const float& dt);
  void IncreaseScanTime(uint time, float val);
  void SetScanTime(CAssetId res, float time);
  const float GetScanTime(CAssetId res) const;
  bool GetIsVisorTransitioning() const;
  float GetVisorTransitionFactor() const;
  void UpdateVisorTransition(float dt);
  void StartTransitionToVisor(EPlayerVisor visor);
  void ResetVisor();
  bool IsAlive() const { return x0_24_alive; }
  void SetPlayerAlive(bool alive) { x0_24_alive = alive; }

  const bool ItemEnabled(const EItemType type) const;
  void DisableItem(const EItemType type);
  void EnableItem(const EItemType type);
  const bool HasPowerUp(const EItemType type) const;
  const uint GetPowerUp(const EItemType type);
  const int GetItemCapacity(const EItemType type) const;
  const int GetItemAmount(const EItemType type) const;
  void DecrPickUp(const EItemType type, const int amount);
  void IncrPickUp(const EItemType type, const int amount);
  void SetPickup(const EItemType type, const int amount);
  static float GetEnergyTankCapacity();
  static float GetBaseHealthCapacity();
  const float CalculateHealth();
  void InitializePowerUp(CPlayerState::EItemType type, int capacity);
  void SetPowerUp(CPlayerState::EItemType type, int capacity);
  static bool IsValidScan(CAssetId res);
  void SetScanCompletionRateFirst(int rate) { x180_scanCompletionRateFirst = rate; }   // name?
  void SetScanCompletionRateSecond(int rate) { x184_scanCompletionRateSecond = rate; } // name?

  void InitializeScanTimes();

  static const uint GetBitCount(uint);

  float GetHudStaticInterferenceAmount() const { return x188_staticIntf.GetTotalInterference(); }
  CStaticInterference& StaticInterference() { return x188_staticIntf; }
  const CStaticInterference& GetStaticInterference() const { return x188_staticIntf; }

  const rstl::vector< rstl::pair< CAssetId, float > >& GetScanTimes() const {
    return x170_scanTimes;
  }
  const int GetLogScans() const { return x180_scanCompletionRateFirst; }
  const int GetTotalLogScans() const { return x184_scanCompletionRateSecond; }

  CHealthInfo* HealthInfo() { return &xc_health; }
  const CHealthInfo& GetHealthInfo() const { return xc_health; }

private:
  struct CPowerUp {
    int x0_amount;
    int x4_capacity;
    CPowerUp() : x0_amount(0), x4_capacity(0) {}
    CPowerUp(int amount, int capacity);

    void Add(int amount) {
      int capacity = x4_capacity;
      x0_amount += amount;
      if (x0_amount > capacity) {
        x0_amount = capacity;
      }
    }

    void Dec(int amount) {
      x0_amount -= amount;
      if (x0_amount < 0) {
        x0_amount = 0;
      }
    }
  };

  bool x0_24_alive : 1;
  bool x0_25_firingComboBeam : 1;
  bool x0_26_fusion : 1;
  uint x4_enabledItems;
  EBeamId x8_currentBeam;
  CHealthInfo xc_health;
  EPlayerVisor x14_currentVisor;
  EPlayerVisor x18_transitioningVisor;
  float x1c_visorTransitionFactor;
  EPlayerSuit x20_currentSuit;
  rstl::reserved_vector< CPowerUp, 41 > x24_powerups;
  rstl::vector< rstl::pair< CAssetId, float > > x170_scanTimes;
  int x180_scanCompletionRateFirst; // pair?
  int x184_scanCompletionRateSecond;
  CStaticInterference x188_staticIntf;
};
CHECK_SIZEOF(CPlayerState, 0x198)

#endif // _CPLAYERSTATE
