#ifndef _CBSLIEONGROUND
#define _CBSLIEONGROUND

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CActor;

class CBSLieOnGround : public CBodyState {
public:
  explicit CBSLieOnGround(const CActor& actor);
  ~CBSLieOnGround();

  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  uint mHasGroundHit : 1;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};

#endif // _CBSLIEONGROUND
