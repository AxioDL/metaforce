#include "MetroidPrime/BodyState/CBSDie.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"

CBSDie::CBSDie() : mRemTime(0.0f), mIsDead(false) {}

void CBSDie::Start(CBodyController& bc, CStateManager& mgr) {
  const CPASDatabase& db = bc.GetPASDatabase();
  int fallState = bc.GetFallState();
  
  bool shouldReset = true;

  if (bc.ShouldPlayDeathAnims()) {
    const CPASAnimParmData parms(pas::kAS_Death, CPASAnimParm::FromEnum(fallState));
    const rstl::pair<float, int> best = db.FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.first > 0.f) {
      const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, false, false);
      mRemTime = bc.GetAnimTimeRemaining();
      shouldReset = false;
    }
  }

  if (shouldReset) {
    bc.EnableAnimation(false);
    mRemTime = bc.ShouldPlayDeathAnims() ? 3.f : 4.f;
  }

  mIsDead = false;
}

pas::EAnimationState CBSDie::UpdateBody(float dt, CBodyController& bc,
                                        CStateManager& mgr) {
  mRemTime -= dt;
  if (mRemTime <= 0.f) {
    bc.EnableAnimation(false);
    mIsDead = true;
  }
  return pas::kAS_Invalid;
}

void CBSDie::Shutdown(CBodyController&) {}

bool CBSDie::IsDying() const { return true; }

bool CBSDie::IsDead() const { return mIsDead; }

CBSDie::~CBSDie() {}
