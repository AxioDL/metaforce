#pragma once

#include <memory>

#include "Runtime/CRandom16.hpp"
#include "Runtime/Collision/CJointCollisionDescription.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CCollisionActorManager;
class CElementGen;
class CParticleSwoosh;
}

namespace urde::MP1 {

class CMetroidBetaData {
  friend class CMetroidBeta;
  CDamageVulnerability x0_;
  CDamageVulnerability x68_;
  float xd0_;
  float xd4_;
  float xd8_;
  float xdc_;
  float xe0_;
  float xe4_;
  float xe8_;
  float xec_;
  float xf0_;
  CAssetId xf4_;
  CAssetId xf8_;
  CAssetId xfc_;
  CAssetId x100_;
  CAssetId x104_;
  bool x108_24_ : 1;

public:
  explicit CMetroidBetaData(CInputStream&);
};
class CMetroidBeta : public CPatterned {
  s32 x568_progState = -1;
  CMetroidBetaData x56c_metroidBetaData;
  TUniqueId x678_teamMgr = kInvalidUniqueId;
  CPathFindSearch x67c_pathFind;
  u8 x760_ = 0xFF;
  u8 x761_ = 0xFF;
  std::unique_ptr<CCollisionActorManager> x764_collisionManager;
  CCollidableAABox x768_colPrim;
  TUniqueId x790_ = kInvalidUniqueId;
  float x794_ = 0.f;
  float x798_ = 0.f;
  float x79c_ = 0.f;
  float x7a0_ = 0.f;
  float x7a4_ = 0.f;
  zeus::CVector3f x7a8_;
  zeus::CVector3f x7b4_;
  zeus::CVector3f x7c0_;
  zeus::CVector3f x7cc_;
  float x7d8_ = 0.f;
  float x7dc_ = 0.f;
  float x7e0_ = 0.f;
  TToken<CGenDescription> x7e4_;
  TToken<CSwooshDescription> x7f0_;
  TToken<CGenDescription> x7fc_;
  TToken<CGenDescription> x808_;
  TToken<CGenDescription> x814_;
  std::unique_ptr<CElementGen> x820_;
  std::unique_ptr<CParticleSwoosh> x824_;
  std::unique_ptr<CElementGen> x828_;
  std::unique_ptr<CElementGen> x82c_;
  std::unique_ptr<CElementGen> x830_;
  float x834_ = 0.f;
  CRandom16 x838_ = CRandom16(1469);
  float x83c_;
  bool x840_24_ : 1 = false;
  bool x840_25_ : 1 = false;
  bool x840_26_ : 1 = false;
  bool x840_27_ : 1 = false;
  bool x840_28_ : 1 = false;
  bool x840_29_ : 1 = false;
  bool x840_30_ : 1 = false;
  bool x840_31_ : 1 = false;

  void CreateCollisionActorManager(CStateManager& mgr);
  void AddSphereJoints(SSphereJointInfo* sphereJoints, s32 count, std::vector<CJointCollisionDescription>& joints);
  void SetCollisionActorHealthAndVulnerability(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);
  void AddToTeam(CStateManager& mgr);
public:
  DEFINE_PATTERNED(MetroidBeta)
  CMetroidBeta(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
               CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
               const CMetroidBetaData& metroidData);

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Render(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& vec1, const zeus::CVector3f& vec2,
                                                     const CDamageInfo& dInfo) const override;
  void Touch(CActor& act, CStateManager& mgr) override;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType eType, float dt) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  void CollidedWith(TUniqueId collidee, const CCollisionInfoList& info, CStateManager& mgr) override;
  bool IsListening() const override { return true; }
  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void WallHang(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool PathShagged(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool ShouldWallHang(CStateManager& mgr, float arg) override;
  bool StartAttack(CStateManager& mgr, float arg) override;
  bool BreakAttack(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  CPathFindSearch* GetSearchPath() override { return &x67c_pathFind; }

  void RenderHitGunEffect() const;
  void RenderHitBallEffect() const;
};

} // namespace urde::MP1
