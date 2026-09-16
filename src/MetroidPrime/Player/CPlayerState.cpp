#include "MetroidPrime/Player/CPlayerState.hpp"

#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include <float.h>
#include <math.h>

static const int kPowerUpMax[] = {
    1, 1, 1, 1,  250, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 14, 1,   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const int kMissileCosts[] = {
    5, 10, 10, 10, 1,
};

static const float kComboAmmoPeriods[] = {
    0.2f, 0.1f, 0.2f, 0.2f, 1.f,
};

static const char* kVisorNames[] = {
    "CombatVisor",
    "XRayVisor",
    "ScanVisor",
    "ThermalVisor",
};

static const float kEnergyTankCapacity = 100.f;
static const float kBaseHealthCapacity = 99.f;

static const float kDefaultKnockbackResistance = 50.f;
static const float kMaxVisorTransitionFactor = 0.2f;

static inline void do_nothing() {}

const uint CPlayerState::GetBitCount(uint val) {
  int bits = 0;
  for (; val != 0; val >>= 1) {
    bits += 1;
  }
  return bits;
}

CPlayerState::CPowerUp::CPowerUp(int amount, int capacity)
: x0_amount(amount), x4_capacity(capacity) {}

CPlayerState::CPlayerState()
: x0_24_alive(true)
, x0_25_firingComboBeam(false)
, x0_26_fusion(false)
, x4_enabledItems(0)
, x8_currentBeam(kBI_Power)
, xc_health(kBaseHealthCapacity, kDefaultKnockbackResistance)
, x14_currentVisor(kPV_Combat)
, x18_transitioningVisor(x14_currentVisor)
, x1c_visorTransitionFactor(kMaxVisorTransitionFactor)
, x20_currentSuit(kPS_Power)
, x24_powerups(CPowerUp(0, 0))
, x170_scanTimes()
, x180_scanCompletionRateFirst(0)
, x184_scanCompletionRateSecond(0)
, x188_staticIntf(5) {}

CPlayerState::CPlayerState(CInputStream& stream)
: x0_24_alive(true)
, x0_25_firingComboBeam(false)
, x0_26_fusion(false)
, x4_enabledItems(0)
, x8_currentBeam(kBI_Power)
, xc_health(kBaseHealthCapacity, kDefaultKnockbackResistance)
, x14_currentVisor(kPV_Combat)
, x18_transitioningVisor(x14_currentVisor)
, x1c_visorTransitionFactor(kMaxVisorTransitionFactor)
, x20_currentSuit(kPS_Power)
, x24_powerups()
, x170_scanTimes()
, x180_scanCompletionRateFirst(0)
, x184_scanCompletionRateSecond(0)
, x188_staticIntf(5) {
  x4_enabledItems = uint(stream.ReadBits(32));

  const uint integralHP = uint(stream.ReadBits(32));
  xc_health.SetHP(*(float*)(&integralHP));
  xc_health.SetKnockbackResistance(kDefaultKnockbackResistance);

  x8_currentBeam = EBeamId(stream.ReadBits(GetBitCount(5)));
  x20_currentSuit = EPlayerSuit(stream.ReadBits(GetBitCount(4)));

  for (int i = 0; i < x24_powerups.capacity(); ++i) {
    int amount = 0;
    int capacity = 0;

    uint maxValue = kPowerUpMax[i];
    if (maxValue != 0) {
      uint bitCount = GetBitCount(maxValue);
      amount = stream.ReadBits(bitCount);
      capacity = stream.ReadBits(bitCount);
    }
    CPowerUp pw(amount, capacity);
    x24_powerups.push_back(pw);
  }

  // Scan
  const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  x170_scanTimes.reserve(scanStates.size());
  for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
       it != scanStates.end(); ++it) {
    bool isScanned = stream.ReadBits(1) != 0;
    x170_scanTimes.push_back(rstl::pair< CAssetId, float >(it->first, isScanned ? 1.f : 0.f));
  }

  x180_scanCompletionRateFirst = uint(stream.ReadBits(GetBitCount(0x100u)));
  x184_scanCompletionRateSecond = uint(stream.ReadBits(GetBitCount(0x100u)));
}

void CPlayerState::PutTo(COutputStream& stream) {
  stream.WriteBits(x4_enabledItems, 32);

  const float realHP = xc_health.GetHP();
  stream.WriteBits(*(int*)(&realHP), 32);
  stream.WriteBits(x8_currentBeam, GetBitCount(5));
  stream.WriteBits(x20_currentSuit, GetBitCount(4));

  CPowerUp* powup = x24_powerups.data();
  for (int i = 0; i < x24_powerups.capacity(); ++i) {
    if (0 < kPowerUpMax[i]) {
      int bitCount = GetBitCount(kPowerUpMax[i]);
      stream.WriteBits(powup[i].x0_amount, bitCount);
      stream.WriteBits(powup[i].x4_capacity, bitCount);
    }
  }

  for (rstl::vector< rstl::pair< CAssetId, float > >::iterator it = x170_scanTimes.begin();
       it != x170_scanTimes.end(); it += 1) {
    int flag;
    if (it->second >= 1.f) {
      flag = 1;
    } else {
      flag = 0;
    }
    stream.WriteBits(flag, 1);
  }

  stream.WriteBits(x180_scanCompletionRateFirst, GetBitCount(0x100));
  stream.WriteBits(x184_scanCompletionRateSecond, GetBitCount(0x100));
}

void CPlayerState::SetPowerUp(CPlayerState::EItemType type, int capacity) {
  x24_powerups[uint(type)].x4_capacity = 0;
  InitializePowerUp(type, capacity);
}

void CPlayerState::InitializePowerUp(CPlayerState::EItemType type, int capacity) {
  if (type < kIT_PowerBeam || type > kIT_Max - 1)
    return;

  CPowerUp& pup = x24_powerups[uint(type)];
  pup.x4_capacity = CMath::Clamp(0, capacity + pup.x4_capacity, kPowerUpMax[uint(type)]);
  pup.x0_amount = rstl::min_val(pup.x0_amount, pup.x4_capacity);
  if (type >= kIT_PowerSuit && type <= kIT_PhazonSuit) {
    if (HasPowerUp(kIT_PhazonSuit))
      x20_currentSuit = kPS_Phazon;
    else if (HasPowerUp(kIT_GravitySuit))
      x20_currentSuit = kPS_Gravity;
    else if (HasPowerUp(kIT_VariaSuit))
      x20_currentSuit = kPS_Varia;
    else
      x20_currentSuit = kPS_Power;
  }
}

const float CPlayerState::CalculateHealth() {
  return (kEnergyTankCapacity * x24_powerups[kIT_EnergyTanks].x0_amount) + kBaseHealthCapacity;
}

void CPlayerState::SetPickup(const CPlayerState::EItemType type, const int amount) {
  x24_powerups[uint(type)].x0_amount = 0;
  IncrPickUp(type, amount);
}

void CPlayerState::IncrPickUp(EItemType type, int amount) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }

  if (0 <= amount) {
    switch (type) {
    case kIT_Missiles:
    case kIT_PowerBombs:
    case kIT_ChargeBeam:
    case kIT_SpaceJumpBoots:
    case kIT_EnergyTanks:
    case kIT_Truth:
    case kIT_Strength:
    case kIT_Elder:
    case kIT_Wild:
    case kIT_Lifegiver:
    case kIT_Warrior:
    case kIT_Chozo:
    case kIT_Nature:
    case kIT_Sun:
    case kIT_World:
    case kIT_Spirit:
    case kIT_Newborn: {
      x24_powerups[type].Add(amount);
      break;
    }
    case kIT_HealthRefill: {
      CHealthInfo* info = &xc_health;
      if (info != NULL) {
        float newHealth = float(amount) + info->GetHP();
        float maxHealth = CalculateHealth();
        if (newHealth > maxHealth) {
          info->SetHP(maxHealth);
        } else {
          info->SetHP(newHealth);
        }
      }
    }
    }
    if (type == kIT_EnergyTanks)
      IncrPickUp(kIT_HealthRefill, 9999);
  }
}

void CPlayerState::DecrPickUp(CPlayerState::EItemType type, int amount) {
  if (type < 0 || kIT_Max - 1 < type) {
    return;
  }

  switch (type) {
  case kIT_Missiles:
  case kIT_PowerBombs:
  case kIT_Flamethrower:
    x24_powerups[type].Dec(amount);
  default:
    return;
  }
}

const int CPlayerState::GetItemAmount(const CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }

  switch (type) {
  case kIT_SpaceJumpBoots:
  case kIT_PowerBombs:
  case kIT_Flamethrower:
  case kIT_EnergyTanks:
  case kIT_Missiles:
  case kIT_Truth:
  case kIT_Strength:
  case kIT_Elder:
  case kIT_Wild:
  case kIT_Lifegiver:
  case kIT_Warrior:
  case kIT_Chozo:
  case kIT_Nature:
  case kIT_Sun:
  case kIT_World:
  case kIT_Spirit:
  case kIT_Newborn:
    return x24_powerups[uint(type)].x0_amount;
  default:
    break;
  }

  return 0;
}

const int CPlayerState::GetItemCapacity(const CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return x24_powerups[uint(type)].x4_capacity;
}

const bool CPlayerState::HasPowerUp(const CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return false;
  }
  return x24_powerups[uint(type)].x4_capacity > 0;
}

const uint CPlayerState::GetPowerUp(const CPlayerState::EItemType type) {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return x24_powerups[uint(type)].x4_capacity;
}

void CPlayerState::EnableItem(const CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    x4_enabledItems |= (1 << uint(type));
}

void CPlayerState::DisableItem(const CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    x4_enabledItems &= ~(1 << uint(type));
}

const bool CPlayerState::ItemEnabled(const CPlayerState::EItemType type) const {
  if (HasPowerUp(type))
    return (x4_enabledItems & (1 << uint(type)));
  return false;
}

void CPlayerState::ResetVisor() {
  x14_currentVisor = x18_transitioningVisor = kPV_Combat;
  x1c_visorTransitionFactor = 0.0f;
}

void CPlayerState::StartTransitionToVisor(const CPlayerState::EPlayerVisor visor) {
  if (visor == x18_transitioningVisor)
    return;

  x18_transitioningVisor = visor;

  if (x18_transitioningVisor == x14_currentVisor)
    return;
}

void CPlayerState::UpdateVisorTransition(float dt) {
  if (!GetIsVisorTransitioning())
    return;

  if (x14_currentVisor == x18_transitioningVisor) {
    x1c_visorTransitionFactor = rstl::min_val(kMaxVisorTransitionFactor, x1c_visorTransitionFactor + dt);
  } else {
    x1c_visorTransitionFactor -= dt;
    if (x1c_visorTransitionFactor < 0.f) {
      x14_currentVisor = x18_transitioningVisor;
      x1c_visorTransitionFactor = fabs(x1c_visorTransitionFactor);
      x1c_visorTransitionFactor =
          rstl::min_val(x1c_visorTransitionFactor, kMaxVisorTransitionFactor - FLT_EPSILON);
    }
  }
}

float CPlayerState::GetVisorTransitionFactor() const {
  return x1c_visorTransitionFactor / kMaxVisorTransitionFactor;
}

bool CPlayerState::GetIsVisorTransitioning() const {
  return x14_currentVisor != x18_transitioningVisor ||
         kMaxVisorTransitionFactor > x1c_visorTransitionFactor;
}

float CPlayerState::GetBaseHealthCapacity() { return kBaseHealthCapacity; }

float CPlayerState::GetEnergyTankCapacity() { return kEnergyTankCapacity; }

void CPlayerState::InitializeScanTimes() {
  if (x170_scanTimes.size())
    return;

  const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  x170_scanTimes.reserve(scanStates.size());
  for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
       it != scanStates.end(); ++it) {
    x170_scanTimes.push_back(rstl::pair< CAssetId, float >(it->first, 0.f));
  }
}

const float CPlayerState::GetScanTime(const CAssetId res) const {
  rstl::vector< rstl::pair< CAssetId, float > >::const_iterator it =
      rstl::find_by_key(x170_scanTimes, res);
  return it->second;
}

void CPlayerState::SetScanTime(const CAssetId res, float time) {
  rstl::vector< rstl::pair< CAssetId, float > >::iterator it =
      rstl::find_by_key_nc(x170_scanTimes, res);
  it->second = time;
}

void CPlayerState::UpdateStaticInterference(CStateManager& stateMgr, const float& dt) {
  x188_staticIntf.Update(stateMgr, dt);
}

CPlayerState::EPlayerVisor CPlayerState::GetActiveVisor(const CStateManager& stateMgr) const {
  const CFirstPersonCamera* cam = TCastToConstPtr< CFirstPersonCamera >(
      stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr));
  return (cam ? x14_currentVisor : kPV_Combat);
}

bool CPlayerState::CanVisorSeeFog(const CStateManager& stateMgr) const {
  EPlayerVisor activeVisor = GetActiveVisor(stateMgr);
  return (activeVisor == kPV_Combat || activeVisor == kPV_Scan);
}

CPlayerState::EPlayerSuit CPlayerState::GetCurrentSuit() const {
  if (GetIsFusionEnabled())
    return kPS_FusionPower;

  return x20_currentSuit;
}

bool CPlayerState::GetIsFusionEnabled() const { return x0_26_fusion || false; }

void CPlayerState::SetIsFusionEnabled(bool val) { x0_26_fusion = val; }

int CPlayerState::GetTotalPickupCount() const { return 99; }

int CPlayerState::CalculateItemCollectionRate() const {
  int pbCount = GetItemCapacity(kIT_PowerBombs);
  return GetItemCapacity(kIT_IceBeam) + GetItemCapacity(kIT_WaveBeam) +
         GetItemCapacity(kIT_PlasmaBeam) + GetItemCapacity(kIT_Missiles) / 5 +
         GetItemCapacity(kIT_MorphBallBombs) + (pbCount >= 4 ? pbCount - 3 : pbCount) +
         GetItemCapacity(kIT_Flamethrower) + GetItemCapacity(kIT_ThermalVisor) +
         GetItemCapacity(kIT_ChargeBeam) + GetItemCapacity(kIT_SuperMissile) +
         GetItemCapacity(kIT_GrappleBeam) + GetItemCapacity(kIT_XRayVisor) +
         GetItemCapacity(kIT_IceSpreader) + GetItemCapacity(kIT_SpaceJumpBoots) +
         GetItemCapacity(kIT_MorphBall) + GetItemCapacity(kIT_BoostBall) +
         GetItemCapacity(kIT_SpiderBall) + GetItemCapacity(kIT_GravitySuit) +
         GetItemCapacity(kIT_VariaSuit) + GetItemCapacity(kIT_EnergyTanks) +
         GetItemCapacity(kIT_Truth) + GetItemCapacity(kIT_Strength) + GetItemCapacity(kIT_Elder) +
         GetItemCapacity(kIT_Wild) + GetItemCapacity(kIT_Lifegiver) + GetItemCapacity(kIT_Warrior) +
         GetItemCapacity(kIT_Chozo) + GetItemCapacity(kIT_Nature) + GetItemCapacity(kIT_Sun) +
         GetItemCapacity(kIT_World) + GetItemCapacity(kIT_Spirit) + GetItemCapacity(kIT_Newborn) +
         GetItemCapacity(kIT_Wavebuster);
}

int CPlayerState::CalculateItemCollectionPercentage() const {
  return CalculateItemCollectionRate() * 100 / GetTotalPickupCount();
}

int CPlayerState::GetMissileCostForAltAttack() const {
  return kMissileCosts[size_t(x8_currentBeam)];
}

float CPlayerState::GetComboFireAmmoPeriod() const {
  return kComboAmmoPeriods[size_t(x8_currentBeam)];
}

float CPlayerState::GetMissileComboChargeFactor() { return 1.8f; }
