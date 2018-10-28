#pragma once

#include "CStateManager.hpp"

namespace urde
{
struct SBurst
{
    u32 x0_;
    u32 x4_;
    u32 x8_;
    s32 xc_;
    s32 x10_;
    u32 x14_;
    u32 x18_;
    u32 x1c_;
    u32 x20_;
    float x24_;
    float x28_;
};

class CBurstFire
{
    s32 x0_ = -1;
    s32 x4_ = -1;
    float x8_ = 0.f;
    u32 xc_ = 0;
    s32 x10_;
    union
    {
        struct { bool x14_24_ : 1; bool x14_25_ : 1; };
        u32 _dummy = 0;
    };

    rstl::reserved_vector<SBurst*, 16> x18_bursts;
public:
    CBurstFire(SBurst**, s32);

    void SetFirstBurst(bool);
    void SetBurstType(s32);
    bool IsBurstSet() const;
    void SetTimeToNextShot(float);
    bool ShouldFire() const;
    s32 GetBurstType() const;
    void Start(CStateManager&);
    void Update(CStateManager&, float);
    void Update();
    void GetError(float, float) const;
    zeus::CVector3f GetDistanceCompensatedError(float, float) const;
    float GetMaxXError() const;
    float GetMaxZError() const;
    void GetError() const;
    void SetFirstBurstIndex(s32);

    bool GetX14_24() const { return x14_24_; }
};
}
