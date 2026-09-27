#ifndef _CABSFLINCH
#define _CABSFLINCH

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

class CABSFlinch : public CAdditiveBodyState {
  float mWeight;
  uint mAnim;
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc) const;

public:
  CABSFlinch();
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;
  ~CABSFlinch();
};

#endif // _CABSFLINCH
