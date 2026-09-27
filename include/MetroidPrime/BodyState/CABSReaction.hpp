#ifndef _CABSREACTION
#define _CABSREACTION

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"

class CABSReaction : public CAdditiveBodyState {
public:
  CABSReaction();
  ~CABSReaction();

  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

  float GetWeight() const { return mWeight; }
  bool IsActive() const { return mActive; }

private:
  float mWeight;
  int mAnim;
  pas::EAdditiveReactionType mType;
  bool mActive;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  void StopAnimation(CBodyController& bc);
};

#endif // _CABSREACTION
