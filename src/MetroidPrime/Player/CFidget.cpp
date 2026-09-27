#include "MetroidPrime/Player/CFidget.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

static float kMinorFidgetDelay = 20.f;
static float kMajorFidgetDelay = 20.f;

CFidget::CFidget()
: mState(kS_NoFidget)
, mType(SamusGun::kFT_Invalid)
, mDelayTriggerBits(0)
, mAnimSet(-1)
, mDelayTimerEnableBits(3)
, mTimeSinceFire(0.f)
, mTimeSinceStrikeCooldown(0.f)
, mTimeSinceUnmorph(0.f)
, mTimeSinceBobbing(0.f)
, mMinorDelayTimer(0.f)
, mMajorDelayTimer(0.f)
, mHolsterTimeSinceFire(0.f)
, mTimeUntilHolster(105.f)
, mLoading(false) {}

void CFidget::ResetAll() {
  mState = kS_NoFidget;
  mType = SamusGun::kFT_Invalid;
  mTimeSinceStrikeCooldown = 0.f;
  mTimeSinceUnmorph = 0.f;
  mTimeSinceFire = 0.f;
  mMinorDelayTimer = 0.f;
  mMajorDelayTimer = 0.f;
  mHolsterTimeSinceFire = 0.f;
  mDelayTriggerBits = 0;
  mAnimSet = -1;
  mDelayTimerEnableBits = 3;
  mLoading = false;
}

void CFidget::ResetMinor() { mState = kS_NoFidget; }

CFidget::EState CFidget::Update(int fireButtonStates, bool bobbing, bool inStrikeCooldown, float dt,
                                CStateManager& mgr) {
  if (mState) {
    switch (mState) {
    case kS_MinorFidget:
      return mLoading ? kS_Loading : kS_StillMinorFidget;
    case kS_MajorFidget:
      return mLoading ? kS_Loading : kS_StillMajorFidget;
    case kS_HolsterBeam:
      return mLoading ? kS_Loading : kS_StillHolsterBeam;
    default:
      mState = kS_NoFidget;
      break;
    }
  }

  if (fireButtonStates != 0) {
    mTimeSinceFire = 0.f;
    mHolsterTimeSinceFire = 0.f;
  } else {
    if (mTimeSinceFire < 6.f)
      mTimeSinceFire += dt;
    if (mHolsterTimeSinceFire < mTimeUntilHolster + 1.f)
      mHolsterTimeSinceFire += dt;
  }

  if (inStrikeCooldown)
    mTimeSinceStrikeCooldown = 0.f;
  else if (mTimeSinceStrikeCooldown < 11.f)
    mTimeSinceStrikeCooldown += dt;

  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    if (mTimeSinceUnmorph < 21.f)
      mTimeSinceUnmorph += dt;
  } else {
    mTimeSinceUnmorph = 0.f;
  }

  if (bobbing)
    mTimeSinceBobbing = 0.f;
  else if (mTimeSinceBobbing < 21.f)
    mTimeSinceBobbing += dt;

  u32 pendingTriggerBits = 0;
  if (mState == kS_NoFidget) {
    if ((mDelayTimerEnableBits & 0x1) != 0) {
      mMinorDelayTimer += dt;
      if (mMinorDelayTimer > kMinorFidgetDelay) {
        pendingTriggerBits |= 0x1;
        mMinorDelayTimer = 0.f;
      }
    }

    if ((mDelayTimerEnableBits & 0x2) != 0) {
      mMajorDelayTimer += dt;
      if (mMajorDelayTimer > kMajorFidgetDelay) {
        pendingTriggerBits |= 0x2;
        mMajorDelayTimer = 0.f;
      }
    }
  }

  if (mHolsterTimeSinceFire > mTimeUntilHolster) {
    mState = kS_HolsterBeam;
  } else {
    if (mTimeSinceStrikeCooldown > 10.f && mTimeSinceUnmorph > 20.f &&
        mTimeSinceBobbing > 20.f) {
      if ((pendingTriggerBits & 0x1) != 0)
        mDelayTriggerBits |= 0x1;
      else if ((pendingTriggerBits & 0x2) != 0)
        mDelayTriggerBits |= 0x2;
    }

    if ((mDelayTriggerBits & 0x3) == 0x3) {
      EState newState = kS_MinorFidget;
      mState = 50 < (mgr.Random()->Next() % 100) ? kS_MajorFidget : kS_MinorFidget;
    } else if ((mDelayTriggerBits & 0x1)) {
      mState = kS_MinorFidget;
    } else {
      if ((mDelayTriggerBits & 0x2)) {
        mState = kS_MajorFidget;
      } else {
        mState = kS_NoFidget;
      }
    }
  }

  switch (mState) {
  case kS_MinorFidget:
    mLoading = true;
    mDelayTimerEnableBits = 2;
    mDelayTriggerBits &= ~0x1;
    kMinorFidgetDelay = mgr.Random()->Range(20.f, 29.f);
    mType = SamusGun::kFT_Minor;
    mAnimSet = mgr.Random()->Range(0, 4);
    break;
  case kS_MajorFidget:
    mLoading = true;
    mDelayTimerEnableBits = 1;
    mDelayTriggerBits &= ~0x2;
    kMajorFidgetDelay = mgr.Random()->Range(20.f, 30.f);
    mType = SamusGun::kFT_Major;
    mAnimSet = mgr.Random()->Range(0, 5);
    break;
  case kS_HolsterBeam:
    mType = SamusGun::kFT_Minor;
    mLoading = true;
    mAnimSet = 0;
    break;
  default:
    break;
  }

  return mState;
}
