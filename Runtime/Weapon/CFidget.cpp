#include "CFidget.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"

namespace urde {

static float kMinorFidgetDelay = 20.f;
static float kMajorFidgetDelay = 20.f;

CFidget::EState CFidget::Update(int fireButtonStates, bool bobbing, bool inStrikeCooldown, float dt,
                                CStateManager& mgr) {
  switch (x0_state) {
  case EState::NoFidget:
    break;
  case EState::MinorFidget:
    return x34_24_loading ? EState::Loading : EState::StillMinorFidget;
  case EState::MajorFidget:
    return x34_24_loading ? EState::Loading : EState::StillMajorFidget;
  case EState::HolsterBeam:
    return x34_24_loading ? EState::Loading : EState::StillHolsterBeam;
  default:
    x0_state = EState::NoFidget;
    break;
  }

  if (fireButtonStates != 0) {
    x14_timeSinceFire = 0.f;
    x2c_holsterTimeSinceFire = 0.f;
  } else {
    if (x14_timeSinceFire < 6.f)
      x14_timeSinceFire += dt;
    if (x2c_holsterTimeSinceFire < x30_timeUntilHolster + 1.f)
      x2c_holsterTimeSinceFire += dt;
  }

  if (inStrikeCooldown)
    x18_timeSinceStrikeCooldown = 0.f;
  else if (x18_timeSinceStrikeCooldown < 11.f)
    x18_timeSinceStrikeCooldown += dt;

  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    if (x1c_timeSinceUnmorph < 21.f)
      x1c_timeSinceUnmorph += dt;
  } else {
    x1c_timeSinceUnmorph = 0.f;
  }

  if (bobbing)
    x20_timeSinceBobbing = 0.f;
  else if (x20_timeSinceBobbing < 21.f)
    x20_timeSinceBobbing += dt;

  u32 pendingTriggerBits = 0;
  if (x0_state == EState::NoFidget) {
    if ((x10_delayTimerEnableBits & 0x1) != 0) {
      x24_minorDelayTimer += dt;
      if (x24_minorDelayTimer > kMinorFidgetDelay) {
        pendingTriggerBits |= 0x1;
        x24_minorDelayTimer = 0.f;
      }
    }

    if ((x10_delayTimerEnableBits & 0x2) != 0) {
      x28_majorDelayTimer += dt;
      if (x28_majorDelayTimer > kMajorFidgetDelay) {
        pendingTriggerBits |= 0x2;
        x28_majorDelayTimer = 0.f;
      }
    }
  }

  if (x2c_holsterTimeSinceFire > x30_timeUntilHolster) {
    x0_state = EState::HolsterBeam;
  } else {
    if (x18_timeSinceStrikeCooldown > 10.f && x1c_timeSinceUnmorph > 20.f && x20_timeSinceBobbing > 20.f) {
      if ((pendingTriggerBits & 0x1) != 0)
        x8_delayTriggerBits |= 0x1;
      else if ((pendingTriggerBits & 0x2) != 0)
        x8_delayTriggerBits |= 0x2;
    }

    if ((x8_delayTriggerBits & 0x3) == 0x3)
      x0_state = (mgr.GetActiveRandom()->Next() % 100) >= 50 ? EState::MajorFidget : EState::MinorFidget;
    else if ((x8_delayTriggerBits & 0x1) == 0x1)
      x0_state = EState::MinorFidget;
    else if ((x8_delayTriggerBits & 0x2) == 0x2)
      x0_state = EState::MajorFidget;
    else
      x0_state = EState::NoFidget;
  }

  switch (x0_state) {
  case EState::MinorFidget:
    x34_24_loading = true;
    x10_delayTimerEnableBits = 2;
    x8_delayTriggerBits &= ~0x1;
    kMinorFidgetDelay = mgr.GetActiveRandom()->Range(20.f, 29.f);
    x4_type = SamusGun::EFidgetType::Minor;
    xc_animSet = mgr.GetActiveRandom()->Range(0, 4);
    break;
  case EState::MajorFidget:
    x34_24_loading = true;
    x10_delayTimerEnableBits = 1;
    x8_delayTriggerBits &= ~0x2;
    kMajorFidgetDelay = mgr.GetActiveRandom()->Range(20.f, 30.f);
    x4_type = SamusGun::EFidgetType::Major;
    xc_animSet = mgr.GetActiveRandom()->Range(0, 5);
    break;
  case EState::HolsterBeam:
    x4_type = SamusGun::EFidgetType::Minor;
    x34_24_loading = true;
    xc_animSet = 0;
    break;
  default:
    break;
  }

  return x0_state;
}

void CFidget::ResetMinor() { x0_state = EState::NoFidget; }

void CFidget::ResetAll() {
  x0_state = EState::NoFidget;
  x4_type = SamusGun::EFidgetType::Invalid;
  x18_timeSinceStrikeCooldown = 0.f;
  x1c_timeSinceUnmorph = 0.f;
  x14_timeSinceFire = 0.f;
  x24_minorDelayTimer = 0.f;
  x28_majorDelayTimer = 0.f;
  x2c_holsterTimeSinceFire = 0.f;
  x8_delayTriggerBits = 0;
  xc_animSet = -1;
  x10_delayTimerEnableBits = 3;
  x34_24_loading = false;
}

} // namespace urde
