#pragma once

#include "RetroTypes.hpp"
#include "CharacterCommon.hpp"
#include "CBodyState.hpp"
#include "CAdditiveBodyState.hpp"

namespace urde {
class CActor;

class CBodyStateInfo {
  friend class CBodyController;
  std::map<pas::EAnimationState, std::unique_ptr<CBodyState>> x0_stateMap;
  pas::EAnimationState x14_state = pas::EAnimationState::Invalid;
  CBodyController* x18_bodyController = nullptr;
  std::vector<std::pair<pas::EAnimationState, std::unique_ptr<CAdditiveBodyState>>> x1c_additiveStates;
  pas::EAnimationState x2c_additiveState = pas::EAnimationState::AdditiveIdle;
  float x30_maxPitch = 0.f;
  bool x34_24_changeLocoAtEndOfAnimOnly;
  std::unique_ptr<CBodyState> SetupRestrictedFlyerBodyStates(int stateId, CActor& actor);
  std::unique_ptr<CBodyState> SetupNewFlyerBodyStates(int stateId, CActor& actor);
  std::unique_ptr<CBodyState> SetupWallWalkerBodyStates(int stateId, CActor& actor);
  std::unique_ptr<CBodyState> SetupPitchableBodyStates(int stateId, CActor& actor);
  std::unique_ptr<CBodyState> SetupFlyerBodyStates(int stateId, CActor& actor);
  std::unique_ptr<CBodyState> SetupRestrictedBodyStates(int stateId, CActor& actor);
  std::unique_ptr<CBodyState> SetupBiPedalBodyStates(int stateId, CActor& actor);

public:
  CBodyStateInfo(CActor& actor, EBodyType type);
  float GetLocomotionSpeed(pas::ELocomotionAnim anim) const;
  float GetMaxSpeed() const;
  float GetMaximumPitch() const { return x30_maxPitch; }
  void SetMaximumPitch(float pitch) { x30_maxPitch = pitch; }
  bool GetLocoAnimChangeAtEndOfAnimOnly() const { return x34_24_changeLocoAtEndOfAnimOnly; }
  void SetLocoAnimChangeAtEndOfAnimOnly(bool s) { x34_24_changeLocoAtEndOfAnimOnly = s; }
  CBodyState* GetCurrentState();
  const CBodyState* GetCurrentState() const;
  pas::EAnimationState GetCurrentStateId() const { return x14_state; }
  void SetState(pas::EAnimationState s);
  CAdditiveBodyState* GetCurrentAdditiveState();
  pas::EAnimationState GetCurrentAdditiveStateId() const { return x2c_additiveState; }
  void SetAdditiveState(pas::EAnimationState s);
  bool ApplyHeadTracking() const;
};

} // namespace urde
