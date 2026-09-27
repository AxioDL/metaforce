#ifndef _CBODYSTATEINFO
#define _CBODYSTATEINFO

#include "rstl/map.hpp"

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CAdditiveBodyState;
class CBodyController;
class CBodyState;
class CActor;

class CBodyStateInfo {
public:
  CBodyStateInfo(CActor& actor, EBodyType type);
  ~CBodyStateInfo();

  void SetBodyController(CBodyController* controller) { mBodyController = controller; }
  const float& GetMaximumPitch() const { return mMaxPitch; }
  void SetMaximumPitch(float pitch) { mMaxPitch = pitch; }
  bool GetLocoAnimChangeAtEndOfAnimOnly() const { return mChangeLocoAtEndOfAnimOnly; }
  void SetLocoAnimChangeAtEndOfAnimOnly(bool s) { mChangeLocoAtEndOfAnimOnly = s; }
  pas::EAnimationState GetCurrentStateId() const { return mState; }
  pas::EAnimationState GetCurrentAdditiveStateId() const { return mAdditiveState; }

  void SetState(pas::EAnimationState s);
  const CBodyState* GetCurrentState() const;
  CBodyState* GetCurrentState();
  bool ApplyHeadTracking() const;
  bool IsDead() const { return GetCurrentState()->IsDead(); }
  bool IsInAir() const { return GetCurrentState()->IsInAir(*mBodyController); }
  void SetAdditiveState(pas::EAnimationState s);
  CAdditiveBodyState* GetCurrentAdditiveState();
  float GetMaxSpeed() const;
  float GetLocomotionSpeed(pas::ELocomotionAnim anim) const;

private:
  friend class CBodyController;
  rstl::map< int, CBodyState* > mStateMap;
  pas::EAnimationState mState;
  CBodyController* mBodyController;
  rstl::vector< rstl::pair< int, rstl::auto_ptr< CAdditiveBodyState > > > mAdditiveStates;
  pas::EAnimationState mAdditiveState;
  float mMaxPitch;
  bool mChangeLocoAtEndOfAnimOnly : 1;

  CBodyState* SetupRestrictedFlyerBodyStates(int stateId, CActor& actor);
  CBodyState* SetupNewFlyerBodyStates(int stateId, CActor& actor);
  CBodyState* SetupWallWalkerBodyStates(int stateId, CActor& actor);
  CBodyState* SetupFlyerBodyStates(int stateId, CActor& actor);
  CBodyState* SetupPitchableFlyerBodyStates(int stateId, CActor& actor);
  CBodyState* SetupRestrictedBodyStates(int stateId, CActor& actor);
  CBodyState* SetupBiPedalBodyStates(int stateId, CActor& actor);
};
CHECK_SIZEOF(CBodyStateInfo, 0x38)

#endif // _CBODYSTATEINFO
