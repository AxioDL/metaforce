#ifndef _CBSLOOPATTACK
#define _CBSLOOPATTACK

#include "MetroidPrime/BodyState/CBodyState.hpp"

class CBSLoopAttack : public CBodyState {
public:
  CBSLoopAttack();

  bool CanShoot() const override { return true; }
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController&) override;

  pas::ELoopState GetState() const { return mState; }
  pas::ELoopAttackType GetAttackType() const { return mLoopAttackType; }
  bool GetAdvance() const { return mAdvance; }

private:
  pas::ELoopState mState;
  pas::ELoopAttackType mLoopAttackType;
  bool mWaitForAnimOver : 1;
  bool mAdvance : 1;

  pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
};

#endif // _CBSLOOPATTACK
