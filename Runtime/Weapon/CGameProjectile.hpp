#pragma once

#include "Collision/CMaterialList.hpp"
#include "Weapon/CWeaponMode.hpp"
#include "Weapon/CWeapon.hpp"
#include "World/CDamageInfo.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "Weapon/CProjectileWeapon.hpp"
#include "Collision/CRayCastResult.hpp"

namespace urde
{
class CGenDescription;
class CWeaponDescription;

class CProjectileTouchResult
{
    TUniqueId x0_id;
    std::experimental::optional<CRayCastResult> x4_result;
public:
    CProjectileTouchResult(TUniqueId id, const std::experimental::optional<CRayCastResult>& result)
    : x0_id(id), x4_result(result) {}
    TUniqueId GetActorId() const { return x0_id; }
    bool HasRayCastResult() const { return x4_result.operator bool(); }
    const CRayCastResult& GetRayCastResult() const { return *x4_result; }
};

class CGameProjectile : public CWeapon
{
protected:
    std::experimental::optional<TLockedToken<CGenDescription>> x158_visorParticle;
    u16 x168_visorSfx;
    CProjectileWeapon x170_projectile;
    zeus::CVector3f x298_lastOrigin;
    float x2a4_projExtent;
    float x2a8_homingDt = 0.03f;
    double x2b0_targetHomingTime = 0.0;
    double x2b8_curHomingTime = x2a8_homingDt;
    TUniqueId x2c0_homingTargetId;
    TUniqueId x2c2_ = kInvalidUniqueId;
    TUniqueId x2c4_ = kInvalidUniqueId;
    TUniqueId x2c6_ = kInvalidUniqueId;
    TUniqueId x2c8_projectileLight = kInvalidUniqueId;
    CAssetId x2cc_wpscId;
    std::vector<CProjectileTouchResult> x2d0_touchResults;
    float x2e0_minHomingDist = 0.f;
    union
    {
        struct
        {
            bool x2e4_24_active : 1;
            bool x2e4_25_startedUnderwater : 1;
            bool x2e4_26_waterUpdate : 1;
            bool x2e4_27_inWater : 1;
            bool x2e4_28_sendProjectileCollideMsg : 1;
        };
    };
public:
    CGameProjectile(bool active, const TToken<CWeaponDescription>&, std::string_view name,
                    EWeaponType wType, const zeus::CTransform& xf, EMaterialTypes excludeMat,
                    const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                    TUniqueId homingTarget, EProjectileAttrib attribs, bool underwater, const zeus::CVector3f& scale,
                    const std::experimental::optional<TLockedToken<CGenDescription>>& visorParticle,
                    u16 visorSfx, bool sendCollideMsg);

    virtual void Accept(IVisitor &visitor);
    virtual void ResolveCollisionWithActor(const CRayCastResult& res, CActor& act, CStateManager& mgr);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    static EProjectileAttrib GetBeamAttribType(EWeaponType wType);
    void DeleteProjectileLight(CStateManager&);
    void CreateProjectileLight(std::string_view, const CLight&, CStateManager&);
    void Chase(float dt, CStateManager& mgr);
    void UpdateHoming(float dt, CStateManager& mgr);
    void UpdateProjectileMovement(float dt, CStateManager& mgr);
    CRayCastResult DoCollisionCheck(TUniqueId& idOut, CStateManager& mgr);
    void ApplyDamageToActors(CStateManager& mgr, const CDamageInfo& dInfo);
    void FluidFxThink(EFluidState state, CScriptWater& water, CStateManager& mgr);
    CRayCastResult RayCollisionCheckWithWorld(TUniqueId& idOut, const zeus::CVector3f& start,
                                              const zeus::CVector3f& end, float mag,
                                              const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                              CStateManager& mgr);
    CProjectileTouchResult CanCollideWith(CActor& act, CStateManager& mgr);
    CProjectileTouchResult CanCollideWithComplexCollision(CActor& act, CStateManager& mgr);
    CProjectileTouchResult CanCollideWithGameObject(CActor& act, CStateManager& mgr);
    CProjectileTouchResult CanCollideWithTrigger(CActor& act, CStateManager& mgr);
    zeus::CAABox GetProjectileBounds() const;
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    TUniqueId GetHomingTargetId() const { return x2c0_homingTargetId; }
};
}

