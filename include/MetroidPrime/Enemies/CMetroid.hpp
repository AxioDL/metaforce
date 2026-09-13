#ifndef _CMETROID
#define _CMETROID

#include "Collision/CCollidableSphere.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/Enemies/CMetroidData.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

class CSpacePirate;

class CMetroid : public CPatterned {
  static const CDamageVulnerability skGammaRedDamageVulnerability;
  static const CDamageVulnerability skGammaWhiteDamageVulnerability;
  static const CDamageVulnerability skGammaPurpleDamageVulnerability;
  static const CDamageVulnerability skGammaYellowDamageVulnerability;
  static const CDamageVulnerability skStandingFaceHugVulnerability;
  static const CColor skGammaRedColorMod;
  static const CColor skGammaWhiteColorMod;
  static const CColor skGammaPurpleColorMod;
  static const CColor skGammaYellowColorMod;

public:
  CMetroid(const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
           const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
           const CPatternedInfo& pInfo, const CActorParameters& aParms,
           const CMetroidData& metroidData, const TUniqueId other);
  ~CMetroid();
  bool IsAttacking() const { return mIsAttacking; }
  TUniqueId GetAttackTargetId() const { return mAttackTarget; }

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                     const CDamageInfo&) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  void KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool LostInterest(CStateManager& mgr, float arg) override;
  bool PatternShagged(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldWallHang(CStateManager& mgr, float arg) override;
  bool Inside(CStateManager& mgr, float arg) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;
  void WallHang(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;
  bool ShouldDodge(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  bool IsListening() const override { return true; }

private:
  void SpawnGammaMetroid(CStateManager& mgr);
  CMetroidData::EGammaFlavor PickRandomGammaFlavor(CStateManager& mgr,
                                                   CMetroidData::EGammaFlavor previous);
  void ApplySplitGammas(CStateManager& mgr, float dt);
  void ApplyForwardSteering(CStateManager& mgr, const CVector3f& destination);
  void ApplySeparationBehavior(CStateManager& mgr, float distance);
  void SetUpPathFindBehavior(CStateManager& mgr);
  void UpdateTouchBounds();
  void UpdateSoundVolume();
  bool IsSuckingEnergy() const;
  float GetGrowthStage() const;
  float GetDamageMultiplier() const;
  bool ShouldSpawnGammaMetroid() const;
  void ApplyGrowth(float damage, CStateManager& mgr);
  void SwarmAdd(CStateManager& mgr);
  void SwarmRemove(CStateManager& mgr);
  void UpdateAILogicTimers(float dt, CStateManager& mgr);
  bool IsTargetGettingSucked(const CStateManager& mgr) const;
  bool IsBetaMetroidAttackingPlayer(const CStateManager& mgr) const;
  bool IsPlayerInFluid(const CStateManager& mgr) const;
  bool CanStartAttack(CStateManager& mgr) const;
  bool IsPirateValidTarget(const CSpacePirate& pirate, const CStateManager& mgr) const;
  CVector3f GetAttackTargetPos(const CStateManager& mgr) const;
  void SuckEnergyFromTarget(float dt, CStateManager& mgr);
  void PreventWorldCollisions(float dt, CStateManager& mgr);
  void RestoreSolidCollision(CStateManager& mgr);
  bool AttachToTarget(CStateManager& mgr);
  bool PreDamageSpacePirate(CStateManager& mgr);
  void DetachFromTarget(CStateManager& mgr);
  void SetupExitFaceHugDirection(CActor* actor, CStateManager& mgr, const CVector3f& direction,
                                 const CTransform4f& xf);
  void DisableSolidCollision(CMetroid& target);
  bool ShouldReleaseFromTarget(CStateManager& mgr);
  void InterpolateToPosRot(CStateManager& mgr, float dt);
  float ComputeMorphingPlayerSuckUpPos(const CPlayer& player) const;
  void ComputeSuckTargetPosRot(CStateManager& mgr, CVector3f& pos, CQuaternion& rot) const;
  void ComputeSuckPlayerPosRot(CStateManager& mgr, CVector3f& pos, CQuaternion& rot) const;
  void ComputeSuckPiratePosRot(CStateManager& mgr, CVector3f& pos, CQuaternion& rot) const;

  enum EAIState {
    kAiState_Invalid = -1,
    kAiState_Zero,
    kAiState_One,
    kAiState_Two,
    kAiState_Over,
  };

  enum EAttackState {
    kAttackState_None,
    kAttackState_Attached,
    kAttackState_Draining,
    kAttackState_Over,
  };

  EAIState mState;
  CMetroidData mMetroidData;
  TUniqueId mTeamAiManagerId;
  CCollidableSphere mCollisionPrimitive;
  CPathFindSearch mPathFindSearch;
  CVector3f x7a4;
  TUniqueId mAttackTarget;
  float mAttackChance;
  float mTelegraphAttackTime;
  float mEnergyDrained;
  float mEnergyDrainTime;
  float mCollisionRestoreTime;
  EAttackState mAttackState;
  CMetroidData::EGammaFlavor mGammaType;
  CVector3f mScale1;
  CVector3f mScale2;
  CVector3f mScale3;
  float mGrowthDuration;
  float mGrowthEnergy;
  float mLastGrowthEnergy;
  float mSeekTime;
  float mMaxSeekTime;
  float mLoopAttackDistance;
  CVector3f mDetachPos;
  pas::EStepDirection mDodgeDirection;
  CPatternedInfo mPatternedInfo;
  CActorParameters mActorParameters;
  TUniqueId mParent;
  uchar x9be;
  bool mAlert : 1;
  bool mGrowing : 1;
  bool mShotAt : 1;
  bool x9bf_27_ : 1;
  bool x9bf_28_ : 1;
  bool mIsAttacking : 1;
  bool mRestoreSolidCollision : 1;
  bool mRestoreCharacterCollision : 1;
  bool mIsEnergyDrainVulnerable : 1;
};

CHECK_SIZEOF(CMetroid, 0x9C8)

#endif // _CMETROID
