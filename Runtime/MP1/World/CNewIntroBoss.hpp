#pragma once

#include "World/CPatterned.hpp"
#include "Character/CBoneTracking.hpp"
#include "Weapon/CProjectileInfo.hpp"

namespace urde
{
class CDamageInfo;
class CCollisionActorManager;

namespace MP1
{
class CNewIntroBoss : public CPatterned
{
    pas::ELocomotionType x568_locomotion = pas::ELocomotionType::Relaxed;
    u32 x56c_ = 0;
    float x570_;
    CBoneTracking x574_boneTracking;
    CProjectileInfo x5ac_projectileInfo;
    TUniqueId x5d4_stage1Projectile = kInvalidUniqueId;
    TUniqueId x5d6_stage2Projectile = kInvalidUniqueId;
    TUniqueId x5d8_stage3Projectile = kInvalidUniqueId;
    std::string x5dc_damageLocator; // ???
    std::unique_ptr<CCollisionActorManager> x5ec_collisionManager;
    CAssetId x5f0_;
    CAssetId x5f4_;
    CAssetId x5f8_;
    CAssetId x5fc_;
    TUniqueId x600_headActor = kInvalidUniqueId;
    TUniqueId x602_pelvisActor = kInvalidUniqueId;
    zeus::CVector3f x604_predictedPlayerPos;
    zeus::CVector3f x610_lookPos;
    zeus::CVector3f x61c_;
    float x628_ = 0.f;
    zeus::CVector3f x62c_targetPos;
    float x638_ = 0.2f;
    float x63c_attackTime = 8.f;
    float x640_initialHp = 0.f;
    zeus::CTransform x644_;
    s16 x674_rumbleVoice = -1;
    TUniqueId x676_curProjectile = kInvalidUniqueId;
    bool x678_ = false;
    pas::ELocomotionType GetLocoForHealth(const CStateManager&) const;
    pas::EGenerateType GetGenerateForHealth(const CStateManager&) const;
    float GetNextAttackTime(CStateManager&) const;
    zeus::CVector3f PlayerPos(const CStateManager&) const;
    void DeleteBeam(CStateManager&);
    void StopRumble(CStateManager&);
public:
    DEFINE_PATTERNED(NewIntroBoss)
    CNewIntroBoss(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                  const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                  const CActorParameters& actParms, float, CAssetId, const CDamageInfo& dInfo,
                  CAssetId, CAssetId, CAssetId, CAssetId);

    void Accept(IVisitor &visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager&);
    void Think(float dt, CStateManager& mgr);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    void OnScanStateChanged(EScanState, CStateManager&);
    CProjectileInfo* GetProjectileInfo() { return &x5ac_projectileInfo; }
    zeus::CAABox GetSortingBounds(const CStateManager&) const
    {
        zeus::CAABox box = GetModelData()->GetBounds();
        return zeus::CAABox({-0.5f, -0.5f, box.min.z}, {0.5f, 0.5f, box.max.z}).getTransformedAABox(x34_transform);
    }

    std::experimental::optional<zeus::CAABox> GetTouchBounds() const { return  {}; }
    void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt);
    void Generate(CStateManager&, EStateMsg, float);
    void Attack(CStateManager&, EStateMsg, float);
    void Patrol(CStateManager&, EStateMsg, float);
    bool ShouldTurn(CStateManager&, float);
    bool ShouldAttack(CStateManager&, float);
    bool AIStage(CStateManager&, float);
    bool AnimOver(CStateManager&, float);
    bool InAttackPosition(CStateManager&, float);

};

}
}

