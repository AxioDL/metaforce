#pragma once

#include "Runtime/RetroTypes.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CRandom16;
class CStateManager;

struct SCameraShakePoint {
  friend class CCameraShakeData;
  bool x0_useEnvelope = false;
  float x4_value = 0.f;
  float x8_magnitude = 0.f;
  float xc_attackTime = 0.f;
  float x10_sustainTime = 0.f;
  float x14_duration = 0.f;
  constexpr SCameraShakePoint() noexcept = default;
  constexpr SCameraShakePoint(bool useEnvelope, float attackTime, float sustainTime, float duration,
                              float magnitude) noexcept
  : x0_useEnvelope(useEnvelope)
  , x8_magnitude(magnitude)
  , xc_attackTime(attackTime)
  , x10_sustainTime(sustainTime)
  , x14_duration(duration) {}
  [[nodiscard]] constexpr float GetValue() const noexcept { return x0_useEnvelope ? x8_magnitude : x4_value; }
  static SCameraShakePoint LoadCameraShakePoint(CInputStream& in);
  void Update(float curTime);
};

class CCameraShakerComponent {
  friend class CCameraShakeData;
  bool x4_useModulation = false;
  SCameraShakePoint x8_am, x20_fm;
  float x38_value = 0.f;

public:
  constexpr CCameraShakerComponent() noexcept = default;
  constexpr CCameraShakerComponent(bool useModulation, const SCameraShakePoint& am,
                                   const SCameraShakePoint& fm) noexcept
  : x4_useModulation(useModulation), x8_am(am), x20_fm(fm) {}
  static CCameraShakerComponent LoadNewCameraShakerComponent(CInputStream& in);
  void Update(float curTime, float duration, float distAtt);
  [[nodiscard]] constexpr float GetValue() const noexcept { return x38_value; }
};

class CCameraShakeData {
  friend class CCameraManager;
  float x0_duration;
  float x4_curTime = 0.f;
  CCameraShakerComponent x8_shakerX;
  CCameraShakerComponent x44_shakerY;
  CCameraShakerComponent x80_shakerZ;
  u32 xbc_shakerId = 0;
  u32 xc0_flags; // 0x1: positional sfx
  zeus::CVector3f xc4_sfxPos;
  float xd0_sfxDist;

public:
  static const CCameraShakeData skChargedShotCameraShakeData;

  constexpr CCameraShakeData(float duration, float sfxDist, u32 flags, const zeus::CVector3f& sfxPos,
                             const CCameraShakerComponent& shaker1, const CCameraShakerComponent& shaker2,
                             const CCameraShakerComponent& shaker3) noexcept
  : x0_duration(duration)
  , x8_shakerX(shaker1)
  , x44_shakerY(shaker2)
  , x80_shakerZ(shaker3)
  , xc0_flags(flags)
  , xc4_sfxPos(sfxPos)
  , xd0_sfxDist(sfxDist) {}

  constexpr CCameraShakeData(float duration, float magnitude) noexcept
  : CCameraShakeData(
        duration, 100.f, 0, zeus::skZero3f, CCameraShakerComponent{}, CCameraShakerComponent{},
        CCameraShakerComponent{true, SCameraShakePoint{false, 0.25f * duration, 0.f, 0.75f * duration, magnitude},
                               SCameraShakePoint{true, 0.f, 0.f, 0.5f * duration, 2.f}}) {}

  explicit CCameraShakeData(CInputStream&);

  static constexpr CCameraShakeData BuildLandingCameraShakeData(float duration, float magnitude) noexcept {
    return {
        duration,
        100.f,
        0,
        zeus::skZero3f,
        CCameraShakerComponent{
            true,
            SCameraShakePoint(false, 0.15f * duration, 0.f, 0.85f * duration, magnitude),
            SCameraShakePoint(true, 0.f, 0.f, 0.4f * duration, 1.5f),
        },
        CCameraShakerComponent{},
        CCameraShakerComponent{
            true,
            SCameraShakePoint(false, 0.25f * duration, 0.f, 0.75f * duration, magnitude),
            SCameraShakePoint(true, 0.f, 0.f, 0.5f * duration, 2.f),
        },
    };
  }

  static constexpr CCameraShakeData BuildProjectileCameraShake(float duration, float magnitude) noexcept {
    return {
        duration,
        100.f,
        0,
        zeus::skZero3f,
        CCameraShakerComponent{
            true,
            SCameraShakePoint(false, 0.f, 0.f, duration, magnitude),
            SCameraShakePoint(true, 0.f, 0.f, 0.5f * duration, 3.f),
        },
        CCameraShakerComponent{},
        CCameraShakerComponent{},
    };
  }

  static constexpr CCameraShakeData BuildMissileCameraShake(float duration, float magnitude, float sfxDistance,
                                                            const zeus::CVector3f& sfxPos) noexcept {
    CCameraShakeData ret(duration, magnitude);
    ret.SetSfxPositionAndDistance(sfxPos, sfxDistance);
    return ret;
  }

  static constexpr CCameraShakeData BuildPhazonCameraShakeData(float duration, float magnitude) noexcept {
    return {
        duration,
        100.f,
        0,
        zeus::skZero3f,
        CCameraShakerComponent{
            true,
            SCameraShakePoint(false, 0.15f * duration, 0.f, 0.25f * duration, magnitude),
            SCameraShakePoint(true, 0.f, 0.f, 0.4f * duration, 0.3f),
        },
        CCameraShakerComponent{},
        CCameraShakerComponent{
            true,
            SCameraShakePoint(false, 0.25f * duration, 0.f, 0.25f * duration, magnitude),
            SCameraShakePoint(true, 0.f, 0.f, 0.5f * duration, 0.5f),
        },
    };
  }

  static constexpr CCameraShakeData BuildPatternedExplodeShakeData(float duration, float magnitude) noexcept {
    return {
        duration,
        100.f,
        0,
        zeus::skZero3f,
        CCameraShakerComponent{
            true,
            SCameraShakePoint(false, 0.25f * duration, 0.f, 0.75f * duration, magnitude),
            SCameraShakePoint(true, 0.f, 0.f, 0.5f * duration, 2.0f),
        },
        CCameraShakerComponent{},
        CCameraShakerComponent{},
    };
  }

  static constexpr CCameraShakeData BuildPatternedExplodeShakeData(const zeus::CVector3f& pos, float duration,
                                                                   float magnitude, float distance) noexcept {
    CCameraShakeData shakeData = BuildPatternedExplodeShakeData(duration, magnitude);
    shakeData.SetSfxPositionAndDistance(pos, distance);
    return shakeData;
  }

  void Update(float dt, CStateManager& mgr);
  zeus::CVector3f GetPoint() const;
  float GetMaxAMComponent() const;
  float GetMaxFMComponent() const;
  void SetShakerId(u32 id) { xbc_shakerId = id; }
  u32 GetShakerId() const { return xbc_shakerId; }
  static CCameraShakeData LoadCameraShakeData(CInputStream& in);
  constexpr void SetSfxPositionAndDistance(const zeus::CVector3f& pos, float sfxDistance) noexcept {
    xc0_flags |= 0x1;
    xc4_sfxPos = pos;
    xd0_sfxDist = sfxDistance;
  }
};

} // namespace urde
