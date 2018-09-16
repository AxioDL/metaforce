#ifndef __URDE_MP1_CPARASITE_HPP__
#define __URDE_MP1_CPARASITE_HPP__

#include "World/CWallWalker.hpp"
#include "Collision/CCollisionActorManager.hpp"
namespace urde
{
class CModelData;
}

namespace urde::MP1
{
class CParasite : public CWallWalker
{
    class CRepulsor
    {
    };
    static const float flt_805A8FB0;
    static const float skAttackVelocity;
    static short word_805A8FC0;
    static const float flt_805A8FB8;
    static const float skRetreatVelocity;
    std::vector<CRepulsor> x5d8_doorRepulsors;
    s32 x5e8_ = -1;
    float x5ec_ = 0.f;
    float x5f0_ = 0.f;
    float x5f4_ = 0.f;
    float x5f8_ = 0.f;
    float x5fc_ = 0.f;
    float x600_ = 0.f;
    float x604_ = 1.f;
    float x608_ = 0.f;
    float x60c_ = 0.f;
    zeus::CVector3f x614_;
    std::unique_ptr<CCollisionActorManager> x620_ = 0;
    u32 x624_ = 0;
    float x628_ = 0.f;
    float x62c_ = 0.f;
    float x630_ = 0.f;
    float x634_ = 0.f;
    float x638_ = 0.f;
    float x640_ = 0.f;
    float x644_ = 0.f;
    float x648_ = 0.f;
    CDamageVulnerability x64c_;
    CDamageInfo x6b4_;
    float x6d0_;
    float x6d4_;
    float x6dc_;
    float x6e0_;
    float x6e4_;
    float x6e8_;
    float x6ec_;
    float x6f0_;
    float x6f4_;
    float x6f8_;
    float x6fc_;
    float x700_;
    float x704_;
    float x708_;
    float x710_;
    float x714_;
    float x718_ = 0.f;
    float x71c_ = 0.f;
    float x720_ = 0.f;
    float x724_ = 0.f;
    float x728_ = 0.f;
    float x72c_ = 0.f;
    float x730_ = 0.f;
    float x734_ = 0.f;
    float x738_ = 0.f;
    s16 x73c_;
    s16 x73e_;
    s16 x740_;
    union
    {
        struct
        {
            bool x742_24_ : 1;
            bool x742_25_ : 1;
            bool x742_26_ : 1;
            bool x742_27_ : 1;
            bool x742_28_ : 1;
            bool x742_29_ : 1;
            bool x742_30_ : 1;
            bool x742_31_ : 1;
            bool x743_24_ : 1;
            bool x743_25_ : 1;
            bool x743_26_ : 1;
            bool x743_27_ : 1;
        };
        u16 _dummy = 0;
    };
public:
    DEFINE_PATTERNED(Parasite)
    CParasite(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
              const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo&, EBodyType, float, float, float,
              float, float, float, float, float, float, float, float, float, float, float, float, float, float, float,
              bool, u32, const CDamageVulnerability&, const CDamageInfo&, u16, u16, u16, u32, u32, float,
              const CActorParameters&);

    void Accept(IVisitor&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void PreThink(float, CStateManager&);
    void Think(float dt, CStateManager& mgr);
    void DestroyActorManager(CStateManager& mgr);
    void UpdateJumpVelocity();
    void UpdateCollisionActors(CStateManager&) {}
    CDamageInfo GetContactDamage() const
    {
        if (x5d0_ == 1 && x743_24_)
            return x6b4_;
        return CPatterned::GetContactDamage();
    }
    void AlignToFloor(CStateManager&, float, const zeus::CVector3f&, float);
};
}
#endif // __URDE_MP1_CPARASITE_HPP__
