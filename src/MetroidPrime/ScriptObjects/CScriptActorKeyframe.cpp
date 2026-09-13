#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"

CScriptActorKeyframe::CScriptActorKeyframe(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info, int animId, bool looping,
                                           float lifetime, bool isPassive, int fadeOut, bool active,
                                           float totalPlayback)
: CEntity(uid, info, active, name)
, x34_animationId(animId)
, x38_initialLifetime(lifetime)
, x3c_playbackRate(totalPlayback)
, x40_lifetime(lifetime)
, x44_24_isLooped(looping)
, x44_25_isPassive(isPassive)
, x44_26_fadeOut((fadeOut << 5) & 0x20)
, x44_27_timedLoop((fadeOut << 3) & 0x10)
, x44_28_playing(false) {}

void CScriptActorKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                           CStateManager& mgr) {
  switch (msg) {
  case kSM_Action:
    if (GetActive()) {
      if (!x44_25_isPassive) {
        rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
        for (; conn != GetConnectionList().end(); ++conn) {
          if (conn->x0_state != kSS_Play || conn->x4_msg != kSM_Play) {
            continue;
          }

          CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->x8_objId);
          for (CStateManager::TIdList::const_iterator it = search.first; it != search.second;
               ++it) {
            UpdateEntity(it->second, mgr);
          }
        }
      }

      x44_28_playing = true;
      x40_lifetime = x38_initialLifetime;
      SendScriptMsgs(kSS_Play, mgr, kSM_None);
    }
    break;
  case kSM_InitializedInArea:
    if (x34_animationId == -1)
      x34_animationId = 0;
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptActorKeyframe)

void CScriptActorKeyframe::UpdateEntity(TUniqueId uid, CStateManager& mgr) {
  CEntity* ent = mgr.ObjectById(uid);
  CActor* act = TCastToPtr< CScriptActor >(ent);
  if (!act) {
    act = TCastToPtr< CScriptPlatform >(ent);
  }
  if (act) {
    if (!act->GetActive()) {
      mgr.DeliverScriptMsg(act, GetUniqueId(), kSM_Activate);
    }
    act->SetModelFlags(CModelFlags::Normal());
    if (act->HasAnimation()) {
      if (act->AnimationData()->IsAdditiveAnimation(x34_animationId)) {
        act->AnimationData()->AddAdditiveAnimation(x34_animationId, 1.f, x44_24_isLooped,
                                                   x44_26_fadeOut);
      } else {
        act->AnimationData()->SetAnimation(CAnimPlaybackParms(x34_animationId, -1, 1.f, true),
                                           false);
        act->ModelData()->EnableLooping(x44_24_isLooped);
        act->AnimationData()->MultiplyPlaybackRate(x3c_playbackRate);
      }
    }
  } else if (CPatterned* ai = TCastToPtr< CPatterned >(ent)) {
    if (ai->AnimationData()->IsAdditiveAnimation(x34_animationId)) {
      ai->AnimationData()->AddAdditiveAnimation(x34_animationId, 1.f, x44_24_isLooped,
                                                x44_26_fadeOut);
    } else {
      CBodyStateCmdMgr& cmdMgr = ai->BodyCtrl()->CommandMgr();
      cmdMgr.DeliverCmd(
          CBCScriptedCmd(x34_animationId, x44_24_isLooped, x44_27_timedLoop, x38_initialLifetime));
    }
  }
}

void CScriptActorKeyframe::Think(float dt, CStateManager& mgr) {
  if (!x44_25_isPassive && x44_24_isLooped && x44_27_timedLoop && x44_28_playing &&
      x40_lifetime > 0.f) {

    x40_lifetime -= dt;
    if (x40_lifetime <= 0.f) {

      x44_28_playing = false;
      rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
      for (; conn != GetConnectionList().end(); ++conn) {
        if (conn->x0_state != kSS_Play || conn->x4_msg != kSM_Play) {
          continue;
        }
        TUniqueId uid = mgr.GetIdForScript(conn->x8_objId);
        CEntity* ent = mgr.ObjectById(uid);
        if (CScriptActor* act = TCastToPtr< CScriptActor >(ent)) {
          if (act->HasAnimation()) {
            if (act->AnimationData()->IsAdditiveAnimation(x34_animationId)) {
              act->AnimationData()->DelAdditiveAnimation(x34_animationId);
            } else if (act->AnimationData()->GetCurrentAnimation() == x34_animationId) {
              act->ModelData()->EnableLooping(false);
            }
          }
        } else if (CPatterned* ai = TCastToPtr< CPatterned >(ent)) {
          if (ai->AnimationData()->IsAdditiveAnimation(x34_animationId)) {
            ai->AnimationData()->DelAdditiveAnimation(x34_animationId);

          } else if (ai->BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Scripted &&
                     ai->AnimationData()->GetCurrentAnimation() == x34_animationId) {
            ai->BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
          }
        }
      }
    }
  }

  CEntity::Think(dt, mgr);
}
