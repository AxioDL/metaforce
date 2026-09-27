#ifndef _CBSSCRIPTED
#define _CBSSCRIPTED

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSScripted : public CBodyState {
public:
  CBSScripted();
  ~CBSScripted();

  bool ApplyHeadTracking() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController&) override;

private:
  bool mLoopAnim : 1;
  bool mTimedLoop : 1;
  float mRemTime;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};

#endif // _CBSSCRIPTED
