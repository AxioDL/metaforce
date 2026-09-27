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
: mAmount(amount), mCapacity(capacity) {}

CPlayerState::CPlayerState()
: mAlive(true)
, mFiringComboBeam(false)
, mFusion(false)
, mEnabledItems(0)
, mCurrentBeam(kBI_Power)
, mHealth(kBaseHealthCapacity, kDefaultKnockbackResistance)
, mCurrentVisor(kPV_Combat)
, mTransitioningVisor(mCurrentVisor)
, mVisorTransitionFactor(kMaxVisorTransitionFactor)
, mCurrentSuit(kPS_Power)
, mPowerups(CPowerUp(0, 0))
, mScanTimes()
, mScanCompletionRateFirst(0)
, mScanCompletionRateSecond(0)
, mStaticIntf(5) {}

CPlayerState::CPlayerState(CInputStream& stream)
: mAlive(true)
, mFiringComboBeam(false)
, mFusion(false)
, mEnabledItems(0)
, mCurrentBeam(kBI_Power)
, mHealth(kBaseHealthCapacity, kDefaultKnockbackResistance)
, mCurrentVisor(kPV_Combat)
, mTransitioningVisor(mCurrentVisor)
, mVisorTransitionFactor(kMaxVisorTransitionFactor)
, mCurrentSuit(kPS_Power)
, mPowerups()
, mScanTimes()
, mScanCompletionRateFirst(0)
, mScanCompletionRateSecond(0)
, mStaticIntf(5) {
  mEnabledItems = uint(stream.ReadBits(32));

  const uint integralHP = uint(stream.ReadBits(32));
  mHealth.SetHP(*(float*)(&integralHP));
  mHealth.SetKnockbackResistance(kDefaultKnockbackResistance);

  mCurrentBeam = EBeamId(stream.ReadBits(GetBitCount(5)));
  mCurrentSuit = EPlayerSuit(stream.ReadBits(GetBitCount(4)));

  for (int i = 0; i < mPowerups.capacity(); ++i) {
    int amount = 0;
    int capacity = 0;

    uint maxValue = kPowerUpMax[i];
    if (maxValue != 0) {
      uint bitCount = GetBitCount(maxValue);
      amount = stream.ReadBits(bitCount);
      capacity = stream.ReadBits(bitCount);
    }
    CPowerUp pw(amount, capacity);
    mPowerups.push_back(pw);
  }

  // Scan
  const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  mScanTimes.reserve(scanStates.size());
  for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
       it != scanStates.end(); ++it) {
    bool isScanned = stream.ReadBits(1) != 0;
    mScanTimes.push_back(rstl::pair< CAssetId, float >(it->first, isScanned ? 1.f : 0.f));
  }

  mScanCompletionRateFirst = uint(stream.ReadBits(GetBitCount(0x100u)));
  mScanCompletionRateSecond = uint(stream.ReadBits(GetBitCount(0x100u)));
}

void CPlayerState::PutTo(COutputStream& stream) {
  stream.WriteBits(mEnabledItems, 32);

  const float realHP = mHealth.GetHP();
  stream.WriteBits(*(int*)(&realHP), 32);
  stream.WriteBits(mCurrentBeam, GetBitCount(5));
  stream.WriteBits(mCurrentSuit, GetBitCount(4));

  CPowerUp* powup = mPowerups.data();
  for (int i = 0; i < mPowerups.capacity(); ++i) {
    if (0 < kPowerUpMax[i]) {
      int bitCount = GetBitCount(kPowerUpMax[i]);
      stream.WriteBits(powup[i].mAmount, bitCount);
      stream.WriteBits(powup[i].mCapacity, bitCount);
    }
  }

  for (rstl::vector< rstl::pair< CAssetId, float > >::iterator it = mScanTimes.begin();
       it != mScanTimes.end(); it += 1) {
    int flag;
    if (it->second >= 1.f) {
      flag = 1;
    } else {
      flag = 0;
    }
    stream.WriteBits(flag, 1);
  }

  stream.WriteBits(mScanCompletionRateFirst, GetBitCount(0x100));
  stream.WriteBits(mScanCompletionRateSecond, GetBitCount(0x100));
}

void CPlayerState::SetPowerUp(CPlayerState::EItemType type, int capacity) {
  mPowerups[uint(type)].mCapacity = 0;
  InitializePowerUp(type, capacity);
}

void CPlayerState::InitializePowerUp(CPlayerState::EItemType type, int capacity) {
  if (type < kIT_PowerBeam || type > kIT_Max - 1)
    return;

  CPowerUp& pup = mPowerups[uint(type)];
  pup.mCapacity = CMath::Clamp(0, capacity + pup.mCapacity, kPowerUpMax[uint(type)]);
  pup.mAmount = rstl::min_val(pup.mAmount, pup.mCapacity);
  if (type >= kIT_PowerSuit && type <= kIT_PhazonSuit) {
    if (HasPowerUp(kIT_PhazonSuit))
      mCurrentSuit = kPS_Phazon;
    else if (HasPowerUp(kIT_GravitySuit))
      mCurrentSuit = kPS_Gravity;
    else if (HasPowerUp(kIT_VariaSuit))
      mCurrentSuit = kPS_Varia;
    else
      mCurrentSuit = kPS_Power;
  }
}

const float CPlayerState::CalculateHealth() {
  return (kEnergyTankCapacity * mPowerups[kIT_EnergyTanks].mAmount) + kBaseHealthCapacity;
}

void CPlayerState::SetPickup(const CPlayerState::EItemType type, const int amount) {
  mPowerups[uint(type)].mAmount = 0;
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
      mPowerups[type].Add(amount);
      break;
    }
    case kIT_HealthRefill: {
      CHealthInfo* info = &mHealth;
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
    mPowerups[type].Dec(amount);
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
    return mPowerups[uint(type)].mAmount;
  default:
    break;
  }

  return 0;
}

const int CPlayerState::GetItemCapacity(const CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return mPowerups[uint(type)].mCapacity;
}

const bool CPlayerState::HasPowerUp(const CPlayerState::EItemType type) const {
  if (type < 0 || kIT_Max - 1 < type) {
    return false;
  }
  return mPowerups[uint(type)].mCapacity > 0;
}

const uint CPlayerState::GetPowerUp(const CPlayerState::EItemType type) {
  if (type < 0 || kIT_Max - 1 < type) {
    return 0;
  }
  return mPowerups[uint(type)].mCapacity;
}

void CPlayerState::EnableItem(const CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    mEnabledItems |= (1 << uint(type));
}

void CPlayerState::DisableItem(const CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    mEnabledItems &= ~(1 << uint(type));
}

const bool CPlayerState::ItemEnabled(const CPlayerState::EItemType type) const {
  if (HasPowerUp(type))
    return (mEnabledItems & (1 << uint(type)));
  return false;
}

void CPlayerState::ResetVisor() {
  mCurrentVisor = mTransitioningVisor = kPV_Combat;
  mVisorTransitionFactor = 0.0f;
}

void CPlayerState::StartTransitionToVisor(const CPlayerState::EPlayerVisor visor) {
  if (visor == mTransitioningVisor)
    return;

  mTransitioningVisor = visor;

  if (mTransitioningVisor == mCurrentVisor)
    return;
}

void CPlayerState::UpdateVisorTransition(float dt) {
  if (!GetIsVisorTransitioning())
    return;

  if (mCurrentVisor == mTransitioningVisor) {
    mVisorTransitionFactor = rstl::min_val(kMaxVisorTransitionFactor, mVisorTransitionFactor + dt);
  } else {
    mVisorTransitionFactor -= dt;
    if (mVisorTransitionFactor < 0.f) {
      mCurrentVisor = mTransitioningVisor;
      mVisorTransitionFactor = fabs(mVisorTransitionFactor);
      mVisorTransitionFactor =
          rstl::min_val(mVisorTransitionFactor, kMaxVisorTransitionFactor - FLT_EPSILON);
    }
  }
}

float CPlayerState::GetVisorTransitionFactor() const {
  return mVisorTransitionFactor / kMaxVisorTransitionFactor;
}

bool CPlayerState::GetIsVisorTransitioning() const {
  return mCurrentVisor != mTransitioningVisor ||
         kMaxVisorTransitionFactor > mVisorTransitionFactor;
}

float CPlayerState::GetBaseHealthCapacity() { return kBaseHealthCapacity; }

float CPlayerState::GetEnergyTankCapacity() { return kEnergyTankCapacity; }

void CPlayerState::InitializeScanTimes() {
  if (mScanTimes.size())
    return;

  const rstl::vector< CMemoryCard::ScanState >& scanStates = gpMemoryCard->GetScanStates();
  mScanTimes.reserve(scanStates.size());
  for (rstl::vector< CMemoryCard::ScanState >::const_iterator it = scanStates.begin();
       it != scanStates.end(); ++it) {
    mScanTimes.push_back(rstl::pair< CAssetId, float >(it->first, 0.f));
  }
}

const float CPlayerState::GetScanTime(const CAssetId res) const {
  rstl::vector< rstl::pair< CAssetId, float > >::const_iterator it =
      rstl::find_by_key(mScanTimes, res);
  return it->second;
}

void CPlayerState::SetScanTime(const CAssetId res, float time) {
  rstl::vector< rstl::pair< CAssetId, float > >::iterator it =
      rstl::find_by_key_nc(mScanTimes, res);
  it->second = time;
}

void CPlayerState::UpdateStaticInterference(CStateManager& stateMgr, const float& dt) {
  mStaticIntf.Update(stateMgr, dt);
}

CPlayerState::EPlayerVisor CPlayerState::GetActiveVisor(const CStateManager& stateMgr) const {
  const CFirstPersonCamera* cam = TCastToConstPtr< CFirstPersonCamera >(
      stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr));
  return (cam ? mCurrentVisor : kPV_Combat);
}

bool CPlayerState::CanVisorSeeFog(const CStateManager& stateMgr) const {
  EPlayerVisor activeVisor = GetActiveVisor(stateMgr);
  return (activeVisor == kPV_Combat || activeVisor == kPV_Scan);
}

CPlayerState::EPlayerSuit CPlayerState::GetCurrentSuit() const {
  if (GetIsFusionEnabled())
    return kPS_FusionPower;

  return mCurrentSuit;
}

bool CPlayerState::GetIsFusionEnabled() const { return mFusion || false; }

void CPlayerState::SetIsFusionEnabled(bool val) { mFusion = val; }

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
  return kMissileCosts[size_t(mCurrentBeam)];
}

float CPlayerState::GetComboFireAmmoPeriod() const {
  return kComboAmmoPeriods[size_t(mCurrentBeam)];
}

float CPlayerState::GetMissileComboChargeFactor() { return 1.8f; }
