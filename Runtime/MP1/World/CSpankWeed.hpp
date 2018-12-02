#pragma once

#include "World/CPatterned.hpp"
#include "Collision/CCollisionActorManager.hpp"

namespace urde::MP1
{
class CSpankWeed : public CPatterned
{
    float x568_;
    float x56c_;
    float x570_;
    float x574_;
    float x578_;
    bool x57c_ = false;
    float x580_ = 0.f;
    std::unique_ptr<CCollisionActorManager> x594_collisionMgr;
    zeus::CVector3f x584_;
    zeus::CVector3f x59c_;
    zeus::CVector3f x5a8_;
    s32 x5b4_ = -1;
    s32 x5b8_ = -1;
    s32 x5bc_ = -1;

public:
    DEFINE_PATTERNED(SpankWeed)

    CSpankWeed(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
               const CActorParameters&, const CPatternedInfo&, float, float, float, float);
};
}