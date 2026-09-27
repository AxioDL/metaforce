#ifndef _CBODYCONTROLLER
#define _CBODYCONTROLLER

#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"

#include "Kyoto/Math/CQuaternion.hpp"

class CActor;
class CPASDatabase;
class CAnimPlaybackParms;
class CPASAnimParmData;
class CRandom16;
class CStateManager;

class CBodyController {
public:
  CBodyController(CActor& actor, float turnSpeed, EBodyType bodyType);

  CActor& GetOwner() const { return *mActor; }
  CBodyStateCmdMgr& CommandMgr() { return mCmdMgr; }
  const CBodyStateCmdMgr& GetCommandMgr() const { return mCmdMgr; }

  const CPASDatabase& GetPASDatabase() const;
  float GetAnimTimeRemaining() const;
  void SetDeltaRotation(const CQuaternion& q);
  void SetCurrentAnimation(const CAnimPlaybackParms& parms, bool loop, bool noTrans);
  void FaceDirection(const CVector3f& v0, float dt);
  void FaceDirection3D(const CVector3f& v0, const CVector3f& v1, float dt);
  void EnableAnimation(bool enable);
  void PlayBestAnimation(const CPASAnimParmData& parms, CRandom16& r);
  void LoopBestAnimation(const CPASAnimParmData& parms, CRandom16& r);
  bool HasIceBreakoutState();
  void Activate(CStateManager& mgr);
  void Update(float dt, CStateManager& mgr);
  void UpdateBody(float dt, CStateManager& mgr);
  void UpdateFrozenInfo(float dt, CStateManager& mgr);
  bool HasBodyState(pas::EAnimationState state) const;
  void SetLocomotionType(pas::ELocomotionType type);
  float GetPercentageFrozen() const;

  void SetPlaybackRate(float rate);
  void SetFallState(pas::EFallState state); // { x2f0_fallState = state; }
  pas::EFallState GetFallState() const;     // { return x2f0_fallState; }
  const CBodyStateInfo& GetBodyStateInfo() const { return mBodyStateInfo; }
  CBodyStateInfo& BodyStateInfo() { return mBodyStateInfo; }
  pas::ELocomotionType GetLocomotionType() const { return mLocomotionType; }
  EBodyType GetBodyType() const { return mBodyType; }
  int GetCurrentAnimId() const { return mCurAnim; }
  void SetTurnSpeed(float speed);
  float GetTurnSpeed() const { return mTurnSpeed; }
  bool IsAnimationOver() const { return mAnimationOver; }
  bool GetIsActive() const { return mActive; }
  bool ShouldPlayDeathAnims() const { return mPlayDeathAnims; }
  pas::EAnimationState GetCurrentStateId() const { return mBodyStateInfo.GetCurrentStateId(); }
  float GetRestrictedFlyerMoveSpeed() const { return mRestrictedFlyerMoveSpeed; }
  void SetRestrictedFlyerMoveSpeed(float speed) { mRestrictedFlyerMoveSpeed = speed; }

  void Freeze(float intoFreezeDur, float frozenDur, float breakoutDur);
  void UnFreeze();
  void FrozenBreakout();
  void MultiplyPlaybackRate(float scale);
  void SetOnFire(float duration);
  void DouseFlames();
  bool HasBeenFrozen() const { return mHasBeenFrozen; }
  bool IsFrozen() const { return mFrozen; }
  void SetElectrocuting(float duration);
  bool IsElectrocuting() const { return mElectrocutionDur > 0.f; }
  bool IsOnFire() const { return mFireDur > 0.f; }
  void StopElectrocution();

private:
  CActor* mActor;
  CBodyStateCmdMgr mCmdMgr;
  CBodyStateInfo mBodyStateInfo;
  CQuaternion mRot;
  pas::ELocomotionType mLocomotionType;
  pas::EFallState mFallState;
  EBodyType mBodyType;
  int mCurAnim;
  float mTurnSpeed;
  bool mAnimationOver : 1;
  bool mActive : 1;
  bool mFrozen : 1;
  bool mHasBeenFrozen : 1;
  bool mPlayDeathAnims : 1;
  float mIntoFreezeDur;
  float mFrozenDur;
  float mBreakoutDur;
  float mTimeFrozen;
  CVector3f mBackedUpForce;
  float mFireDur;
  float mElectrocutionDur;
  float mTimeOnFire;
  float mTimeElectrocuting;
  float mRestrictedFlyerMoveSpeed;
};
CHECK_SIZEOF(CBodyController, 0x334)

#endif // _CBODYCONTROLLER
