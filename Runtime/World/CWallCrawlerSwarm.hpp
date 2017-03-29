#ifndef __URDE_CWALLCRAWLERSWARM_HPP__
#define __URDE_CWALLCRAWLERSWARM_HPP__

#include "World/CActor.hpp"
#include "Collision/CCollisionSurface.hpp"

namespace urde
{
class CWallCrawlerSwarm : public CActor
{
public:
    class CBoid
    {
        zeus::CTransform x0_;
        float x30_ = 0.f;
        float x34_ = 0.f;
        float x38_ = 0.f;
        TUniqueId x3c_ = kInvalidUniqueId;
        zeus::CColor x40_ = zeus::CColor(0.3f, 0.3f, 0.3f, 1.f);
        u32 x44_ = 0;
        float x48_ = 0.f;
        float x4c_ = 0.f;
        CCollisionSurface x50_ = CCollisionSurface(zeus::CVector3f(0.f, 0.f, 1.f), zeus::CVector3f(0.f, 1.f, 0.f),
                                                   zeus::CVector3f(1.f, 0.f, 0.f), -1);
        union
        {
            struct
            {
                u32 x7c_unk1 : 8;
                u32 x7c_unk2 : 10;
            };
            u32 x7c_;
        };

        union
        {
            struct
            {
                bool x80_24_ : 1;
                bool x80_25_ : 1;
                bool x80_26_ : 1;
                bool x80_27_ : 1;
                bool x80_28_ : 1;
            };
            u32 x80_;
        };
    };

    zeus::CAABox xe8_aabox = zeus::CAABox::skNullBox;
    zeus::CVector3f x130_lastKilledOffset;
public:
    CWallCrawlerSwarm(TUniqueId, bool, const std::string&, const CEntityInfo&, const zeus::CVector3f&,
                      const zeus::CTransform&, u32, const CAnimRes&, u32, u32, u32, u32, u32, u32, const CDamageInfo&,
                      const CDamageInfo&, float, float, float, float, u32, u32, float, float, float, float, float,
                      float, float, float, float, u32, float, float, float, const CHealthInfo&, const CDamageVulnerability&,
                      u32, u32, const CActorParameters&);

    void Accept(IVisitor &visitor);
    zeus::CVector3f GetLastKilledOffset() const { return x130_lastKilledOffset; }
    void ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info,
                           CStateManager& stateMgr) {}
};
}

#endif // __URDE_CWALLCRAWLERSWARM_HPP__
