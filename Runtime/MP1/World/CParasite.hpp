#pragma once

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
        zeus::CVector3f x0_v;
        float xc_f;
    public:
        CRepulsor(const zeus::CVector3f& v, float f) : x0_v(v), xc_f(f) {}
        const zeus::CVector3f& GetVector() const { return x0_v; }
        float GetFloat() const { return xc_f; }
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
    zeus::CVector3f x5f8_;
    float x604_ = 1.f;
    float x608_ = 0.f;
    float x60c_ = 0.f;
    zeus::CVector3f x614_;
    std::unique_ptr<CCollisionActorManager> x620_ = 0;
    TLockedToken<CSkinnedModel> x624_extraModel;
    zeus::CVector3f x628_;
    zeus::CVector3f x634_;
    zeus::CVector3f x640_;
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
    u16 x73c_haltSfx;
    u16 x73e_getUpSfx;
    u16 x740_crouchSfx;
    union
    {
        struct
        {
            bool x742_24_ : 1;
            bool x742_25_jumpVelDirty : 1;
            bool x742_26_ : 1;
            bool x742_27_landed : 1;
            bool x742_28_onGround : 1;
            bool x742_29_ : 1;
            bool x742_30_attackOver : 1;
            bool x742_31_ : 1;
            bool x743_24_ : 1;
            bool x743_25_ : 1;
            bool x743_26_oculusShotAt : 1;
            bool x743_27_inJump : 1;
        };
        u16 _dummy = 0;
    };
    bool CloseToWall(CStateManager& mgr);
    void FaceTarget(const zeus::CVector3f& target);
    TUniqueId RecursiveFindClosestWayPoint(CStateManager& mgr, TUniqueId id, float& dist);
    TUniqueId GetClosestWaypointForState(EScriptObjectState state, CStateManager& mgr);
    void UpdatePFDestination(CStateManager& mgr);
    void DoFlockingBehavior(CStateManager& mgr);
public:
    DEFINE_PATTERNED(Parasite)
    CParasite(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
              const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo&, EBodyType, float, float, float,
              float, float, float, float, float, float, float, float, float, float, float, float, float, float, float,
              bool, EWalkerType wType, const CDamageVulnerability&, const CDamageInfo&, u16, u16, u16, CAssetId, CAssetId, float,
              const CActorParameters&);

    void Accept(IVisitor&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void PreThink(float, CStateManager&);
    void Think(float dt, CStateManager& mgr);
    void Render(const CStateManager&) const;
    const CDamageVulnerability* GetDamageVulnerability() const;
    void Touch(CActor& actor, CStateManager&);
    zeus::CVector3f GetAimPosition(const CStateManager&, float) const;
    void CollidedWith(TUniqueId uid, const CCollisionInfoList&, CStateManager&);
    void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state);
    void Patrol(CStateManager&, EStateMsg msg, float dt);
    void PathFind(CStateManager&, EStateMsg msg, float dt);
    void TargetPlayer(CStateManager&, EStateMsg msg, float dt);
    void TargetPatrol(CStateManager&, EStateMsg msg, float dt);
    void Halt(CStateManager&, EStateMsg, float);
    void Run(CStateManager&, EStateMsg, float);
    void Generate(CStateManager&, EStateMsg, float);
    void Deactivate(CStateManager&, EStateMsg, float);
    void Attack(CStateManager&, EStateMsg, float);
    void Crouch(CStateManager&, EStateMsg, float);
    void GetUp(CStateManager&, EStateMsg, float);
    void TelegraphAttack(CStateManager&, EStateMsg, float);
    void Jump(CStateManager&, EStateMsg, float);
    void Retreat(CStateManager&, EStateMsg, float);
    bool AnimOver(CStateManager&, float);
    bool ShouldAttack(CStateManager&, float);
    bool HitSomething(CStateManager&, float);
    bool Stuck(CStateManager&, float);
    bool Landed(CStateManager&, float);
    bool AttackOver(CStateManager&, float);
    bool ShotAt(CStateManager&, float);
    void MassiveDeath(CStateManager&);
    void MassiveFrozenDeath(CStateManager&);
    void ThinkAboutMove(float);
    bool IsOnGround() const;
    virtual void UpdateWalkerAnimation(CStateManager&, float);

    void DestroyActorManager(CStateManager& mgr);
    void UpdateJumpVelocity();
    void UpdateCollisionActors(CStateManager&) {}
    CDamageInfo GetContactDamage() const
    {
        if (x5d0_walkerType == EWalkerType::Oculus && x743_24_)
            return x6b4_;
        return CPatterned::GetContactDamage();
    }
    void AlignToFloor(CStateManager&, float, const zeus::CVector3f&, float);
};
}
