#include "Runtime/World/CScriptBallTrigger.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CMorphBall.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "DNAMP1/Tweaks/CTweakPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

static zeus::CAABox calculate_ball_aabox() {
  float extent = 0.33f * g_tweakPlayer->GetPlayerBallHalfExtent();
  return {-extent, extent};
}

CScriptBallTrigger::CScriptBallTrigger(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                       const zeus::CVector3f& pos, const zeus::CVector3f& scale, bool active, float f1,
                                       float f2, float f3, const zeus::CVector3f& vec, bool b2)
: CScriptTrigger(uid, name, info, pos, calculate_ball_aabox(), CDamageInfo(CWeaponMode::Power(), 0.f, 0.f, 0.f),
                 zeus::skZero3f, ETriggerFlags::DetectMorphedPlayer, active, false, false)
, x150_force(f1)
, x154_minAngle(f2)
, x158_maxDistance(f3)
, x168_25_stopPlayer(b2) {

  if (vec.canBeNormalized())
    x15c_forceAngle = vec.normalized();
}

void CScriptBallTrigger::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptBallTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Deactivate && GetActive()) {
    mgr.GetPlayer().GetMorphBall()->SetBallBoostState(CMorphBall::EBallBoostState::BoostAvailable);
    x168_24_canApplyForce = false;
  }

  CScriptTrigger::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptBallTrigger::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;
  CScriptTrigger::Think(dt, mgr);
  CPlayer& player = mgr.GetPlayer();

  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    x168_24_canApplyForce = false;
    return;
  }

  const float ballRadius = player.GetMorphBall()->GetBallRadius();
  const zeus::CVector3f radiusPosDif =
      (player.GetTranslation() - (player.GetTranslation() + zeus::CVector3f{0.f, 0.f, ballRadius}));
  const float distance = radiusPosDif.magnitude();

  if (!x168_24_canApplyForce) {
    if (distance < ballRadius)
      x168_24_canApplyForce = true;
    else {
      zeus::CVector3f offset = radiusPosDif.normalized();
      if (std::cos(zeus::degToRad(x154_minAngle)) < (-offset).dot(x15c_forceAngle) && distance < x158_maxDistance) {
        float force = zeus::min((1.f / dt * distance), x150_force * (distance * distance));
        player.ApplyForceWR(force * (player.GetMass() * offset), zeus::CAxisAngle());
      }
    }
  }

  if (x148_28_playerTriggerProc) {
    zeus::CVector3f offset = GetTranslation() - zeus::CVector3f(0.f, 0.f, ballRadius);
    if (x168_25_stopPlayer)
      player.Stop();
    player.MoveToWR(offset, dt);
  } else
    x168_24_canApplyForce = false;
}

void CScriptBallTrigger::InhabitantAdded(CActor& act, CStateManager& /*mgr*/) {
  if (TCastToPtr<CPlayer> player = act)
    player->GetMorphBall()->SetBallBoostState(CMorphBall::EBallBoostState::BoostDisabled);
}

void CScriptBallTrigger::InhabitantExited(CActor& act, CStateManager&) {
  if (TCastToPtr<CPlayer> player = act) {
    player->GetMorphBall()->SetBallBoostState(CMorphBall::EBallBoostState::BoostAvailable);
    x168_24_canApplyForce = false;
  }
}

} // namespace urde
