#pragma once

#include <memory>
#include <string>

#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CCollisionActorManager;
class CDamageInfo;

namespace MP1 {
class CNewIntroBoss : public CPatterned {
  pas::ELocomotionType x568_locomotion = pas::ELocomotionType::Relaxed;
  u32 x56c_stateProg = 0;
  float x570_minTurnAngle;
  CBoneTracking x574_boneTracking;
  CProjectileInfo x5ac_projectileInfo;
  TUniqueId x5d4_stage1Projectile = kInvalidUniqueId;
  TUniqueId x5d6_stage2Projectile = kInvalidUniqueId;
  TUniqueId x5d8_stage3Projectile = kInvalidUniqueId;
  std::string x5dc_damageLocator; // ???
  std::unique_ptr<CCollisionActorManager> x5ec_collisionManager;
  CAssetId x5f0_beamContactFxId;
  CAssetId x5f4_beamPulseFxId;
  CAssetId x5f8_beamTextureId;
  CAssetId x5fc_beamGlowTextureId;
  TUniqueId x600_headActor = kInvalidUniqueId;
  TUniqueId x602_pelvisActor = kInvalidUniqueId;
  zeus::CVector3f x604_predictedPlayerPos;
  zeus::CVector3f x610_lookPos;
  zeus::CVector3f x61c_startPlayerPos;
  float x628_firingTime = 0.f;
  zeus::CVector3f x62c_targetPos;
  float x638_ = 0.2f;
  float x63c_attackTime = 8.f;
  float x640_initialHp = 0.f;
  zeus::CTransform x644_initialXf;
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
  CNewIntroBoss(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, float minTurnAngle,
                CAssetId projectile, const CDamageInfo& dInfo, CAssetId beamContactFxId, CAssetId beamPulseFxId,
                CAssetId beamTextureId, CAssetId beamGlowTextureId);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager&) override;
  void Think(float dt, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  void OnScanStateChanged(EScanState, CStateManager&) override;
  CProjectileInfo* GetProjectileInfo() override { return &x5ac_projectileInfo; }
  zeus::CAABox GetSortingBounds(const CStateManager&) const override {
    zeus::CAABox box = GetModelData()->GetBounds();
    return zeus::CAABox({-0.5f, -0.5f, box.min.z()}, {0.5f, 0.5f, box.max.z()}).getTransformedAABox(x34_transform);
  }

  std::optional<zeus::CAABox> GetTouchBounds() const override { return {}; }
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt) override;
  void Generate(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void Patrol(CStateManager&, EStateMsg, float) override;
  bool ShouldTurn(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;
  bool AIStage(CStateManager&, float) override;
  bool AnimOver(CStateManager&, float) override;
  bool InAttackPosition(CStateManager&, float) override;
};

} // namespace MP1
} // namespace urde
