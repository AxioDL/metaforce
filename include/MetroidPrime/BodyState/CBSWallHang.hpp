#ifndef _CBSWALLHANG
#define _CBSWALLHANG

#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CVector3f.hpp"

class CBSWallHang : public CBodyState {
public:
  ~CBSWallHang() {}

  bool IsInAir(const CBodyController& bc) const override;
  bool IsMoving() const override;
  bool ApplyGravity() const override;
  bool ApplyHeadTracking() const override;
  bool ApplyAnimationDeltas() const override;
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

  CBSWallHang();

private:
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  void FixInPlace(CBodyController& bc);
  bool CheckForLand(CBodyController& bc, CStateManager& mgr);
  bool CheckForWall(CBodyController& bc, CStateManager& mgr);
  void SetLaunchVelocity(CBodyController& bc);

  pas::EWallHangState mState;
  TUniqueId mWpId;
  CVector3f mLaunchVel;
  bool mLaunched : 1;
  bool mNeedsExit : 1;
};
CHECK_SIZEOF(CBSWallHang, 0x1c)

#endif // _CBSWALLHANG
