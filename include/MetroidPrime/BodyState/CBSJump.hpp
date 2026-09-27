#ifndef _CBSJUMP
#define _CBSJUMP

#include "MetroidPrime/BodyState/CBodyState.hpp"

#include "Kyoto/Math/CVector3f.hpp"

class CBSJump : public CBodyState {
public:
  CBSJump();
  ~CBSJump() {}

  // CBodyState
  bool IsInAir(const CBodyController& bc) const override;
  bool IsMoving() const override;
  bool ApplyHeadTracking() const override;
  bool ApplyAnimationDeltas() const override;
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  uchar CheckForWallJump(CBodyController& bc, CStateManager& mgr);
  void CheckForLand(CBodyController& bc, CStateManager& mgr);
  void PlayJumpLoop(CStateManager& mgr, CBodyController& bc);

  pas::EJumpState mState;
  pas::EJumpType mJumpType;
  CVector3f mWaypoint1;
  CVector3f mVelocity;
  CVector3f mWaypoint2;
  bool mApplyLaunchVel : 1;
  bool mWallJump : 1;
  bool mWallBounceRight : 1;
  bool mHasWallBounced : 1;
  bool mStartInJumpLoop : 1;
};

CHECK_SIZEOF(CBSJump, 0x34)

#endif // _CBSJUMP
