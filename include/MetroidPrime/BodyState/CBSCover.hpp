#ifndef _CBSCOVER
#define _CBSCOVER

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSCover : public CBodyState {
public:
  CBSCover();
  bool IsMoving() const override { return true; }
  bool ApplyHeadTracking() const override { return false; }
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController&) override;

  pas::ECoverDirection GetCoverDirection() const { return mCoverDirection; }
  bool GetNeedsExit() const { return mNeedsExit; }

private:
  pas::ECoverState mState;
  pas::ECoverDirection mCoverDirection;
  bool mNeedsExit;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};
CHECK_SIZEOF(CBSCover, 0x10)

#endif // _CBSCOVER
