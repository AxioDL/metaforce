#pragma once

#include "CActor.hpp"
#include "Camera/CBallCamera.hpp"

namespace urde {

class CCameraHint {
  u32 x4_overrideFlags;
  CBallCamera::EBallCameraBehaviour x8_behaviour;
  float xc_minDist;
  float x10_maxDist;
  float x14_backwardsDist;
  zeus::CVector3f x18_lookAtOffset;
  zeus::CVector3f x24_chaseLookAtOffset;
  zeus::CVector3f x30_ballToCam;
  float x3c_fov;
  float x40_attitudeRange;
  float x44_azimuthRange;
  float x48_anglePerSecond;
  float x4c_clampVelRange;
  float x50_clampRotRange;
  float x54_elevation;
  float x58_interpolateTime;
  float x5c_clampVelTime;
  float x60_controlInterpDur;

public:
  CCameraHint(u32 overrideFlags, CBallCamera::EBallCameraBehaviour behaviour, float minDist, float maxDist,
              float backwardsDist, const zeus::CVector3f& lookAtOffset, const zeus::CVector3f& chaseLookAtOffset,
              const zeus::CVector3f& ballToCam, float fov, float attitudeRange, float azimuthRange,
              float anglePerSecond, float clampVelRange, float clampRotRange, float elevation, float interpolateTime,
              float clampVelTime, float controlInterpDur)
  : x4_overrideFlags(overrideFlags)
  , x8_behaviour(behaviour)
  , xc_minDist(minDist)
  , x10_maxDist(maxDist)
  , x14_backwardsDist(backwardsDist)
  , x18_lookAtOffset(lookAtOffset)
  , x24_chaseLookAtOffset(chaseLookAtOffset)
  , x30_ballToCam(ballToCam)
  , x3c_fov(fov)
  , x40_attitudeRange(attitudeRange)
  , x44_azimuthRange(azimuthRange)
  , x48_anglePerSecond(anglePerSecond)
  , x4c_clampVelRange(clampVelRange)
  , x50_clampRotRange(clampRotRange)
  , x54_elevation(elevation)
  , x58_interpolateTime(interpolateTime)
  , x5c_clampVelTime(clampVelTime)
  , x60_controlInterpDur(controlInterpDur) {}

  u32 GetOverrideFlags() const { return x4_overrideFlags; }
  CBallCamera::EBallCameraBehaviour GetBehaviourType() const { return x8_behaviour; }
  float GetMinDist() const { return xc_minDist; }
  float GetMaxDist() const { return x10_maxDist; }
  float GetBackwardsDist() const { return x14_backwardsDist; }
  const zeus::CVector3f& GetLookAtOffset() const { return x18_lookAtOffset; }
  const zeus::CVector3f& GetChaseLookAtOffset() const { return x24_chaseLookAtOffset; }
  const zeus::CVector3f& GetBallToCam() const { return x30_ballToCam; }
  float GetFov() const { return x3c_fov; }
  float GetAttitudeRange() const { return x40_attitudeRange; }
  float GetAzimuthRange() const { return x44_azimuthRange; }
  float GetAnglePerSecond() const { return x48_anglePerSecond; }
  float GetClampVelRange() const { return x4c_clampVelRange; }
  float GetClampRotRange() const { return x50_clampRotRange; }
  float GetElevation() const { return x54_elevation; }
  float GetInterpolateTime() const { return x58_interpolateTime; }
  float GetClampVelTime() const { return x5c_clampVelTime; }
  float GetControlInterpDur() const { return x60_controlInterpDur; }
};

class CScriptCameraHint : public CActor {
  s32 xe8_priority;
  CCameraHint xec_hint;
  rstl::reserved_vector<TUniqueId, 8> x150_helpers;
  TUniqueId x164_delegatedCamera = kInvalidUniqueId;
  bool x166_inactive = false;
  zeus::CTransform x168_origXf;
  void InitializeInArea(CStateManager& mgr);
  void AddHelper(TUniqueId id);
  void RemoveHelper(TUniqueId id);

public:
  CScriptCameraHint(TUniqueId, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf, bool active,
                    s32 priority, CBallCamera::EBallCameraBehaviour behaviour, u32 overrideFlags, float minDist,
                    float maxDist, float backwardsDist, const zeus::CVector3f& lookAtOffset,
                    const zeus::CVector3f& chaseLookAtOffset, const zeus::CVector3f& ballToCam, float fov,
                    float attitudeRange, float azimuthRange, float anglePerSecond, float clampVelRange,
                    float clampRotRange, float elevation, float interpolateTime, float clampVelTime,
                    float controlInterpDur);

  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);

  void ClearIdList() { x150_helpers.clear(); }
  void SetInactive(bool inactive) { x166_inactive = inactive; }
  bool GetInactive() const { return x166_inactive; }
  size_t GetHelperCount() const { return x150_helpers.size(); }
  TUniqueId GetFirstHelper() const { return x150_helpers.empty() ? kInvalidUniqueId : x150_helpers[0]; }
  s32 GetPriority() const { return xe8_priority; }
  const CCameraHint& GetHint() const { return xec_hint; }
  TUniqueId GetDelegatedCamera() const { return x164_delegatedCamera; }
  const zeus::CTransform& GetOriginalTransform() const { return x168_origXf; }
};
} // namespace urde
