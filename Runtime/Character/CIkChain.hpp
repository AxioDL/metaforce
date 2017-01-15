#ifndef __URDE_CIKCHAIN_HPP__
#define __URDE_CIKCHAIN_HPP__

#include "RetroTypes.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CQuaternion.hpp"
#include "Character/CSegId.hpp"

namespace urde
{
class CAnimData;
class CSegId;
class CIkChain
{
    CSegId x0_;
    CSegId x1_;
    CSegId x2_;
    zeus::CVector3f x4_;
    zeus::CVector3f x10_;
    float x1c_;
    float x20_;
    zeus::CQuaternion x24_;
    zeus::CVector3f x34_;
    float x40_time = 0.f;

    union
    {
        struct
        {
            bool x44_24_activated : 1;
        };
        u8 x44_dummy = 0;
    };
public:
    CIkChain() = default;

    bool GetActive() const { return x44_24_activated; }
    void Update(float);
    void Deactivate();
    void Activate(const CAnimData&, const CSegId&, const zeus::CTransform&);
    void PreRender(CAnimData&, const zeus::CTransform&, const zeus::CVector3f&);
    void Solve(zeus::CQuaternion&, zeus::CQuaternion&, const zeus::CVector3f&);
};
}

#endif // __URDE_CIKCHAIN_HPP__
