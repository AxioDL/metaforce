#include "CCameraShakeData.hpp"
#include "World/ScriptLoader.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"

namespace urde {

SCameraShakePoint SCameraShakePoint::LoadCameraShakePoint(CInputStream& in) {
  u32 useEnvelope = ScriptLoader::LoadParameterFlags(in);
  float attackTime = in.readFloatBig();
  float sustainTime = in.readFloatBig();
  float duration = in.readFloatBig();
  float magnitude = in.readFloatBig();
  return {useEnvelope != 0, attackTime, sustainTime, duration, magnitude};
}

CCameraShakerComponent CCameraShakerComponent::LoadNewCameraShakerComponent(CInputStream& in) {
  u32 useModulation = ScriptLoader::LoadParameterFlags(in);
  SCameraShakePoint am = SCameraShakePoint::LoadCameraShakePoint(in);
  SCameraShakePoint fm = SCameraShakePoint::LoadCameraShakePoint(in);
  return {useModulation != 0, am, fm};
}

CCameraShakeData::CCameraShakeData(float duration, float sfxDist, u32 flags, const zeus::CVector3f& sfxPos,
                                   const CCameraShakerComponent& shaker1, const CCameraShakerComponent& shaker2,
                                   const CCameraShakerComponent& shaker3)
: x0_duration(duration)
, x8_shakerX(shaker1)
, x44_shakerY(shaker2)
, x80_shakerZ(shaker3)
, xc0_flags(flags)
, xc4_sfxPos(sfxPos)
, xd0_sfxDist(sfxDist) {}

CCameraShakeData::CCameraShakeData(float duration, float magnitude)
: CCameraShakeData(duration, 100.f, 0, zeus::skZero3f, CCameraShakerComponent{}, CCameraShakerComponent{},
                   CCameraShakerComponent{1, SCameraShakePoint{0, 0.25f * duration, 0.f, 0.75f * duration, magnitude},
                                          SCameraShakePoint{1, 0.f, 0.f, 0.5f * duration, 2.f}}) {}

CCameraShakeData::CCameraShakeData(CInputStream& in) {
  in.readUint32Big();
  in.readFloatBig();
  in.readFloatBig();
  in.readFloatBig();
  in.readFloatBig();
  in.readFloatBig();
  in.readFloatBig();
  in.readFloatBig();
  in.readBool();
  BuildProjectileCameraShake(0.5f, 0.75f);
}

CCameraShakeData CCameraShakeData::BuildLandingCameraShakeData(float duration, float magnitude) {
  return {duration,
          100.f,
          0,
          zeus::skZero3f,
          CCameraShakerComponent(1, SCameraShakePoint(0, 0.15f * duration, 0.f, 0.85f * duration, magnitude),
                                 SCameraShakePoint(1, 0.f, 0.f, 0.4f * duration, 1.5f)),
          CCameraShakerComponent(),
          CCameraShakerComponent(1, SCameraShakePoint(0, 0.25f * duration, 0.f, 0.75f * duration, magnitude),
                                 SCameraShakePoint(1, 0.f, 0.f, 0.5f * duration, 2.f))};
}

CCameraShakeData CCameraShakeData::BuildProjectileCameraShake(float duration, float magnitude) {
  return {duration,
          100.f,
          0,
          zeus::skZero3f,
          CCameraShakerComponent(1, SCameraShakePoint(0, 0.f, 0.f, duration, magnitude),
                                 SCameraShakePoint(1, 0.f, 0.f, 0.5f * duration, 3.f)),
          CCameraShakerComponent(),
          CCameraShakerComponent()};
}

CCameraShakeData CCameraShakeData::BuildMissileCameraShake(float duration, float magnitude, float sfxDistance,
                                                           const zeus::CVector3f& sfxPos) {
  CCameraShakeData ret(duration, magnitude);
  ret.SetSfxPositionAndDistance(sfxPos, sfxDistance);
  return ret;
}

CCameraShakeData CCameraShakeData::BuildPhazonCameraShakeData(float duration, float magnitude) {
  return {duration,
          100.f,
          0,
          zeus::skZero3f,
          CCameraShakerComponent(1, SCameraShakePoint(0, 0.15f * duration, 0.f, 0.25f * duration, magnitude),
                                 SCameraShakePoint(1, 0.f, 0.f, 0.4f * duration, 0.3f)),
          CCameraShakerComponent(),
          CCameraShakerComponent(1, SCameraShakePoint(0, 0.25f * duration, 0.f, 0.25f * duration, magnitude),
                                 SCameraShakePoint(1, 0.f, 0.f, 0.5f * duration, 0.5f))};
}

CCameraShakeData CCameraShakeData::BuildPatternedExplodeShakeData(float duration, float magnitude) {
  return {duration,
          100.f,
          0,
          zeus::skZero3f,
          CCameraShakerComponent(1, SCameraShakePoint(0, 0.25f * duration, 0.f, 0.75f * duration, magnitude),
                                 SCameraShakePoint(1, 0.f, 0.f, 0.5f * duration, 2.0f)),
          CCameraShakerComponent(),
          CCameraShakerComponent()};
}

CCameraShakeData CCameraShakeData::BuildPatternedExplodeShakeData(const zeus::CVector3f& pos, float duration,
                                                                  float magnitude, float distance) {
  CCameraShakeData shakeData = CCameraShakeData::BuildPatternedExplodeShakeData(duration, magnitude);
  shakeData.SetSfxPositionAndDistance(pos, distance);
  return shakeData;
}

void SCameraShakePoint::Update(float curTime) {
  float offTimePoint = xc_attackTime + x10_sustainTime;
  float factor = 1.f;
  if (curTime < xc_attackTime && xc_attackTime > 0.f)
    factor = zeus::clamp(0.f, curTime / xc_attackTime, 1.f);
  if (curTime >= offTimePoint && x14_duration > 0.f)
    factor = 1.f - (curTime - offTimePoint) / x14_duration;
  x4_value = x8_magnitude * factor;
}

void CCameraShakerComponent::Update(float curTime, float duration, float distAtt) {
  if (std::fabs(duration) < 0.00001f || !x4_useModulation) {
    x38_value = 0.f;
    return;
  }

  x20_fm.Update(curTime);
  float freq = 1.f + x20_fm.GetValue();
  x8_am.Update(curTime);
  x38_value = x8_am.GetValue() * std::sin(2.f * M_PIF * (duration - curTime) * freq);
  x38_value *= distAtt;
}

void CCameraShakeData::Update(float dt, CStateManager& mgr) {
  x4_curTime += dt;
  float distAtt = 1.f;
  if (xc0_flags & 0x1)
    distAtt = 1.f - zeus::clamp(0.f, (xc4_sfxPos - mgr.GetPlayer().GetTranslation()).magnitude() / xd0_sfxDist, 1.f);
  x8_shakerX.Update(x4_curTime, x0_duration, distAtt);
  x44_shakerY.Update(x4_curTime, x0_duration, distAtt);
  x80_shakerZ.Update(x4_curTime, x0_duration, distAtt);
}

zeus::CVector3f CCameraShakeData::GetPoint() const {
  return {x8_shakerX.GetValue(), x44_shakerY.GetValue(), x80_shakerZ.GetValue()};
}

float CCameraShakeData::GetMaxAMComponent() const {
  float ret = 0.f;
  if (x8_shakerX.x4_useModulation)
    ret = x8_shakerX.x8_am.GetValue();
  if (x44_shakerY.x4_useModulation)
    ret = std::max(ret, x44_shakerY.x8_am.GetValue());
  if (x80_shakerZ.x4_useModulation)
    ret = std::max(ret, x80_shakerZ.x8_am.GetValue());
  return ret;
}

float CCameraShakeData::GetMaxFMComponent() const {
  float ret = 0.f;
  if (x8_shakerX.x4_useModulation)
    ret = x8_shakerX.x20_fm.GetValue();
  if (x44_shakerY.x4_useModulation)
    ret = std::max(ret, x44_shakerY.x20_fm.GetValue());
  if (x80_shakerZ.x4_useModulation)
    ret = std::max(ret, x80_shakerZ.x20_fm.GetValue());
  return ret;
}

CCameraShakeData CCameraShakeData::LoadCameraShakeData(CInputStream& in) {
  float xMag = in.readFloatBig();
  in.readFloatBig();
  float yMag = in.readFloatBig();
  in.readFloatBig();
  float zMag = in.readFloatBig();
  in.readFloatBig();
  float duration = in.readFloatBig();

  SCameraShakePoint xAM(0, 0.f, 0.f, duration, 2.f * xMag);
  SCameraShakePoint yAM(0, 0.f, 0.f, duration, 2.f * yMag);
  SCameraShakePoint zAM(0, 0.f, 0.f, duration, 2.f * zMag);
  SCameraShakePoint xFM(0, 0.f, 0.f, 0.5f * duration, 3.f);
  SCameraShakePoint yFM(0, 0.f, 0.f, 0.5f * duration, 0.f);
  SCameraShakePoint zFM(0, 0.f, 0.f, 0.5f * duration, 3.f);

  CCameraShakerComponent shakerX(1, xAM, xFM);
  CCameraShakerComponent shakerY;
  CCameraShakerComponent shakerZ(1, zAM, zFM);

  return {duration, 100.f, 0, zeus::skZero3f, shakerX, shakerY, shakerZ};
}

const CCameraShakeData CCameraShakeData::skChargedShotCameraShakeData = {
    0.3f,
    100.f,
    0,
    zeus::skZero3f,
    CCameraShakerComponent(),
    CCameraShakerComponent(1, {0, 0.f, 0.f, 0.3f, -1.f}, {1, 0.f, 0.f, 0.05f, 0.3f}),
    CCameraShakerComponent()};

} // namespace urde
