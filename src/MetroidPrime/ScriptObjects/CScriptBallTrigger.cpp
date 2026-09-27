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
, mForce(f1)
, mMinAngle(f2)
, mMaxDistance(f3)
, mForceAngle(CVector3f::Zero())
, mCanApplyForce(false)
, mStopPlayer(b2) {

  if (vec.CanBeNormalized()) {
    mForceAngle = vec.AsNormalized();
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
    mCanApplyForce = false;
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

    if (!mCanApplyForce) {
      if (distance < ballRadius) {
        mCanApplyForce = true;
      } else {
        const CVector3f offset = radiusPosDif.AsNormalized();
        // TODO: why doesn't Deg2Rad match?
        float angleCos = cosf(mMinAngle * (1.f * (M_PIF / 180.f)));
        if (angleCos < CVector3f::Dot(-offset, mForceAngle) && distance < mMaxDistance) {
          float a = mForce * (mMaxDistance / (distance * distance));
          float b = 1.f / dt * distance;
          const float force = rstl::min_val(a, b);
          player.ApplyForceWR(force * (player.GetMass() * offset), CAxisAngle::Identity());
        }
      }
    }

    if (mPlayerTriggerProc) {
      const CVector3f offset = GetTranslation() - CVector3f(0.f, 0.f, ballRadius);
      if (mStopPlayer) {
        player.Stop();
      }
      player.MoveToWR(offset, dt);
    }
  } else {
    mCanApplyForce = false;
  }
}

ENTITY_ACCEPT_IMPL(CScriptBallTrigger)

void CScriptBallTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                         CStateManager& mgr) {
  if (msg == kSM_Deactivate && GetActive()) {
    mgr.Player()->MorphBall()->SetBallBoostState(CMorphBall::kBBS_BoostAvailable);
    mCanApplyForce = false;
  }

  CScriptTrigger::AcceptScriptMsg(msg, uid, mgr);
}
