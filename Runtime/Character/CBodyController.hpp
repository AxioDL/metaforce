#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CBodyStateCmdMgr.hpp"
#include "Runtime/Character/CBodyStateInfo.hpp"
#include "Runtime/Character/CharacterCommon.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CActor;
class CAnimPlaybackParms;
class CPASAnimParmData;
class CPASDatabase;
class CRandom16;
class CStateManager;

struct CFinalInput;

class CBodyController {
  CActor& x0_actor;
  CBodyStateCmdMgr x4_cmdMgr;
  CBodyStateInfo x2a4_bodyStateInfo;
  zeus::CQuaternion x2dc_rot;
  pas::ELocomotionType x2ec_locomotionType = pas::ELocomotionType::Relaxed;
  pas::EFallState x2f0_fallState = pas::EFallState::Zero;
  EBodyType x2f4_bodyType;
  s32 x2f8_curAnim = -1;
  float x2fc_turnSpeed;
  bool x300_24_animationOver : 1 = false;
  bool x300_25_active : 1 = false;
  bool x300_26_frozen : 1 = false;
  bool x300_27_hasBeenFrozen : 1 = false;
  bool x300_28_playDeathAnims : 1 = true;
  float x304_intoFreezeDur = 0.f;
  float x308_frozenDur = 0.f;
  float x30c_breakoutDur = 0.f;
  float x310_timeFrozen = 0.f;
  zeus::CVector3f x314_backedUpForce;
  float x320_fireDur = 0.f;
  float x324_electrocutionDur = 0.f;
  float x328_timeOnFire = 0.f;
  float x32c_timeElectrocuting = 0.f;
  float x330_restrictedFlyerMoveSpeed = 0.f;

public:
  CBodyController(CActor& owner, float turnSpeed, EBodyType bodyType);
  pas::EAnimationState GetCurrentStateId() const { return x2a4_bodyStateInfo.GetCurrentStateId(); }
  CBodyStateCmdMgr& GetCommandMgr() { return x4_cmdMgr; }
  const CBodyStateCmdMgr& GetCommandMgr() const { return x4_cmdMgr; }
  void SetDoDeathAnims(bool d) { x300_28_playDeathAnims = d; }
  bool IsElectrocuting() const { return x324_electrocutionDur > 0.f; }
  bool IsOnFire() const { return x320_fireDur > 0.f; }
  bool IsFrozen() const { return x300_26_frozen; }
  const CBodyStateInfo& GetBodyStateInfo() const { return x2a4_bodyStateInfo; }
  CBodyStateInfo& BodyStateInfo() { return x2a4_bodyStateInfo; }
  float GetTurnSpeed() const { return x2fc_turnSpeed; }
  void SetLocomotionType(pas::ELocomotionType type) { x2ec_locomotionType = type; }
  pas::ELocomotionType GetLocomotionType() const { return x2ec_locomotionType; }
  CActor& GetOwner() const { return x0_actor; }
  bool IsAnimationOver() const { return x300_24_animationOver; }
  void EnableAnimation(bool enable);
  bool ShouldPlayDeathAnims() const { return x300_28_playDeathAnims; }
  s32 GetCurrentAnimId() const { return x2f8_curAnim; }
  void Activate(CStateManager& mgr);
  CAdditiveBodyState* GetCurrentAdditiveState() { return x2a4_bodyStateInfo.GetCurrentAdditiveState(); }
  void SetState(pas::EAnimationState state) { x2a4_bodyStateInfo.SetState(state); }
  void Update(float dt, CStateManager& mgr);
  bool ShouldBeHurled() const { return HasBodyState(pas::EAnimationState::Hurled); }
  bool HasBodyState(pas::EAnimationState state) const;
  pas::EFallState GetFallState() const { return x2f0_fallState; }
  void SetFallState(pas::EFallState state) { x2f0_fallState = state; }
  void UpdateBody(float dt, CStateManager& mgr);
  void SetAdditiveState(pas::EAnimationState state) { x2a4_bodyStateInfo.SetAdditiveState(state); }
  void SetTurnSpeed(float speed);
  void SetCurrentAnimation(const CAnimPlaybackParms& parms, bool loop, bool noTrans);
  float GetAnimTimeRemaining() const;
  void SetPlaybackRate(float rate);
  void MultiplyPlaybackRate(float mul);
  void SetDeltaRotation(const zeus::CQuaternion& q) { x2dc_rot *= q; }
  void FaceDirection(const zeus::CVector3f& v0, float dt);
  void FaceDirection3D(const zeus::CVector3f& v0, const zeus::CVector3f& v1, float dt);
  static bool HasBodyInfo(const CActor& actor);
  const CPASDatabase& GetPASDatabase() const;
  void PlayBestAnimation(const CPASAnimParmData& parms, CRandom16& r);
  void LoopBestAnimation(const CPASAnimParmData& parms, CRandom16& r);
  void Freeze(float intoFreezeDur, float frozenDur, float breakoutDur);
  void UnFreeze();
  float GetPercentageFrozen() const;
  void SetOnFire(float duration);
  void DouseFlames();
  void SetElectrocuting(float duration);
  void DouseElectrocuting();
  void UpdateFrozenInfo(float dt, CStateManager& mgr);
  bool HasIceBreakoutState() const;
  void StopElectrocution();
  void FrozenBreakout();
  pas::EAnimationState GetCurrentAdditiveStateId() const { return x2a4_bodyStateInfo.GetCurrentAdditiveStateId(); }
  EBodyType GetBodyType() const { return x2f4_bodyType; }
  bool HasBeenFrozen() const { return x300_27_hasBeenFrozen; }
  float GetRestrictedFlyerMoveSpeed() const { return x330_restrictedFlyerMoveSpeed; }
  void SetRestrictedFlyerMoveSpeed(float speed) { x330_restrictedFlyerMoveSpeed = speed; }
  bool GetActive() const { return x300_25_active; }
};
} // namespace urde
