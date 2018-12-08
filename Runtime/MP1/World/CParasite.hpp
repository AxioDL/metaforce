#pragma once

#include "World/CWallWalker.hpp"
#include "Collision/CCollisionActorManager.hpp"
namespace urde {
class CModelData;
}

namespace urde::MP1 {
class CParasite : public CWallWalker {
  class CRepulsor {
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
  s32 x5e8_stateProgress = -1;
  zeus::CVector3f x5ec_;
  zeus::CVector3f x5f8_targetPos;
  float x604_activeSpeed = 1.f;
  float x608_telegraphRemTime = 0.f;
  float x60c_stuckTime = 0.f;
  zeus::CVector3f x614_lastStuckPos;
  std::unique_ptr<CCollisionActorManager> x620_collisionActorManager;
  TLockedToken<CSkinnedModel> x624_extraModel;
  zeus::CVector3f x628_parasiteSeparationMove;
  zeus::CVector3f x634_parasiteCohesionMove;
  zeus::CVector3f x640_parasiteAlignmentMove;
  CDamageVulnerability x64c_oculusHaltDVuln;
  CDamageInfo x6b4_oculusHaltDInfo;
  float x6d0_maxTelegraphReactDist;
  float x6d4_;
  float x6dc_;
  float x6e0_stuckTimeThreshold;
  float x6e4_parasiteSearchRadius;
  float x6e8_parasiteSeparationDist;
  float x6ec_parasiteSeparationWeight;
  float x6f0_parasiteAlignmentWeight;
  float x6f4_parasiteCohesionWeight;
  float x6f8_destinationSeekWeight;
  float x6fc_forwardMoveWeight;
  float x700_playerSeparationDist;
  float x704_playerSeparationWeight;
  float x708_unmorphedRadius;
  float x710_haltDelay;
  float x714_iceZoomerJointHP;
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
  union {
    struct {
      bool x742_24_receivedTelegraph : 1;
      bool x742_25_jumpVelDirty : 1;
      bool x742_26_ : 1;
      bool x742_27_landed : 1;
      bool x742_28_onGround : 1;
      bool x742_29_ : 1;
      bool x742_30_attackOver : 1;
      bool x742_31_ : 1;
      bool x743_24_halted : 1;
      bool x743_25_vulnerable : 1;
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
  void SetupIceZoomerCollision(CStateManager& mgr);
  void SetupIceZoomerVulnerability(CStateManager& mgr, const CDamageVulnerability& dVuln, const CHealthInfo& hInfo);
  void AddDoorRepulsors(CStateManager& mgr);
  void UpdateCollisionActors(float dt, CStateManager& mgr);
  void DestroyActorManager(CStateManager& mgr);
  void UpdateJumpVelocity();

public:
  DEFINE_PATTERNED(Parasite)
  CParasite(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
            const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, EBodyType bodyType,
            float maxTelegraphReactDist, float advanceWpRadius, float f3, float alignAngVel, float f5,
            float stuckTimeThreshold, float collisionCloseMargin, float parasiteSearchRadius,
            float parasiteSeparationDist, float parasiteSeparationWeight, float parasiteAlignmentWeight,
            float parasiteCohesionWeight, float destinationSeekWeight, float forwardMoveWeight,
            float playerSeparationDist, float playerSeparationWeight, float playerObstructionMinDist, float haltDelay,
            bool disableMove, EWalkerType wType, const CDamageVulnerability& dVuln, const CDamageInfo& parInfo,
            u16 haltSfx, u16 getUpSfx, u16 crouchSfx, CAssetId modelRes, CAssetId skinRes, float iceZoomerJointHP,
            const CActorParameters& aParams);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void PreThink(float, CStateManager&);
  void Think(float dt, CStateManager& mgr);
  void Render(const CStateManager&) const;
  const CDamageVulnerability* GetDamageVulnerability() const;
  CDamageInfo GetContactDamage() const;
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
};
} // namespace urde::MP1
