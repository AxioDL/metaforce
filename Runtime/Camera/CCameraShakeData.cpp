#include "Runtime/Camera/CCameraShakeData.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/ScriptLoader.hpp"

#include <algorithm>
#include <cmath>

#include <zeus/Math.hpp>

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
  const float xMag = in.readFloatBig();
  in.readFloatBig();
  const float yMag = in.readFloatBig();
  in.readFloatBig();
  const float zMag = in.readFloatBig();
  in.readFloatBig();
  const float duration = in.readFloatBig();

  const SCameraShakePoint xAM(false, 0.f, 0.f, duration, 2.f * xMag);
  const SCameraShakePoint yAM(false, 0.f, 0.f, duration, 2.f * yMag);
  const SCameraShakePoint zAM(false, 0.f, 0.f, duration, 2.f * zMag);
  const SCameraShakePoint xFM(false, 0.f, 0.f, 0.5f * duration, 3.f);
  const SCameraShakePoint yFM(false, 0.f, 0.f, 0.5f * duration, 0.f);
  const SCameraShakePoint zFM(false, 0.f, 0.f, 0.5f * duration, 3.f);

  const CCameraShakerComponent shakerX(true, xAM, xFM);
  const CCameraShakerComponent shakerY;
  const CCameraShakerComponent shakerZ(true, zAM, zFM);

  return {duration, 100.f, 0, zeus::skZero3f, shakerX, shakerY, shakerZ};
}

constexpr CCameraShakeData CCameraShakeData::skChargedShotCameraShakeData{
    0.3f,
    100.f,
    0,
    zeus::skZero3f,
    CCameraShakerComponent{},
    CCameraShakerComponent{
        true,
        {false, 0.f, 0.f, 0.3f, -1.f},
        {true, 0.f, 0.f, 0.05f, 0.3f},
    },
    CCameraShakerComponent{},
};

} // namespace urde
