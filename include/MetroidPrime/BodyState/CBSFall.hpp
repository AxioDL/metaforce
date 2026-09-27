#ifndef _CBSFALL
#define _CBSFALL

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSFall : public CBodyState {
public:
  CBSFall();
  ~CBSFall();

  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  float mRotateSpeed;
  float mRemTime;
  pas::EFallState mFallState;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};
CHECK_SIZEOF(CBSFall, 0x10)

#endif // _CBSFALL
