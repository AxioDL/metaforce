#pragma once

#include "RetroTypes.hpp"
#include "zeus/CQuaternion.hpp"
#include "CharacterCommon.hpp"
#include "CBodyStateCmdMgr.hpp"
#include "CBodyStateInfo.hpp"

namespace urde {

class CActor;
class CAnimPlaybackParms;
struct CFinalInput;
class CPASAnimParmData;
class CRandom16;
class CStateManager;
class CPASDatabase;

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
  union {
    struct {
      bool x300_24_animationOver : 1;
      bool x300_25_active : 1;
      bool x300_26_frozen : 1;
      bool x300_27_hasBeenFrozen : 1;
      bool x300_28_playDeathAnims : 1;
    };
    u32 _dummy = 0;
  };
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
  void EnableAnimation(bool e);
  bool ShouldPlayDeathAnims() const { return x300_28_playDeathAnims; }
  s32 GetCurrentAnimId() const { return x2f8_curAnim; }
  void Activate(CStateManager&);
  CAdditiveBodyState* GetCurrentAdditiveState() { return x2a4_bodyStateInfo.GetCurrentAdditiveState(); }
  void SetState(pas::EAnimationState s) { x2a4_bodyStateInfo.SetState(s); }
  void Update(float, CStateManager&);
  bool ShouldBeHurled() const { return HasBodyState(pas::EAnimationState::Hurled); }
  bool HasBodyState(pas::EAnimationState s) const;
  pas::EFallState GetFallState() const { return x2f0_fallState; }
  void SetFallState(pas::EFallState s) { x2f0_fallState = s; }
  void UpdateBody(float, CStateManager&);
  void SetAdditiveState(pas::EAnimationState s) { x2a4_bodyStateInfo.SetAdditiveState(s); }
  void SetTurnSpeed(float s);
  void SetCurrentAnimation(const CAnimPlaybackParms& parms, bool loop, bool noTrans);
  float GetAnimTimeRemaining() const;
  void SetPlaybackRate(float);
  void MultiplyPlaybackRate(float);
  void SetDeltaRotation(const zeus::CQuaternion& q) { x2dc_rot *= q; }
  void FaceDirection(const zeus::CVector3f&, float);
  void FaceDirection3D(const zeus::CVector3f&, const zeus::CVector3f&, float);
  static bool HasBodyInfo(CActor& act);
  const CPASDatabase& GetPASDatabase() const;
  void PlayBestAnimation(const CPASAnimParmData&, CRandom16&);
  void LoopBestAnimation(const CPASAnimParmData&, CRandom16&);
  void Freeze(float intoFreezeDur, float frozenDur, float breakoutDur);
  void UnFreeze();
  float GetPercentageFrozen() const;
  void SetOnFire(float);
  void DouseFlames();
  void SetElectrocuting(float dur);
  void DouseElectrocuting();
  void UpdateFrozenInfo(float, CStateManager&);
  bool HasIceBreakoutState() const;
  void StopElectrocution();
  void FrozenBreakout();
  pas::EAnimationState GetCurrentAdditiveStateId() const { return x2a4_bodyStateInfo.GetCurrentAdditiveStateId(); }
  EBodyType GetBodyType() const { return x2f4_bodyType; }
  bool HasBeenFrozen() const { return x300_27_hasBeenFrozen; }
  float GetRestrictedFlyerMoveSpeed() const { return x330_restrictedFlyerMoveSpeed; }
  bool GetActive() const { return x300_25_active; }
};
} // namespace urde
