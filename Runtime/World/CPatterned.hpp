#pragma once

#include <memory>
#include <vector>

#include "Runtime/Character/CBodyController.hpp"
#include "Runtime/Character/CSteeringBehaviors.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/World/CAi.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CKnockBackController.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

#ifndef DEFINE_PATTERNED
#define DEFINE_PATTERNED(type) static constexpr ECharacter CharacterType = ECharacter::type;
#endif

namespace urde {
class CPatternedInfo;
class CProjectileInfo;
class CPathFindSearch;

using CPatternedTryFunc = void (CPatterned::*)(CStateManager&, int);

class CPatterned : public CAi {
public:
  static const zeus::CColor skDamageColor;
  enum class ECharacter {
    AtomicAlpha = 0,
    AtomicBeta = 1,
    Babygoth = 2,
    Beetle = 3,
    BloodFlower = 4,
    Burrower = 5,
    ChozoGhost = 6,
    Drone = 7,
    ElitePirate = 8,
    EyeBall = 9,
    FireFlea = 10,
    Flaahgra = 11,
    FlaahgraTentacle = 12,
    FlickerBat = 13,
    FlyingPirate = 14,
    IceSheeegoth = 15,
    JellyZap = 16,
    Magdolite = 17,
    Metaree = 18,
    Metroid = 19,
    MetroidBeta = 20,
    MetroidPrimeExo = 21,
    MetroidPrimeEssence = 22,
    NewIntroBoss = 23,
    Parasite = 24,
    PuddleSpore = 27,
    PuddleToad = 28,
    Puffer = 29,
    Ridley = 30,
    Ripper = 31,
    Seedling = 32,
    SpacePirate = 34,
    SpankWeed = 35,
    PhazonHealingNodule = 35,
    Thardus = 36,
    ThardusRockProjectile = 37,
    Tryclops = 38,
    WarWasp = 39,
    EnergyBall = 40
  };
  enum class EFlavorType { Zero = 0, One = 1, Two = 2 };
  enum class EMovementType { Ground = 0, Flyer = 1 };
  enum class EColliderType { Zero = 0, One = 1 };
  enum class EPatternTranslate { RelativeStart, RelativePlayerStart, RelativePlayer, Absolute };
  enum class EPatternOrient { StartToPlayer, StartToPlayerStart, ReversePlayerForward, Forward };
  enum class EPatternFit { Zero, One };
  enum class EMoveState { Zero, One, Two, Three, Four };
  enum class EBehaviour { Zero };
  enum class EBehaviourOrient { MoveDir, Constant, Destination };
  enum class EBehaviourModifiers { Zero };
  enum class EPatrolState { Invalid = -1, Patrol, Pause, Done };
  enum class EAnimState { NotReady, Ready, Repeat, Over };
  class CPatternNode {
    zeus::CVector3f x0_pos;
    zeus::CVector3f xc_forward;
    float x18_speed;
    u8 x1c_behaviour;
    u8 x1d_behaviourOrient;
    u16 x1e_behaviourModifiers;
    u32 x20_animation;

  public:
    CPatternNode(const zeus::CVector3f& pos, const zeus::CVector3f& forward, float speed, u8 behaviour,
                 u8 behaviourOrient, u16 behaviourModifiers, u32 animation)
    : x0_pos(pos)
    , xc_forward(forward)
    , x18_speed(speed)
    , x1c_behaviour(behaviour)
    , x1d_behaviourOrient(behaviourOrient)
    , x1e_behaviourModifiers(behaviourModifiers)
    , x20_animation(animation) {}
    const zeus::CVector3f& GetPos() const { return x0_pos; }
    const zeus::CVector3f& GetForward() const { return xc_forward; }
    float GetSpeed() const { return x18_speed; }
    u8 GetBehaviour() const { return x1c_behaviour; }
    u8 GetBehaviourOrient() const { return x1d_behaviourOrient; }
    u16 GetBehaviourModifiers() const { return x1e_behaviourModifiers; }
  };

protected:
  EPatrolState x2d8_patrolState = EPatrolState::Invalid;
  TUniqueId x2dc_destObj = kInvalidUniqueId;
  zeus::CVector3f x2e0_destPos;
  zeus::CVector3f x2ec_reflectedDestPos;
  float x2f8_waypointPauseRemTime = 0.f;
  float x2fc_minAttackRange;
  float x300_maxAttackRange;
  float x304_averageAttackTime;
  float x308_attackTimeVariation;
  EBehaviourOrient x30c_behaviourOrient = EBehaviourOrient::MoveDir;
  zeus::CVector3f x310_moveVec;
  zeus::CVector3f x31c_faceVec;
  bool x328_24_inPosition : 1 = false;
  bool x328_25_verticalMovement : 1;
  bool x328_26_solidCollision : 1 = false;
  bool x328_27_onGround : 1;
  bool x328_28_prevOnGround : 1 = true;
  bool x328_29_noPatternShagging : 1 = false;
  bool x328_30_lookAtDeathDir : 1 = true;
  bool x328_31_energyAttractor : 1 = false;
  bool x329_24_ : 1 = true;
  EAnimState x32c_animState = EAnimState::NotReady;
  CStateMachineState x330_stateMachineState;
  ECharacter x34c_character;
  zeus::CVector3f x350_patternStartPos;
  zeus::CVector3f x35c_patternStartPlayerPos;
  zeus::CVector3f x368_destWPDelta;
  EPatternTranslate x374_patternTranslate = EPatternTranslate::RelativeStart;
  EPatternOrient x378_patternOrient = EPatternOrient::ReversePlayerForward;
  EPatternFit x37c_patternFit = EPatternFit::One;
  EBehaviour x380_behaviour = EBehaviour::Zero;
  EBehaviourModifiers x384_behaviourModifiers = EBehaviourModifiers::Zero;
  s32 x388_anim;
  std::vector<CPatternNode> x38c_patterns;
  u32 x39c_curPattern = 0;
  zeus::CVector3f x3a0_latestLeashPosition;
  TUniqueId x3ac_lastPatrolDest = kInvalidUniqueId;
  float x3b0_moveSpeed = 1.f;
  float x3b4_speed;
  float x3b8_turnSpeed;
  float x3bc_detectionRange;
  float x3c0_detectionHeightRange;
  float x3c4_detectionAngle;
  float x3c8_leashRadius;
  float x3cc_playerLeashRadius;
  float x3d0_playerLeashTime;
  float x3d4_curPlayerLeashTime = 0.f;
  float x3d8_xDamageThreshold;
  float x3dc_frozenXDamageThreshold;
  float x3e0_xDamageDelay;
  float x3e4_lastHP = 0.f;
  float x3e8_alphaDelta = 0.f;
  float x3ec_pendingFireDamage = 0.f;
  float x3f0_pendingShockDamage = 0.f;
  float x3f4_burnThinkRateTimer = 0.f;
  EMoveState x3f8_moveState = EMoveState::Zero;
  EFlavorType x3fc_flavor;
  bool x400_24_hitByPlayerProjectile : 1 = false;
  bool x400_25_alive : 1 = true;
  bool x400_26_ : 1 = false;
  bool x400_27_fadeToDeath : 1 = false;
  bool x400_28_pendingMassiveDeath : 1 = false;
  bool x400_29_pendingMassiveFrozenDeath : 1 = false;
  bool x400_30_patternShagged : 1 = false;
  bool x400_31_isFlyer : 1;
  uint32_t x401_24_pathOverCount : 2 = 0;
  bool x401_26_disableMove : 1 = false;
  bool x401_27_phazingOut : 1 = false;
  bool x401_28_burning : 1 = false;
  bool x401_29_laggedBurnDeath : 1 = false;
  bool x401_30_pendingDeath : 1 = false;
  bool x401_31_nextPendingShock : 1 = false;
  bool x402_24_pendingShock : 1 = false;
  bool x402_25_lostMassiveFrozenHP : 1 = false;
  bool x402_26_dieIf80PercFrozen : 1 = false;
  bool x402_27_noXrayModel : 1 = false;
  bool x402_28_isMakingBigStrike : 1 = false;
  bool x402_29_drawParticles : 1 = true;
  bool x402_30_updateThermalFrozenState : 1;
  bool x402_31_thawed : 1 = false;
  bool x403_24_keepThermalVisorState : 1 = false;
  bool x403_25_enableStateMachine : 1 = true;
  bool x403_26_stateControlledMassiveDeath : 1 = true;
  CDamageInfo x404_contactDamage;
  float x420_curDamageRemTime = 0.f;
  float x424_damageWaitTime;
  float x428_damageCooldownTimer = -1.f;
  zeus::CColor x42c_color = zeus::skBlack;
  zeus::CColor x430_damageColor = skDamageColor;
  zeus::CVector3f x434_posDelta;
  zeus::CQuaternion x440_rotDelta;
  std::unique_ptr<CBodyController> x450_bodyController;
  u16 x454_deathSfx;
  u16 x458_iceShatterSfx;
  CSteeringBehaviors x45c_steeringBehaviors;

  CKnockBackController x460_knockBackController;
  zeus::CVector3f x4e4_latestPredictedTranslation;
  float x4f0_predictedLeashTime = 0.f;
  float x4f4_intoFreezeDur;
  float x4f8_outofFreezeDur;
  float x4fc_;
  float x500_preThinkDt = 0.f;
  float x504_damageDur = 0.f;
  EColliderType x508_colliderType;
  float x50c_baseDamageMag;
  std::shared_ptr<CVertexMorphEffect> x510_vertexMorph;
  zeus::CVector3f x514_deathExplosionOffset;
  std::optional<TLockedToken<CGenDescription>> x520_deathExplosionParticle;
  std::optional<TLockedToken<CElectricDescription>> x530_deathExplosionElectric;
  zeus::CVector3f x540_iceDeathExplosionOffset;
  std::optional<TLockedToken<CGenDescription>> x54c_iceDeathExplosionParticle;
  zeus::CVector3f x55c_moveScale = zeus::skOne3f;

  void MakeThermalColdAndHot();
  void UpdateThermalFrozenState(bool thawed);
  void GenerateIceDeathExplosion(CStateManager& mgr);
  void GenerateDeathExplosion(CStateManager& mgr);
  void RenderIceModelWithFlags(const CModelFlags& flags) const;
  TUniqueId GetWaypointForState(CStateManager& mgr, EScriptObjectState state, EScriptObjectMessage msg) const;
  void UpdateActorKeyframe(CStateManager& mgr) const;
  pas::EStepDirection GetStepDirection(const zeus::CVector3f& moveVec) const;
  bool IsPatternObstructed(CStateManager& mgr, const zeus::CVector3f& p0, const zeus::CVector3f& p1) const;
  void UpdateDest(CStateManager& mgr);
  void ApproachDest(CStateManager& mgr);
  std::pair<CScriptWaypoint*, CScriptWaypoint*> GetDestWaypoints(CStateManager& mgr) const;
  zeus::CQuaternion FindPatternRotation(const zeus::CVector3f& dir) const;
  zeus::CVector3f FindPatternDir(CStateManager& mgr) const;
  void UpdatePatternDestPos(CStateManager& mgr);
  void SetupPattern(CStateManager& mgr);
  EScriptObjectState GetDesiredAttackState(CStateManager& mgr) const;
  float GetAnimationDistance(const CPASAnimParmData& data) const;

public:
  CPatterned(ECharacter character, TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
             const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pinfo,
             CPatterned::EMovementType movement, EColliderType collider, EBodyType body, const CActorParameters& params,
             EKnockBackVariant kbVariant);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreThink(float dt, CStateManager& mgr) override {
    x500_preThinkDt = dt;
    CEntity::Think(x500_preThinkDt, mgr);
  }
  void Think(float, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager& mgr) override;

  void CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager& mgr) override;
  void Touch(CActor& act, CStateManager& mgr) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const override { return GetAimPosition(mgr, 0.f); }
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float) const override;
  zeus::CTransform GetLctrTransform(std::string_view name) const;
  zeus::CTransform GetLctrTransform(CSegId id) const;

  bool ApplyBoneTracking() const;

  void DeathDelete(CStateManager& mgr);
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type, bool inDeferred,
                 float magnitude) override;
  void TakeDamage(const zeus::CVector3f&, float arg) override;
  bool FixedRandom(CStateManager&, float arg) override;
  bool Random(CStateManager&, float arg) override;
  bool CodeTrigger(CStateManager&, float arg) override;
  bool FixedDelay(CStateManager&, float arg) override;
  bool RandomDelay(CStateManager&, float arg) override;
  bool Delay(CStateManager&, float arg) override;
  bool PatrolPathOver(CStateManager&, float arg) override;
  bool Stuck(CStateManager&, float arg) override;
  bool AnimOver(CStateManager&, float arg) override;
  bool InPosition(CStateManager&, float arg) override;
  bool HasPatrolPath(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager&, float arg) override;
  bool PatternShagged(CStateManager&, float arg) override;
  bool PatternOver(CStateManager&, float arg) override;
  bool HasRetreatPattern(CStateManager& mgr, float arg) override;
  bool HasAttackPattern(CStateManager& mgr, float arg) override;
  bool NoPathNodes(CStateManager&, float arg) override;
  bool PathShagged(CStateManager&, float arg) override;
  bool PathFound(CStateManager&, float arg) override;
  bool PathOver(CStateManager&, float arg) override;
  bool Landed(CStateManager&, float arg) override;
  bool PlayerSpot(CStateManager&, float arg) override;
  bool SpotPlayer(CStateManager&, float arg) override;
  bool Leash(CStateManager&, float arg) override;
  bool InDetectionRange(CStateManager&, float arg) override;
  bool InMaxRange(CStateManager&, float arg) override;
  bool TooClose(CStateManager&, float arg) override;
  bool InRange(CStateManager&, float arg) override;
  bool OffLine(CStateManager&, float arg) override;
  bool Default(CStateManager&, float arg) override { return true; }
  void PathFind(CStateManager&, EStateMsg msg, float dt) override;
  void Dead(CStateManager&, EStateMsg msg, float dt) override;
  void TargetPlayer(CStateManager&, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager&, EStateMsg msg, float dt) override;
  void FollowPattern(CStateManager&, EStateMsg msg, float dt) override;
  void Patrol(CStateManager&, EStateMsg msg, float dt) override;
  void Start(CStateManager&, EStateMsg msg, float dt) override {}

  void TryCommand(CStateManager& mgr, pas::EAnimationState state, CPatternedTryFunc func, int arg);
  void TryLoopReaction(CStateManager& mgr, int arg);
  void TryProjectileAttack(CStateManager& mgr, int arg);
  void TryMeleeAttack(CStateManager& mgr, int arg);
  void TryGenerate(CStateManager& mgr, int arg);
  void TryGenerateNoXf(CStateManager& mgr, int arg);
  void TryJump(CStateManager& mgr, int arg);
  void TryTurn(CStateManager& mgr, int arg);
  void TryGetUp(CStateManager& mgr, int arg);
  void TryTaunt(CStateManager& mgr, int arg);
  void TryJumpInLoop(CStateManager& mgr, int arg);
  void TryDodge(CStateManager& mgr, int arg);
  void TryRollingDodge(CStateManager& mgr, int arg);
  void TryBreakDodge(CStateManager& mgr, int arg);
  void TryCover(CStateManager& mgr, int arg);
  void TryWallHang(CStateManager& mgr, int arg);
  void TryKnockBack(CStateManager& mgr, int arg);
  void TryGenerateDeactivate(CStateManager& mgr, int arg);
  void TryStep(CStateManager& mgr, int arg);

  virtual bool KnockbackWhenFrozen() const { return true; }
  virtual void MassiveDeath(CStateManager& mgr);
  virtual void MassiveFrozenDeath(CStateManager& mgr);
  virtual void Burn(float duration, float damage);
  virtual void Shock(CStateManager& mgr, float duration, float damage);
  virtual void Freeze(CStateManager& mgr, const zeus::CVector3f& pos, const zeus::CUnitVector3f& dir, float frozenDur);
  virtual void ThinkAboutMove(float);
  virtual CPathFindSearch* GetSearchPath() { return nullptr; }
  virtual CDamageInfo GetContactDamage() const { return x404_contactDamage; }
  virtual u8 GetModelAlphau8(const CStateManager&) const { return u8(x42c_color.a() * 255); }
  virtual bool IsOnGround() const { return x328_27_onGround; }
  virtual float GetGravityConstant() const { return CPhysicsActor::GravityConstant(); }
  virtual CProjectileInfo* GetProjectileInfo() { return nullptr; }
  virtual void PhazeOut(CStateManager&);
  virtual const std::optional<TLockedToken<CGenDescription>>& GetDeathExplosionParticle() const {
    return x520_deathExplosionParticle;
  }
  float GetDamageDuration() const { return x504_damageDur; }
  zeus::CVector3f GetGunEyePos() const;
  bool IsEnergyAttractor() const { return x328_31_energyAttractor; }
  bool IsAlive() const { return x400_25_alive; }

  void BuildBodyController(EBodyType);
  const CBodyController* GetBodyController() const { return x450_bodyController.get(); }
  CBodyController* GetBodyController() { return x450_bodyController.get(); }
  const CKnockBackController& GetKnockBackController() const { return x460_knockBackController; }
  CKnockBackController& GetKnockBackController() { return x460_knockBackController; }
  void SetupPlayerCollision(bool);
  CGameProjectile* LaunchProjectile(const zeus::CTransform& gunXf, CStateManager& mgr, int maxAllowed,
                                    EProjectileAttrib attrib, bool playerHoming,
                                    const std::optional<TLockedToken<CGenDescription>>& visorParticle, u16 visorSfx,
                                    bool sendCollideMsg, const zeus::CVector3f& scale);
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;

  const zeus::CVector3f& GetDestPos() const { return x2e0_destPos; }
  void SetDestPos(const zeus::CVector3f& pos) { x2e0_destPos = pos; }
  void UpdateAlphaDelta(float dt, CStateManager& mgr);
  void SetModelAlpha(float a) { x42c_color.a() = a; }
  float CalcDyingThinkRate() const;
  void UpdateDamageColor(float dt);
  CScriptCoverPoint* GetCoverPoint(CStateManager& mgr, TUniqueId id) const;
  void SetCoverPoint(CScriptCoverPoint* cp, TUniqueId& id);
  void ReleaseCoverPoint(CStateManager& mgr, TUniqueId& id);

  bool MadeSolidCollision() const { return x328_26_solidCollision; }
  bool IsMakingBigStrike() const { return x402_28_isMakingBigStrike; }

  // region Casting Functions

  template <class T>
  static T* CastTo(CEntity* ent) {
    if (TCastToPtr<CPatterned> patterned = ent)
      return CastTo<T>(patterned.GetPtr());
    return nullptr;
  }

  template <class T>
  static const T* CastTo(const CEntity* ent) {
    if (TCastToConstPtr<CPatterned> patterned = ent)
      return CastTo<T>(patterned.GetPtr());
    return nullptr;
  }

  template <class T>
  static T* CastTo(CPatterned* patterned) {
    if (patterned->x34c_character == T::CharacterType)
      return static_cast<T*>(patterned);
    return nullptr;
  }

  template <class T>
  static const T* CastTo(const CPatterned* patterned) {
    if (patterned->x34c_character == T::CharacterType)
      return static_cast<const T*>(patterned);
    return nullptr;
  }

  // endregion
};
} // namespace urde
