#ifndef __URDE_CCAMERASHAKEDATA_HPP__
#define __URDE_CCAMERASHAKEDATA_HPP__

#include "zeus/CVector3f.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CRandom16;
class CStateManager;

struct SCameraShakePoint
{
    friend class CCameraShakeData;
    u32 x0_w1 = 0;
    float x4_ = 0.f;
    float x8_magnitude = 0.f;
    float xc_f1 = 0.f;
    float x10_f2 = 0.f;
    float x14_duration = 0.f;
    SCameraShakePoint() = default;
    SCameraShakePoint(u32 w1, float f1, float f2, float duration, float magnitude)
    : x0_w1(w1), x8_magnitude(magnitude), xc_f1(f1), x10_f2(f2), x14_duration(duration) {}
    float GetSomething() const { return x0_w1 ? x8_magnitude : x4_; }
    static SCameraShakePoint LoadCameraShakePoint(CInputStream& in);
};

class CCameraShakerComponent
{
    friend class CCameraShakeData;
    u32 x4_w1 = 0;
    SCameraShakePoint x8_sp1, x20_sp2;
    float x38_value = 0.f;
public:
    CCameraShakerComponent() = default;
    CCameraShakerComponent(u32 w1, const SCameraShakePoint& sp1, const SCameraShakePoint& sp2)
    : x4_w1(w1), x8_sp1(sp1), x20_sp2(sp2) {}
    static CCameraShakerComponent LoadNewCameraShakerComponent(CInputStream& in);
};

class CCameraShakeData
{
    friend class CCameraManager;
    float x0_duration;
    float x4_curTime = 0.f;
    CCameraShakerComponent x8_shaker1;
    CCameraShakerComponent x44_shaker2;
    CCameraShakerComponent x80_shaker3;
    u32 xbc_shakerId = 0;
    u32 xc0_flags; // 0x1: positional sfx
    zeus::CVector3f xc4_sfxPos;
    float xd0_sfxDist;

public:
    static const CCameraShakeData skChargedShotCameraShakeData;
    CCameraShakeData(float duration, float sfxDist, u32 w1, const zeus::CVector3f& sfxPos,
                     const CCameraShakerComponent& shaker1, const CCameraShakerComponent& shaker2,
                     const CCameraShakerComponent& shaker3);
    CCameraShakeData(float duration, float magnitude);
    static CCameraShakeData BuildLandingCameraShakeData(float duration, float magnitude);
    static CCameraShakeData BuildProjectileCameraShake(float duration, float magnitude);
    static CCameraShakeData BuildMissileCameraShake(float duration, float magnitude, float sfxDistance,
                                                    const zeus::CVector3f& sfxPos);
    static CCameraShakeData BuildPhazonCameraShakeData(float duration, float magnitude);
    //zeus::CVector3f GeneratePoint(float dt, CRandom16& r);
    void Update(float dt, CStateManager& mgr);
    zeus::CVector3f GetPoint() const;
    float GetSomething() const;
    float GetSomething2() const;
    void SetShakerId(u32 id) { xbc_shakerId = id; }
    u32 GetShakerId() const { return xbc_shakerId; }
    static CCameraShakeData LoadCameraShakeData(CInputStream& in);
    void SetSfxPositionAndDistance(const zeus::CVector3f& pos, float f2)
    { xc0_flags |= 0x1; xc4_sfxPos = pos; xd0_sfxDist = f2; }
};

}

#endif // __URDE_CCAMERASHAKEDATA_HPP__
