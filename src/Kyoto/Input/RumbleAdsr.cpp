#include "Kyoto/Input/CRumbleVoice.hpp"

SAdsrDelta SAdsrDelta::Start(ERumblePriority priority, bool prePulse) {
  return SAdsrDelta(prePulse ? kP_PrePulse : kP_Attack, priority);
}
SAdsrDelta SAdsrDelta::Stopped() { return SAdsrDelta(kP_Stop); }

SAdsrDelta::SAdsrDelta(EPhase phase)
: mCurIntensity(0.f)
, mAttackTime(0.f)
, mDecayTime(0.f)
, mReleaseTime(0.f)
, mAutoReleaseTime(0.f)
, mPriority(kRP_None)
, mPhase(phase) {}

SAdsrDelta::SAdsrDelta(EPhase phase, ERumblePriority priority)
: mCurIntensity(phase == kP_PrePulse ? 2.f : 0.f)
, mAttackTime(0.f)
, mDecayTime(0.f)
, mReleaseTime(0.f)
, mAutoReleaseTime(0.f)
, mPriority(priority)
, mPhase(phase) {}

SAdsrData::SAdsrData()
: mAttackGain(0.f)
, mAutoReleaseDur(0.f)
, mAttackDur(0.f)
, mDecayDur(0.f)
, mSustainGain(0.f)
, mReleaseDur(0.f)
, mHasSustain(false)
, mAutoRelease(false) {}

SAdsrData::SAdsrData(float attackGain, float autoReleaseDur, float attackDur, float decayDur,
                     float sustainGain, float releaseDur, bool hasSustain, bool autoRelease)
: mAttackGain(attackGain)
, mAutoReleaseDur(autoReleaseDur)
, mAttackDur(attackDur)
, mDecayDur(decayDur)
, mSustainGain(sustainGain)
, mReleaseDur(releaseDur)
, mHasSustain(hasSustain)
, mAutoRelease(autoRelease) {}
