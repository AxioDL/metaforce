#pragma once

#include "zeus/CVector3f.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CRandom16;
class CStateManager;

struct SCameraShakePoint
{
    friend class CCameraShakeData;
    bool x0_useEnvelope = false;
    float x4_value = 0.f;
    float x8_magnitude = 0.f;
    float xc_attackTime = 0.f;
    float x10_sustainTime = 0.f;
    float x14_duration = 0.f;
    SCameraShakePoint() = default;
    SCameraShakePoint(bool useEnvelope, float attackTime, float sustainTime, float duration, float magnitude)
    : x0_useEnvelope(useEnvelope), x8_magnitude(magnitude), xc_attackTime(attackTime),
      x10_sustainTime(sustainTime), x14_duration(duration) {}
    float GetValue() const { return x0_useEnvelope ? x8_magnitude : x4_value; }
    static SCameraShakePoint LoadCameraShakePoint(CInputStream& in);
    void Update(float curTime);
};

class CCameraShakerComponent
{
    friend class CCameraShakeData;
    bool x4_useModulation = false;
    SCameraShakePoint x8_am, x20_fm;
    float x38_value = 0.f;
public:
    CCameraShakerComponent() = default;
    CCameraShakerComponent(bool useModulation, const SCameraShakePoint& am, const SCameraShakePoint& fm)
    : x4_useModulation(useModulation), x8_am(am), x20_fm(fm) {}
    static CCameraShakerComponent LoadNewCameraShakerComponent(CInputStream& in);
    void Update(float curTime, float duration, float distAtt);
    float GetValue() const { return x38_value; }
};

class CCameraShakeData
{
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
    CCameraShakeData(float duration, float sfxDist, u32 flags, const zeus::CVector3f& sfxPos,
                     const CCameraShakerComponent& shaker1, const CCameraShakerComponent& shaker2,
                     const CCameraShakerComponent& shaker3);
    CCameraShakeData(float duration, float magnitude);
    CCameraShakeData(CInputStream&);
    static CCameraShakeData BuildLandingCameraShakeData(float duration, float magnitude);
    static CCameraShakeData BuildProjectileCameraShake(float duration, float magnitude);
    static CCameraShakeData BuildMissileCameraShake(float duration, float magnitude, float sfxDistance,
                                                    const zeus::CVector3f& sfxPos);
    static CCameraShakeData BuildPhazonCameraShakeData(float duration, float magnitude);
    static CCameraShakeData BuildPatternedExplodeShakeData(float duration, float magnitude);
    static CCameraShakeData BuildPatternedExplodeShakeData(const zeus::CVector3f& pos, float duration,
                                                           float magnitude, float distance);
    void Update(float dt, CStateManager& mgr);
    zeus::CVector3f GetPoint() const;
    float GetMaxAMComponent() const;
    float GetMaxFMComponent() const;
    void SetShakerId(u32 id) { xbc_shakerId = id; }
    u32 GetShakerId() const { return xbc_shakerId; }
    static CCameraShakeData LoadCameraShakeData(CInputStream& in);
    void SetSfxPositionAndDistance(const zeus::CVector3f& pos, float f2)
    { xc0_flags |= 0x1; xc4_sfxPos = pos; xd0_sfxDist = f2; }
};

}

