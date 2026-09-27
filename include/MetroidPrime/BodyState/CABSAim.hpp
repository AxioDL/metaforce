#ifndef _CABSAIM
#define _CABSAIM

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

class CABSAim : public CAdditiveBodyState {
public:
  CABSAim();
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

private:
  bool mNeedsIdle;
  int mAnims[4];
  float mAngles[4];
  float mHWeight;
  float mHWeightVel;
  float mVWeight;
  float mVWeightVel;
  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};

#endif // _CABSAIM
