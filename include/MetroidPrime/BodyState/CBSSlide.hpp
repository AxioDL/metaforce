#ifndef _CBSSLIDE
#define _CBSSLIDE

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSSlide : public CBodyState {
public:
  CBSSlide();
  ~CBSSlide() override;

  bool ApplyHeadTracking() const override;
  bool IsMoving() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController&) override;

private:
  float mRotateSpeed;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};
CHECK_SIZEOF(CBSSlide, 0x8)

#endif // _CBSSLIDE
