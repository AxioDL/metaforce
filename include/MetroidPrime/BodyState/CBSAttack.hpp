#ifndef _CBSATTACK
#define _CBSATTACK

#include "MetroidPrime/BodyState/CBodyState.hpp"

#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"

#include "Kyoto/Math/CVector3f.hpp"

class CBSAttack : public CBodyState {
public:
  CBSAttack();
  
  bool CanShoot() const override { return false; }
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController&) override;

private:
  pas::EAnimationState mNextState; // = pas::kAS_Invalid;
  CBCSlideCmd mSlide;
  CVector3f mTargetPos;
  float mAlignTargetPosStartTime; // = -1.f;
  float mAlignTargetPosTime;      // = -1.f;
  float mCurTime;                 // = 0.f;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  void UpdatePhysicsActor(CBodyController& bc, float dt);
};

#endif // _CBSATTACK
