#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/CRandom16.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

const CKnockBackMgr::KnockBackParms CKnockBackMgr::skDefaultParameters = {
    kAR_None,
    kKBAFU_None,
    0.f,
    0.f,
};

const CKnockBackMgr::KnockBackParms CKnockBackMgr::skKnockBackParameters[3][19][4] = {
    {
        // Small
        {
            {kAR_KnockBack, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_KnockBack, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 1.f, 0.f},
        },
        {
            {kAR_Hurled, kKBAFU_Shock, 2.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 2.f, 0.f},
        },
        {
            {kAR_None, kKBAFU_Shock, 4.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 4.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 0.f, 0.f},
        },
        {
            {kAR_Hurled, kKBAFU_Shock, 4.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_Shock, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 5.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_Freeze, 5.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 8.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 4.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_KnockBack, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_ExplodeDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
    },
    {
        // Medium
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
        },
        {
            {kAR_KnockBack, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 2.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 1.f, 0.f},
        },
        {
            {kAR_KnockBack, kKBAFU_Shock, 2.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 2.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 2.f, 0.f},
        },
        {
            {kAR_None, kKBAFU_Shock, 4.f, -1.f},
            {kAR_Hurled, kKBAFU_Shock, 4.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 0.f, 0.f},
        },
        {
            {kAR_Hurled, kKBAFU_Shock, 4.f, -1.f},
            {kAR_Hurled, kKBAFU_Shock, 4.f, -1.f},
            {kAR_Hurled, kKBAFU_Shock, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, -1.f},
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 5.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_Freeze, 5.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 8.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_Freeze, 8.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 4.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_None, kKBAFU_Freeze, 4.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_KnockBack, kKBAFU_Burn, 6.f, -1.f},
            {kAR_Fall, kKBAFU_BurnDeath, 6.f, -1.f},
            {kAR_KnockBack, kKBAFU_Burn, 6.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_KnockBack, kKBAFU_BurnDeath, 6.f, -1.f},
            {kAR_Fall, kKBAFU_BurnDeath, 6.f, -1.f},
            {kAR_KnockBack, kKBAFU_Burn, 6.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Burn, 6.f, -1.f},
            {kAR_Fall, kKBAFU_BurnDeath, 0.f, -1.f},
            {kAR_KnockBack, kKBAFU_Burn, 6.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_KnockBack, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_KnockBack, kKBAFU_IceDeath, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_KnockBack, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_LaggedBurnDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_LaggedBurnDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_PhazeOut, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
    },
    {
        // Large
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
        },
        {
            {kAR_KnockBack, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_KnockBack, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 2.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 1.f, 0.f},
        },
        {
            {kAR_Flinch, kKBAFU_Shock, 2.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 2.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 2.f, 0.f},
        },
        {
            {kAR_None, kKBAFU_Shock, 4.f, -1.f},
            {kAR_Hurled, kKBAFU_Shock, 4.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_Shock, 4.f, 0.f},
        },
        {
            {kAR_Hurled, kKBAFU_Shock, 4.f, -1.f},
            {kAR_Hurled, kKBAFU_Shock, 4.f, -1.f},
            {kAR_Hurled, kKBAFU_Shock, 4.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 4.5f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
            {kAR_None, kKBAFU_Freeze, 4.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, 0.f},
            {kAR_None, kKBAFU_Freeze, 2.5f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_Burn, 6.f, -1.f},
            {kAR_Fall, kKBAFU_Burn, 6.f, -1.f},
            {kAR_KnockBack, kKBAFU_Burn, 6.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_KnockBack, kKBAFU_Burn, 6.f, -1.f},
            {kAR_Fall, kKBAFU_Burn, 6.f, -1.f},
            {kAR_KnockBack, kKBAFU_Burn, 6.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_None, kKBAFU_Burn, 6.f, -1.f},
            {kAR_Fall, kKBAFU_BurnDeath, 0.f, -1.f},
            {kAR_KnockBack, kKBAFU_Burn, 6.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_None, 0.f, -1.f},
            {kAR_KnockBack, kKBAFU_IceDeath, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_None, 0.f, -1.f},
            {kAR_None, kKBAFU_None, 0.f, 0.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Hurled, kKBAFU_LaggedBurnDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_LaggedBurnDeath, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
        {
            {kAR_Flinch, kKBAFU_None, 0.f, -1.f},
            {kAR_Fall, kKBAFU_PhazeOut, 0.f, -1.f},
            {kAR_Hurled, kKBAFU_IceDeath, 0.f, -1.f},
            {kAR_Fall, kKBAFU_IceDeath, 0.f, -1.f},
        },
    },
};

static const float skImpulseDurations[2] = {0.1f, 0.3f};

CKnockBackMgr::CKnockBackMgr(const ECreatureSize size)
: mSize(size)
, mActiveParms(skDefaultParameters)
, mDeferWeaponType(kWT_None)
, mMinAnimState(kAR_None)
, mMaxAnimState(kAR_Fall)
, mImpulseDurationIdx(0)
, mImpulseDir(CVector3f::Zero())
, mImpulseMag(0.f)
, mImpulseRemTime(0.f)
, mFlinchRemTime(0.f)
, mDeferRemTime(0.f)
, x6c_(0)
, x70_(0)
, x74_(0)
, mSeverity(pas::kS_One)
, mAvailableStates(0)
, mAutoResetImpulse(true)
, mEnableFreeze(true)
, mEnableShock(false)
, mEnableBurn(true)
, mEnableBurnDeath(true)
, mEnableExplodeDeath(true)
, mEnableLaggedBurnDeath(true)
, x81_31_(true)
, x82_24_(true)
, mInDeferredKnockBack(false)
, mLocomotionDuringElectrocution(false) {
  float f = FLT_MAX;
  for (int i = 0; i <= kAR_Fall; i++) {
    x24_.push_back(rstl::pair< float, float >(0.f, f));
    EnableAnimReaction(static_cast< EAnimReaction >(i), true);
  }
}

void CKnockBackMgr::EnableAnimReaction(EAnimReaction state, bool enabled) {
  const uchar mask = 1 << state;
  if (enabled) {
    mAvailableStates |= mask;
  } else {
    mAvailableStates &= ~mask;
  }
}

bool CKnockBackMgr::TestAvailableState(int state) const {
  const uchar mask = 1 << state;
  return (mAvailableStates & mask) != 0;
}

void CKnockBackMgr::SetAnimationStateRange(EAnimReaction min, EAnimReaction max) {
  mMinAnimState = min;
  mMaxAnimState = max;
}

void CKnockBackMgr::SetImpulseDurationIdx(int index) { mImpulseDurationIdx = index; }

void CKnockBackMgr::SetAutoResetImpulse(bool enabled) {
  mAutoResetImpulse = enabled;
  if (!mAutoResetImpulse) {
    mImpulseMag = 0.f;
    mImpulseRemTime = 0.f;
  }
}

void CKnockBackMgr::sub80233d40(int index, float min, float max) {
  if (index >= 0 && index <= 4) {
    x24_[index] = rstl::pair< float, float >(min, max);
  }
}

void CKnockBackMgr::DeferKnockBack(EWeaponType type) {
  mDeferWeaponType = type;
  mDeferRemTime = 0.05f;
}

void CKnockBackMgr::SetCreatureSize(ECreatureSize size) { mSize = size; }

void CKnockBackMgr::KnockBack(const CVector3f& backVec, CStateManager& mgr, CPatterned& parent,
                              const CDamageInfo& info, float magnitude, bool direct) {
  if (mInDeferredKnockBack) {
    return;
  }
  CVector3f vec(backVec.DropZ(), 0.f);
  if (!vec.IsMagnitudeSafe()) {
    vec = -parent.GetTransform().GetForward();
  }
  SelectDamageState(parent, info, info.GetWeaponMode().GetType(), direct);
  DoKnockBackAnimation(vec, mgr, parent, magnitude);
  ResetKnockBackImpulse(parent, vec, 2.f);
}

void CKnockBackMgr::Update(float dt, CStateManager& mgr, CPatterned& parent) {
  ApplyImpulse(dt, parent);
  mFlinchRemTime -= dt;
  if (TickDeferredTimer(dt)) {
    DoDeferredKnockBack(mgr, parent);
  }
  if (mLocomotionDuringElectrocution && parent.BodyCtrl()->IsElectrocuting()) {
    parent.BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_Locomotion));
  }
}

void CKnockBackMgr::SelectDamageState(CPatterned& parent, const CDamageInfo& info,
                                      EWeaponType wType, bool direct) {
  mActiveParms = skDefaultParameters;
  const EKnockBackWeaponType weapon = GetKnockBackWeaponType(info, wType, direct);
  if (weapon != kKBWT_Invalid) {
    const EKnockBackCharacterState state = GetKnockBackCharacterState(parent);
    mActiveParms = skKnockBackParameters[mSize][weapon][state];
    ValidateState(parent);
  }
}

EKnockBackWeaponType CKnockBackMgr::GetKnockBackWeaponType(const CDamageInfo& info,
                                                           EWeaponType wType, bool direct) {
  EKnockBackWeaponType result = kKBWT_Invalid;
  int stacking = 0;
  if (info.GetWeaponMode().IsCharged()) {
    stacking = 1;
  } else if (info.GetWeaponMode().IsComboed()) {
    stacking = 2;
  }
  switch (wType) {
  case kWT_Power:
    result = static_cast< EKnockBackWeaponType >(stacking);
    if (!direct) {
      result = static_cast< EKnockBackWeaponType >(result + 1);
    }
    break;
  case kWT_Ice:
    result = static_cast< EKnockBackWeaponType >(stacking + 8);
    if (!direct) {
      result = static_cast< EKnockBackWeaponType >(result + 1);
    }
    break;
  case kWT_Wave:
    result = static_cast< EKnockBackWeaponType >(stacking + 4);
    break;
  case kWT_Plasma:
    result = static_cast< EKnockBackWeaponType >(stacking + 12);
    break;
  case kWT_Bomb:
    result = kKBWT_Bomb;
    break;
  case kWT_PowerBomb:
    result = kKBWT_PowerBomb;
    break;
  case kWT_Missile:
    result = kKBWT_Missile;
    break;
  case kWT_Phazon:
    result = kKBWT_Phazon;
    break;
  default:
    break;
  }
  return result;
}

EKnockBackCharacterState CKnockBackMgr::GetKnockBackCharacterState(CPatterned& parent) {
  if (parent.BodyCtrl()->IsFrozen()) {
    return parent.IsAlive() ? kKBCS_FrozenAlive : kKBCS_FrozenDead;
  }
  return parent.IsAlive() ? kKBCS_Alive : kKBCS_Dead;
}

float CKnockBackMgr::CalculateExtraHurlVelocity(CStateManager& mgr, float magnitude,
                                                float kbResistance) {
  float velocity = 0.f;
  if (magnitude > kbResistance) {
    const float randomFactor = 1.1f - 0.2f * mgr.Random()->Float();
    velocity = 2.f * randomFactor * (magnitude - kbResistance);
  }
  return velocity;
}

void CKnockBackMgr::ValidateState(CPatterned& parent) {
  if (mActiveParms.mAnimState < mMinAnimState) {
    mActiveParms.mAnimState = mMinAnimState;
  } else if (mActiveParms.mAnimState > mMaxAnimState) {
    mActiveParms.mAnimState = mMaxAnimState;
  }
  EAnimReaction state = kAR_Invalid;
  CBodyController& body = *parent.BodyCtrl();
  if (parent.IsAlive()) {
    if (body.HasBodyState(pas::kAS_Hurled) && TestAvailableState(kAR_Hurled) &&
        mActiveParms.mAnimState >= kAR_Hurled) {
      state = kAR_Hurled;
    } else if (body.HasBodyState(pas::kAS_KnockBack) && TestAvailableState(kAR_KnockBack) &&
               mActiveParms.mAnimState >= kAR_KnockBack) {
      state = kAR_KnockBack;
    } else if (body.HasBodyState(pas::kAS_AdditiveFlinch) && TestAvailableState(kAR_Flinch) &&
               mActiveParms.mAnimState >= kAR_Flinch) {
      state = kAR_Flinch;
    }
  } else if (body.HasBodyState(pas::kAS_Fall) && TestAvailableState(kAR_Fall) &&
             (mActiveParms.mAnimState >= kAR_Fall ||
              (!body.HasBodyState(pas::kAS_Hurled) && mActiveParms.mAnimState >= kAR_Hurled))) {
    state = kAR_Fall;
  } else if (body.HasBodyState(pas::kAS_Hurled) && TestAvailableState(kAR_Hurled) &&
             mActiveParms.mAnimState >= kAR_Hurled) {
    state = kAR_Hurled;
  }
  mActiveParms.mAnimState = state != kAR_Invalid ? state : kAR_None;
  bool disableFollowup = false;
  switch (mActiveParms.mAnimFollowup) {
  case kKBAFU_Freeze:
    disableFollowup = !mEnableFreeze;
    break;
  case kKBAFU_Shock:
    disableFollowup = !mEnableShock;
    break;
  case kKBAFU_Burn:
    disableFollowup = !mEnableBurn;
    break;
  case kKBAFU_ExplodeDeath:
    disableFollowup = !mEnableExplodeDeath;
    break;
  case kKBAFU_IceDeath:
    disableFollowup = !mEnableExplodeDeath;
    break;
  case kKBAFU_BurnDeath:
    disableFollowup = !mEnableBurnDeath;
    break;
  case kKBAFU_LaggedBurnDeath:
    disableFollowup = !mEnableLaggedBurnDeath;
    break;
  default:
    break;
  }
  if (disableFollowup) {
    mActiveParms.mAnimFollowup = kKBAFU_None;
    mActiveParms.mFollowupDuration = 0.f;
  }
}

bool CKnockBackMgr::TickDeferredTimer(float dt) {
  mDeferRemTime -= dt;
  if (mDeferWeaponType != kWT_None) {
    return mDeferRemTime <= 0.f;
  }
  return false;
}

void CKnockBackMgr::ResetKnockBackImpulse(CPatterned& parent, const CVector3f& backVec,
                                          float magnitude) {
  if (mAutoResetImpulse && mActiveParms.mAnimState == kAR_KnockBack &&
      mActiveParms.mAnimFollowup != kKBAFU_Freeze) {
    mImpulseDir =
        backVec.CanBeNormalized() ? backVec.AsNormalized() : -parent.GetTransform().GetForward();
    if (mImpulseRemTime <= 0.f) {
      mImpulseMag = magnitude;
    } else {
      mImpulseMag +=
          magnitude * (1.f - mImpulseRemTime / skImpulseDurations[mImpulseDurationIdx]);
    }
    mImpulseRemTime = skImpulseDurations[mImpulseDurationIdx];
  }
}

void CKnockBackMgr::ApplyImpulse(float dt, CPatterned& parent) {
  mImpulseRemTime = CMath::Max(0.f, mImpulseRemTime - dt);
  if (!parent.GetMaterialList().HasMaterial(kMT_Immovable) && mImpulseRemTime > 0.f) {
    float remFac = 1.f;
    switch (mImpulseDurationIdx) {
    case 1:
      remFac = mImpulseRemTime / skImpulseDurations[mImpulseDurationIdx];
      break;
    }
    const CVector3f velocity = remFac * mImpulseMag * mImpulseDir;
    const CVector3f displacement = dt * velocity;
    const CVector3f movement = displacement / skImpulseDurations[mImpulseDurationIdx];
    CVector3f impulse =
        parent.GetMoveToORImpulseWR(parent.GetTransform().TransposeRotate(movement), dt);
    parent.ApplyImpulseWR(impulse, CAxisAngle::Identity());
  }
}

void CKnockBackMgr::DoDeferredKnockBack(CStateManager& mgr, CPatterned& parent) {
  if (mDeferWeaponType == kWT_Wave) {
    const EKnockBackCharacterState state = GetKnockBackCharacterState(parent);
    mActiveParms = skKnockBackParameters[mSize][kKBWT_WaveComboedDirect][state];
    ValidateState(parent);
    if (parent.HealthInfo(mgr)) {
      const CVector3f backVec = -parent.GetTransform().GetForward();
      DoKnockBackAnimation(backVec, mgr, parent, 10.f);
      ResetKnockBackImpulse(parent, backVec, 2.f);
      CDamageInfo info(CWeaponMode(mDeferWeaponType, false, true, false), 0.f, 0.f, 10.f);
      mInDeferredKnockBack = true;
      parent.KnockBack(backVec, mgr, info, 10.f, true, mInDeferredKnockBack);
      mInDeferredKnockBack = false;
    }
  }
  mDeferRemTime = 0.f;
  mActiveParms = skDefaultParameters;
  mDeferWeaponType = kWT_None;
}

void CKnockBackMgr::DoKnockBackAnimation(const CVector3f& backVec, CStateManager& mgr,
                                         CPatterned& parent, float magnitude) {
  CBodyController& body = *parent.BodyCtrl();
  switch (mActiveParms.mAnimState) {
  case kAR_Hurled: {
    float hurlVelocity = 5.f;
    if (CHealthInfo* health = parent.HealthInfo(mgr)) {
      hurlVelocity += CalculateExtraHurlVelocity(mgr, magnitude, health->GetKnockBackResistance());
    }
    hurlVelocity = CMath::SqrtF(0.5f * hurlVelocity * parent.GetGravityConstant());
    const CVector3f backUpVec = backVec + backVec.Magnitude() * CVector3f::Up();
    if (backUpVec.CanBeNormalized()) {
      body.CommandMgr().DeliverCmd(CBCHurledCmd(-backVec, hurlVelocity * backUpVec.AsNormalized()));
      parent.SetMomentumWR(CVector3f(0.f, 0.f, -parent.GetMass() * parent.GetGravityConstant()));
    }
    break;
  }
  case kAR_Fall:
    body.CommandMgr().DeliverCmd(CBCKnockDownCmd(-backVec, mSeverity));
    break;
  case kAR_KnockBack:
    body.CommandMgr().DeliverCmd(CBCKnockBackCmd(-backVec, mSeverity));
    break;
  case kAR_Flinch: {
    const CPASDatabase& db = body.GetPASDatabase();
    const CPASAnimParmData parms(pas::kAS_AdditiveFlinch);
    const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.first > 0.f) {
      parent.ModelData()->AnimationData()->AddAdditiveAnimation(best.second, 1.f, false, true);
      mFlinchRemTime =
          rstl::max_val(mFlinchRemTime,
                        parent.ModelData()->AnimationData()->GetAnimationDuration(best.second));
    }
    break;
  }
  default:
    break;
  }
}
