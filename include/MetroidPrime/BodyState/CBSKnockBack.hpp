#ifndef _CBSKNOCKBACK
#define _CBSKNOCKBACK

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSKnockBack : public CBodyState {
public:
  CBSKnockBack();
  ~CBSKnockBack();

  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController&) override;
  bool IsMoving() const override;

private:
  float mCurTime;
  float mRotateSpeed;
  float mRemTime;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};
CHECK_SIZEOF(CBSKnockBack, 0x10)

#endif // _CBSKNOCKBACK
