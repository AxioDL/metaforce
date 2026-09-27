#ifndef _CBSDIE
#define _CBSDIE

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSDie : public CBodyState {
public:
  CBSDie();
  ~CBSDie();

  bool IsDead() const override;
  bool IsDying() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController&) override;

private:
  float mRemTime;
  bool mIsDead;
};

#endif // _CBSDIE
