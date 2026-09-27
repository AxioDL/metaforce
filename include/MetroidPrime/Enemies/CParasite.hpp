#ifndef _CPARASITE
#define _CPARASITE

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CWallWalker.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/list.hpp"
#include "rstl/vector.hpp"

class CActorParameters;
class CCollisionActorManager;
class CHealthInfo;
class CSkinnedModel;
class IVisitor;

class CParasite : public CWallWalker {
public:
  class CRepulsor {
  public:
    CRepulsor(CVector3f pos, float radius) : mPos(pos), mRadius(radius) {}

    const CVector3f& GetPos() const { return mPos; }
    float GetRadius() const { return mRadius; }

  private:
    CVector3f mPos;
    float mRadius;
  };

  CParasite(TUniqueId uid, const rstl::string& name, CPatterned::EFlavorType flavor,
            const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
            const CPatternedInfo& pInfo, EBodyType bodyType, float maxTelegraphReactDist,
            float advanceWpRadius, float f3, float alignAngVel, float f5, float stuckTimeThreshold,
            float collisionCloseMargin, float parasiteSearchRadius, float parasiteSeparationDist,
            float parasiteSeparationWeight, float parasiteAlignmentWeight,
            float parasiteCohesionWeight, float destinationSeekWeight, float forwardMoveWeight,
            float playerSeparationDist, float playerSeparationWeight,
            float playerObstructionMinDist, float haltDelay, bool disableMove,
            CWallWalker::EType wType, const CDamageVulnerability& dVuln, const CDamageInfo& dInfo,
            ushort haltSfx, ushort getUpSfx, ushort crouchSfx, CAssetId modelRes, CAssetId skinRes,
            float iceZoomerJointHP, const CActorParameters& aParams);

  // CEntity
  ~CParasite() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& uid, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CPatterned
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  bool IsOnGround() const override;
  void ThinkAboutMove(float dt) override;
  CDamageInfo GetContactDamage() const override;
  void MassiveDeath(CStateManager& mgr) override;
  void MassiveFrozenDeath(CStateManager& mgr) override;

  // CAi
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Halt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Run(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Crouch(CStateManager& mgr, EStateMsg msg, float dt) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Jump(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool HitSomething(CStateManager& mgr, float arg) override;
  bool Stuck(CStateManager& mgr, float arg) override;
  bool Landed(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;

  // CParasite
  virtual CAdvancementDeltas UpdateWalkerAnimation(CStateManager& mgr, float dt);

private:
  bool CloseToWall(CStateManager& mgr);
  void FaceTarget(CVector3f target);
  TUniqueId RecursiveFindClosestWayPoint(CStateManager& mgr, TUniqueId id, float& dist) const;
  TUniqueId GetClosestWaypointForState(EScriptObjectState state, CStateManager& mgr) const;
  void UpdatePFDestination(CStateManager& mgr);
  void DoFlockingBehavior(CStateManager& mgr);
  void SetupIceZoomerCollision(CStateManager& mgr);
  void SetupIceZoomerVulnerability(CStateManager& mgr, const CDamageVulnerability& dVuln,
                                   const CHealthInfo& hInfo);
  void AddDoorRepulsors(CStateManager& mgr);
  void UpdateCollisionActors(float dt, CStateManager& mgr);
  void DestroyActorManager(CStateManager& mgr);
  void UpdateJumpVelocity();

  static float skAttackVelocity;
  static float skRetreatVelocity;
  static float skAttackTime;
  static float skRetreatTime;

  rstl::vector< CRepulsor > mDoorRepulsors;
  int mStateProgress;
  CVector3f x5ec_;
  CVector3f mTargetPos;
  float mActiveSpeed;
  float mTelegraphRemTime;
  float mStuckTime;
  float x610_;
  CVector3f mLastStuckPos;
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager;
  rstl::single_ptr< TLockedToken< CSkinnedModel > > mExtraModel;
  CVector3f mParasiteSeparationMove;
  CVector3f mParasiteCohesionMove;
  CVector3f mParasiteAlignmentMove;
  CDamageVulnerability mOculusHaltDVuln;
  CDamageInfo mOculusHaltDInfo;
  float mMaxTelegraphReactDist;
  float x6d4_;
  float x6d8_;
  float x6dc_;
  float mStuckTimeThreshold;
  float mParasiteSearchRadius;
  float mParasiteSeparationDist;
  float mParasiteSeparationWeight;
  float mParasiteAlignmentWeight;
  float mParasiteCohesionWeight;
  float mDestinationSeekWeight;
  float mForwardMoveWeight;
  float mPlayerSeparationDist;
  float mPlayerSeparationWeight;
  float mUnmorphedRadius;
  float x70c_;
  float mHaltDelay;
  float mIceZoomerJointHP;
  float x718_;
  float x71c_;
  float x720_;
  float x724_;
  float x728_;
  float x72c_;
  float x730_;
  float x734_;
  float x738_;
  ushort mHaltSfx;
  ushort mGetUpSfx;
  ushort mCrouchSfx;
  bool mReceivedTelegraph : 1;
  bool mJumpVelDirty : 1;
  bool x742_26_ : 1;
  bool mLanded : 1;
  bool mOnGround : 1;
  bool x742_29_ : 1;
  bool mAttackOver : 1;
  bool x742_31_ : 1;
  bool mHalted : 1;
  bool mVulnerable : 1;
  bool mOculusShotAt : 1;
  bool mInJump : 1;
};
CHECK_SIZEOF(CParasite, (VERSION >= VERSION_GM8E_02 ? 0x758 : 0x748))

#endif // _CPARASITE
