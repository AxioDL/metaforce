#include "MetroidPrime/ScriptObjects/CScriptBallTrigger.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "rstl/math.hpp"

static CAABox calculate_ball_aabox() {
  const float extent = 0.33f * gpTweakPlayer->mPlayerBallHalfExtent;
  return CAABox(CVector3f(-extent, -extent, -extent), CVector3f(extent, extent, extent));
}

CScriptBallTrigger::CScriptBallTrigger(const TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CVector3f& pos,
                                       const CVector3f& scale, bool active, const float f1,
                                       const float f2, const float f3, CVector3f vec,
                                       const bool b2)
: CScriptTrigger(uid, name, info, pos, calculate_ball_aabox(),
                 CDamageInfo(CWeaponMode::Power(), 0.f, 0.f, 0.f), CVector3f::Zero(),
                 kTFL_DetectMorphedPlayer, active, false, false)
, x150_force(f1)
, x154_minAngle(f2)
, x158_maxDistance(f3)
, x15c_forceAngle(CVector3f::Zero())
, x168_24_canApplyForce(false)
, x168_25_stopPlayer(b2) {

  if (vec.CanBeNormalized()) {
    x15c_forceAngle = vec.AsNormalized();
  }
}

CScriptBallTrigger::~CScriptBallTrigger() {}

void CScriptBallTrigger::InhabitantAdded(CActor& act, CStateManager& /*mgr*/) {
  if (CPlayer* player = TCastToPtr< CPlayer >(act)) {
    player->MorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostDisabled);
  }
}

void CScriptBallTrigger::InhabitantExited(CActor& act, CStateManager&) {
  if (CPlayer* player = TCastToPtr< CPlayer >(act)) {
    player->MorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostAvailable);
    x168_24_canApplyForce = false;
  }
}

void CScriptBallTrigger::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CScriptTrigger::Think(dt, mgr);
  CPlayer& player = *mgr.Player();
  const float ballRadius = player.GetMorphBall()->GetBallRadius();
  CVector3f playerTrans = player.GetTranslation() + CVector3f(0.f, 0.f, ballRadius);

  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const CVector3f radiusPosDif = GetTranslation() - playerTrans;
    const float distance = radiusPosDif.Magnitude();

    if (!x168_24_canApplyForce) {
      if (distance < ballRadius) {
        x168_24_canApplyForce = true;
      } else {
        const CVector3f offset = radiusPosDif.AsNormalized();
        // TODO: why doesn't Deg2Rad match?
        float angleCos = cosf(x154_minAngle * (1.f * (M_PIF / 180.f)));
        if (angleCos < CVector3f::Dot(-offset, x15c_forceAngle) && distance < x158_maxDistance) {
          float a = x150_force * (x158_maxDistance / (distance * distance));
          float b = 1.f / dt * distance;
          const float force = rstl::min_val(a, b);
          player.ApplyForceWR(force * (player.GetMass() * offset), CAxisAngle::Identity());
        }
      }
    }

    if (x148_28_playerTriggerProc) {
      const CVector3f offset = GetTranslation() - CVector3f(0.f, 0.f, ballRadius);
      if (x168_25_stopPlayer) {
        player.Stop();
      }
      player.MoveToWR(offset, dt);
    }
  } else {
    x168_24_canApplyForce = false;
  }
}

ENTITY_ACCEPT_IMPL(CScriptBallTrigger)

void CScriptBallTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                         CStateManager& mgr) {
  if (msg == kSM_Deactivate && GetActive()) {
    mgr.Player()->MorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostAvailable);
    x168_24_canApplyForce = false;
  }

  CScriptTrigger::AcceptScriptMsg(msg, uid, mgr);
}
