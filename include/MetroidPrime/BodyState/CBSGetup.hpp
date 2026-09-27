#ifndef _CBSGETUP
#define _CBSGETUP

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSGetup : public CBodyState {
public:
  CBSGetup();
  ~CBSGetup();

  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  pas::EFallState mFallState;
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};

#endif // _CBSGETUP
