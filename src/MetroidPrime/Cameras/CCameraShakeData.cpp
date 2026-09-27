#include "MetroidPrime/Cameras/CCameraShakeData.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include <math.h>

extern uint LoadParameterFlags(CInputStream& in);

static const SCameraShakePoint skRecoilAM(0, 0.f, 0.f, 0.3f, -1.f);
static const SCameraShakePoint skHeavyRecoilAM(0, 0.f, 0.f, 0.3f, -2.f);
static const SCameraShakePoint skRecoilFM(1, 0.f, 0.f, 0.05f, 0.3f);
static const SCameraShakePoint skHeavyRecoilFM(1, 0.f, 0.f, 0.05f, 0.5f);

CCameraShakeData CCameraShakeData::skSoftRecoil(0.3f, 100.f, 0, CVector3f::Zero(),
                                                CCameraShakerComponent::NoMotion(),
                                                CCameraShakerComponent(1, skRecoilAM, skRecoilFM),
                                                CCameraShakerComponent::NoMotion());

CCameraShakeData CCameraShakeData::skHardRecoil(0.3f, 100.f, 0, CVector3f::Zero(),
                                                CCameraShakerComponent::NoMotion(),
                                                CCameraShakerComponent(1, skHeavyRecoilAM,
                                                                       skHeavyRecoilFM),
                                                CCameraShakerComponent::NoMotion());

CCameraShakeData CCameraShakeData::SoftHorizShake(float duration, float magnitude) {
  const SCameraShakePoint amX(0, 0.f, 0.f, duration, magnitude);
  const SCameraShakePoint fmX(1, 0.f, 0.f, 0.5f * duration, 3.f);
  const CCameraShakerComponent shakerX(1, amX, fmX);

  return CCameraShakeData(duration, 100.f, 0, CVector3f::Zero(), shakerX,
                          CCameraShakerComponent::NoMotion(), CCameraShakerComponent::NoMotion());
}

CCameraShakeData CCameraShakeData::SoftBothAxesShake(float duration, float magnitude) {
  const SCameraShakePoint amX(0, 0.15f * duration, 0.f, 0.25f * duration, magnitude);
  const SCameraShakePoint fmX(1, 0.f, 0.f, 0.4f * duration, 0.3f);
  const CCameraShakerComponent shakerX(1, amX, fmX);
  const SCameraShakePoint amZ(0, 0.25f * duration, 0.f, 0.25f * duration, magnitude);
  const SCameraShakePoint fmZ(1, 0.f, 0.f, 0.5f * duration, 0.5f);
  const CCameraShakerComponent shakerZ(1, amZ, fmZ);

  return CCameraShakeData(duration, 100.f, 0, CVector3f::Zero(), shakerX,
                          CCameraShakerComponent::NoMotion(), shakerZ);
}

CCameraShakeData CCameraShakeData::HardHorizShake(float duration, float magnitude) {
  const SCameraShakePoint amX(0, 0.25f * duration, 0.f, 0.75f * duration, magnitude);
  const SCameraShakePoint fmX(1, 0.f, 0.f, 0.5f * duration, 2.f);
  const CCameraShakerComponent shakerX(1, amX, fmX);

  return CCameraShakeData(duration, 100.f, 0, CVector3f::Zero(), shakerX,
                          CCameraShakerComponent::NoMotion(), CCameraShakerComponent::NoMotion());
}

CCameraShakeData CCameraShakeData::HardVertShake(float duration, float magnitude) {
  const SCameraShakePoint amZ(0, 0.25f * duration, 0.f, 0.75f * duration, magnitude);
  const SCameraShakePoint fmZ(1, 0.f, 0.f, 0.5f * duration, 2.f);
  const CCameraShakerComponent shakerZ(1, amZ, fmZ);

  return CCameraShakeData(duration, 100.f, 0, CVector3f::Zero(), CCameraShakerComponent::NoMotion(),
                          CCameraShakerComponent::NoMotion(), shakerZ);
}

CCameraShakeData CCameraShakeData::HardBothAxesShake(float duration, float magnitude) {
  const SCameraShakePoint amX(0, 0.15f * duration, 0.f, 0.85f * duration, magnitude);
  const SCameraShakePoint fmX(1, 0.f, 0.f, 0.4f * duration, 1.5f);
  const CCameraShakerComponent shakerX(1, amX, fmX);
  const SCameraShakePoint amZ(0, 0.25f * duration, 0.f, 0.75f * duration, magnitude);
  const SCameraShakePoint fmZ(1, 0.f, 0.f, 0.5f * duration, 2.f);
  const CCameraShakerComponent shakerZ(1, amZ, fmZ);

  return CCameraShakeData(duration, 100.f, 0, CVector3f::Zero(), shakerX,
                          CCameraShakerComponent::NoMotion(), shakerZ);
}

CCameraShakeData CCameraShakeData::HardHorizShakeDistance(CVector3f pos, float duration,
                                                          float magnitude, float maxDist) {
  CCameraShakeData data = HardHorizShake(duration, magnitude);
  data.SetSfxPositionAndDistance(maxDist, pos);
  return data;
}

CCameraShakeData CCameraShakeData::HardVertShakeDistance(float duration, float magnitude,
                                                         float distance, CVector3f origin) {
  CCameraShakeData data = HardVertShake(duration, magnitude);
  data.SetSfxPositionAndDistance(distance, origin);
  return data;
}

SCameraShakePoint::SCameraShakePoint(int flags, float attackTime, float sustainTime, float duration,
                                     float magnitude)
: mUseEnvelope(flags)
, mValue(0.f)
, mMagnitude(magnitude)
, mAttackTime(attackTime)
, mSustainTime(sustainTime)
, mDuration(duration) {}

SCameraShakePoint LoadCameraShakePoint(CInputStream& in) {
  const uint flags = LoadParameterFlags(in);
  const float attackTime = in.ReadFloat();
  const float sustainTime = in.ReadFloat();
  const float duration = in.ReadFloat();
  return SCameraShakePoint(flags, attackTime, sustainTime, duration, in.ReadFloat());
}

SCameraShakePoint SCameraShakePoint::NoMotion() { return SCameraShakePoint(0, 0.f, 0.f, 0.f, 0.f); }

void SCameraShakePoint::Update(float curTime) {
  const float offTime = mAttackTime + mSustainTime;
  float factor = 1.f;
  if (curTime < mAttackTime && mAttackTime > 0.f) {
    factor = CMath::Clamp(0.f, curTime / mAttackTime, 1.f);
  }
  if (curTime >= offTime && mDuration > 0.f) {
    factor = 1.f - CMath::Clamp(0.f, (curTime - offTime) / mDuration, 1.f);
  }
  mValue = mMagnitude * factor;
}

float SCameraShakePoint::GetValue() const {
  if (mUseEnvelope & 1) {
    return mMagnitude;
  }
  return mValue;
}

CCameraShakerComponent::CCameraShakerComponent(int flags, const SCameraShakePoint& am,
                                               const SCameraShakePoint& fm)
: mUseModulation(flags), mAm(am), mFm(fm), mValue(0.f) {}

CCameraShakerComponent LoadNewCameraShakerComponent(CInputStream& in) {
  const uint flags = LoadParameterFlags(in);
  const SCameraShakePoint am = LoadCameraShakePoint(in);
  const SCameraShakePoint fm = LoadCameraShakePoint(in);
  return CCameraShakerComponent(flags, am, fm);
}

CCameraShakerComponent CCameraShakerComponent::NoMotion() {
  return CCameraShakerComponent(0, SCameraShakePoint::NoMotion(), SCameraShakePoint::NoMotion());
}

void CCameraShakerComponent::UpdateMotion(float curTime, float duration, float distAtt) {
  if (close_enough(duration, 0.f) || !(mUseModulation & 1)) {
    mValue = 0.f;
    return;
  }

  mFm.Update(curTime);
  const float freq = 1.f + mFm.GetValue();
  mAm.Update(curTime);
  mValue = mAm.GetValue() * sinf(2.f * M_PIF * (duration - curTime) * freq);
  mValue *= distAtt;
}

CCameraShakeData::CCameraShakeData(float duration, float sfxDist, int flags,
                                   const CVector3f& sfxPos, const CCameraShakerComponent& shakerX,
                                   const CCameraShakerComponent& shakerY,
                                   const CCameraShakerComponent& shakerZ)
: mDuration(duration)
, mCurTime(0.f)
, mShakerX(shakerX)
, mShakerY(shakerY)
, mShakerZ(shakerZ)
, mShakerId(0)
, mFlags(flags)
, mSfxPos(sfxPos)
, mSfxDist(sfxDist) {}

void CCameraShakeData::SetSfxPositionAndDistance(float distance, CVector3f pos) {
  mFlags |= 1;
  mSfxDist = distance;
  mSfxPos = pos;
}

void CCameraShakeData::Update(float dt, CStateManager& mgr) {
  mCurTime += dt;
  float distAtt = 1.f;
  if (mFlags & 1) {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    distAtt =
        1.f - CMath::Clamp(0.f, (CVector3f(mSfxPos - playerPos)).Magnitude() / mSfxDist, 1.f);
  }
  mShakerX.UpdateMotion(mCurTime, mDuration, distAtt);
  mShakerY.UpdateMotion(mCurTime, mDuration, distAtt);
  mShakerZ.UpdateMotion(mCurTime, mDuration, distAtt);
}

CVector3f CCameraShakeData::GetPoint() const {
  const float x = mShakerX.GetValue();
  const float y = mShakerY.GetValue();
  const float z = mShakerZ.GetValue();
  return CVector3f(x, y, z);
}

CCameraShakeData LoadCameraShakeData(CInputStream& in) {
  const float xMag = in.Get< float >();
  in.ReadFloat();
  const float yMag = in.Get< float >();
  in.ReadFloat();
  const float zMag = in.Get< float >();
  in.ReadFloat();
  const float duration = in.Get< float >();

  const SCameraShakePoint xAM(0, 0.f, 0.f, duration, 2.f * xMag);
  const SCameraShakePoint yAM(0, 0.f, 0.f, duration, 2.f * yMag);
  const SCameraShakePoint zAM(0, 0.f, 0.f, duration, 2.f * zMag);
  const SCameraShakePoint xFM(0, 0.f, 0.f, 0.5f * duration, 3.f);
  const SCameraShakePoint yFM(0, 0.f, 0.f, 0.5f * duration, 0.f);
  const SCameraShakePoint zFM(0, 0.f, 0.f, 0.5f * duration, 3.f);

  const CCameraShakerComponent shakerX(1, xAM, xFM);
  const CCameraShakerComponent shakerY = CCameraShakerComponent::NoMotion();
  const CCameraShakerComponent shakerZ(1, zAM, zFM);
  return CCameraShakeData(duration, 100.f, 0, CVector3f::Zero(), shakerX, shakerY, shakerZ);
}

CCameraShakeData CCameraShakeData::EatOldCameraShakerData(CInputStream& in) {
  in.ReadLong();
  in.ReadFloat();
  in.ReadFloat();
  in.ReadFloat();
  in.ReadFloat();
  in.ReadFloat();
  in.ReadFloat();
  in.ReadFloat();
  in.ReadBool();
  return SoftHorizShake(0.5f, 0.75f);
}

float CCameraShakeData::GetMaxAmplitude() const {
  float result = 0.f;
  if (mShakerX.IsModulated()) {
    result = mShakerX.GetAmplitude();
  }
  if (mShakerY.IsModulated() && mShakerY.GetAmplitude() > result) {
    result = mShakerY.GetAmplitude();
  }
  if (mShakerZ.IsModulated() && mShakerZ.GetAmplitude() > result) {
    result = mShakerZ.GetAmplitude();
  }
  return result;
}

float CCameraShakeData::GetMaxSeverity() const {
  float result = 0.f;
  if (mShakerX.IsModulated()) {
    result = mShakerX.GetSeverity();
  }
  if (mShakerY.IsModulated() && mShakerY.GetSeverity() > result) {
    result = mShakerY.GetSeverity();
  }
  if (mShakerZ.IsModulated() && mShakerZ.GetSeverity() > result) {
    result = mShakerZ.GetSeverity();
  }
  return result;
}
