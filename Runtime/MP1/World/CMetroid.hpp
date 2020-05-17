#pragma once

#include <optional>
#include <string_view>

#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CAnimationParameters.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/MP1/World/CSpacePirate.hpp"

namespace urde::MP1 {

class CMetroidData {
private:
  static constexpr u32 skNumProperties = 20;
  CDamageVulnerability x0_frozenVulnerability;
  CDamageVulnerability x68_energyDrainVulnerability;
  float xd0_energyDrainPerSec;
  float xd4_maxEnergyDrainAllowed;
  float xd8_telegraphAttackTime;
  float xdc_stage2GrowthScale;
  float xe0_stage2GrowthEnergy;
  float xe4_explosionGrowthEnergy;
  std::optional<CAnimationParameters> xe8_animParms1;
  std::optional<CAnimationParameters> xf8_animParms2;
  std::optional<CAnimationParameters> x108_animParms3;
  std::optional<CAnimationParameters> x118_animParms4;
  bool x128_24_startsInWall : 1;

public:
  explicit CMetroidData(CInputStream& in);
  static u32 GetNumProperties() { return skNumProperties; }
  const CDamageVulnerability& GetFrozenVulnerability() const { return x0_frozenVulnerability; }
  const CDamageVulnerability& GetEnergyDrainVulnerability() const { return x68_energyDrainVulnerability; }
  float GetEnergyDrainPerSec() const { return xd0_energyDrainPerSec; }
  float GetMaxEnergyDrainAllowed() const { return xd4_maxEnergyDrainAllowed; }
  float GetTelegraphAttackTime() const { return xd8_telegraphAttackTime; }
  float GetStage2GrowthScale() const { return xdc_stage2GrowthScale; }
  float GetStage2GrowthEnergy() const { return xe0_stage2GrowthEnergy; }
  float GetExplosionGrowthEnergy() const { return xe4_explosionGrowthEnergy; }
  bool GetStartsInWall() const { return x128_24_startsInWall; }
};

class CMetroid : public CPatterned {
private:
  enum class EState {
    Invalid = -1,
    Zero,
    One,
    Two,
    Over,
  } x568_state = EState::Invalid;
  CMetroidData x56c_data;
  TUniqueId x698_teamAiMgrId = kInvalidUniqueId;
  CCollidableSphere x6a0_collisionPrimitive;
  CPathFindSearch x6c0_pathFindSearch;
  zeus::CVector3f x7a4_;
  TUniqueId x7b0_attackTarget = kInvalidUniqueId;
  float x7b4_attackChance = 0.f;
  float x7b8_telegraphAttackTime = 0.f;
  float x7bc_ = 0.f;
  float x7c0_ = 0.f;
  float x7c4_ = 0.f;
  enum class EUnknown {
    Zero,
    One,
    Two,
    Three,
  } x7c8_ = EUnknown::Zero;
  enum class EGammaType {
    Invalid = -1,
    Normal,
    Red,
    White,
    Purple,
    Orange,
  } x7cc_gammaType;
  zeus::CVector3f x7d0_scale1;
  zeus::CVector3f x7dc_scale2;
  zeus::CVector3f x7e8_scale3;
  float x7f4_ = 0.f;
  float x7f8_ = 0.f;
  float x7fc_ = 0.f;
  float x800_ = 0.f;
  float x804_ = 0.f;
  float x808_loopAttackDistance = 0.f;
  zeus::CVector3f x80c_;
  pas::EStepDirection x818_dodgeDirection = pas::EStepDirection::Invalid;
  CPatternedInfo x81c_patternedInfo;
  CActorParameters x954_actParams;
  TUniqueId x9bc_;
  u8 x9be_ = 0;
  bool x9bf_24_alert : 1 = false;
  bool x9bf_25_ : 1 = false;
  bool x9bf_26_shotAt : 1 = false;
  bool x9bf_27_ : 1 = false;
  bool x9bf_28_ : 1 = false;
  bool x9bf_29_isAttacking : 1 = false;
  bool x9bf_30_ : 1 = false;
  bool x9bf_31_ : 1 = false;
  bool x9c0_24_isPlayerMorphed : 1 = false;

public:
  DEFINE_PATTERNED(Metroid)
  CMetroid(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
           const CMetroidData& metroidData, TUniqueId);

  void Accept(IVisitor& visitor) override { visitor.Visit(this); }
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType eType, float dt) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x6a0_collisionPrimitive; }
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& vec1, const zeus::CVector3f& vec2,
                                                         const CWeaponMode& mode,
                                                         EProjectileAttrib attribute) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& vec1, const zeus::CVector3f& vec2,
                                                     const CDamageInfo& dInfo) const override {
    return GetDamageVulnerability();
  }
  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const override;
  CPathFindSearch* GetSearchPath() override { return &x6c0_pathFindSearch; }
  std::optional<zeus::CAABox> GetTouchBounds() const override {
    return x6a0_collisionPrimitive.CalculateAABox(GetTransform());
  }
  bool IsListening() const override { return true; }
  void Render(CStateManager& mgr) override { return CPatterned::Render(mgr); }
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Touch(CActor& act, CStateManager& mgr) override;

  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void KnockBack(const zeus::CVector3f& dir, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                 bool inDeferred, float magnitude) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  //  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  //  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  //  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  //  void WallHang(CStateManager& mgr, EStateMsg msg, float dt) override;

  bool AnimOver(CStateManager&, float arg) override { return x568_state == EState::Over; }
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  //  bool InPosition(CStateManager& mgr, float arg) override;
  //  bool InRange(CStateManager& mgr, float arg) override;
  //  bool Inside(CStateManager& mgr, float arg) override;
  //  bool Leash(CStateManager& mgr, float arg) override;
  //  bool LostInterest(CStateManager& mgr, float arg) override;
  //  bool PatternShagged(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override { return x9bf_26_shotAt; }
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  //  bool ShouldDodge(CStateManager& mgr, float arg) override;
  //  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool ShouldWallHang(CStateManager& mgr, float arg) override { return x56c_data.GetStartsInWall(); }
  //  bool SpotPlayer(CStateManager& mgr, float arg) override;

  bool IsAttacking() const { return x9bf_29_isAttacking; }

private:
  float ComputeMorphingPlayerSuckZPos(const CPlayer& player) const;
  bool IsPirateValidTarget(CSpacePirate* target, CStateManager& mgr);
  bool CanAttack(CStateManager& mgr);
  void UpdateAttackChance(CStateManager& mgr, float dt);
  bool IsPlayerUnderwater(CStateManager& mgr);
  bool IsHunterAttacking(CStateManager& mgr);
  bool IsAttackInProgress(CStateManager& mgr);
  void ComputeSuckPiratePosRot(CStateManager& mgr, zeus::CVector3f& outPos, zeus::CQuaternion& outRot);
  EGammaType GetRandomGammaType(CStateManager& mgr, EGammaType previous);
  void SpawnGammaMetroid(CStateManager& mgr);
  bool ShouldSpawnGammaMetroid();
  void ComputeSuckPlayerPosRot(CStateManager& mgr, zeus::CVector3f& outPos, zeus::CQuaternion& outRot);
  void ComputeSuckTargetPosRot(CStateManager& mgr, zeus::CVector3f& outPos, zeus::CQuaternion& outRot);
  void InterpolateToPosRot(CStateManager& mgr, float dt);
  void SuckEnergyFromTarget(CStateManager& mgr, float dt);
  bool ShouldReleaseFromTarget(CStateManager& mgr);
  void DisableSolidCollision(CMetroid* target);
  void RestoreSolidCollision(CStateManager& mgr);
  void PreventWorldCollisions(CStateManager& mgr, float dt);
  void SetupExitFaceHugDirection(CActor* actor, CStateManager& mgr, const zeus::CVector3f& vec,
                                 const zeus::CTransform& xf);
  void DetachFromTarget(CStateManager& mgr);
  bool AttachToTarget(CStateManager& mgr);
  void SwarmRemove(CStateManager& mgr);
  void SwarmAdd(CStateManager& mgr);
  void ApplySplitGammas(CStateManager& mgr, float arg);
  void ApplyForwardSteering(CStateManager& mgr, const zeus::CVector3f& vec);
  void ApplySeparationBehavior(CStateManager& mgr, float arg);
  void SetUpPathFindBehavior(CStateManager& mgr);
  void ApplyGrowth(float arg);
  bool PreDamageSpacePirate(CStateManager& mgr);
  float GetDamageMultiplier() { return 0.5f * (GetGrowthStage() - 1.f) + 1.f; }
  float GetGrowthStage();
  zeus::CVector3f GetAttackTargetPos(CStateManager& mgr);
  bool IsSuckingEnergy() const;
  void UpdateVolume();
  void UpdateTouchBounds();
};

} // namespace urde::MP1
