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
, mAnimationId(animId)
, mInitialLifetime(lifetime)
, mPlaybackRate(totalPlayback)
, mLifetime(lifetime)
, mIsLooped(looping)
, mIsPassive(isPassive)
, mFadeOut((fadeOut << 5) & 0x20)
, mTimedLoop((fadeOut << 3) & 0x10)
, mPlaying(false) {}

void CScriptActorKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                           CStateManager& mgr) {
  switch (msg) {
  case kSM_Action:
    if (GetActive()) {
      if (!mIsPassive) {
        rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
        for (; conn != GetConnectionList().end(); ++conn) {
          if (conn->mState != kSS_Play || conn->mMsg != kSM_Play) {
            continue;
          }

          CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->mObjId);
          for (CStateManager::TIdList::const_iterator it = search.first; it != search.second;
               ++it) {
            UpdateEntity(it->second, mgr);
          }
        }
      }

      mPlaying = true;
      mLifetime = mInitialLifetime;
      SendScriptMsgs(kSS_Play, mgr, kSM_None);
    }
    break;
  case kSM_InitializedInArea:
    if (mAnimationId == -1)
      mAnimationId = 0;
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
      if (act->AnimationData()->IsAdditiveAnimation(mAnimationId)) {
        act->AnimationData()->AddAdditiveAnimation(mAnimationId, 1.f, mIsLooped,
                                                   mFadeOut);
      } else {
        act->AnimationData()->SetAnimation(CAnimPlaybackParms(mAnimationId, -1, 1.f, true),
                                           false);
        act->ModelData()->EnableLooping(mIsLooped);
        act->AnimationData()->MultiplyPlaybackRate(mPlaybackRate);
      }
    }
  } else if (CPatterned* ai = TCastToPtr< CPatterned >(ent)) {
    if (ai->AnimationData()->IsAdditiveAnimation(mAnimationId)) {
      ai->AnimationData()->AddAdditiveAnimation(mAnimationId, 1.f, mIsLooped,
                                                mFadeOut);
    } else {
      CBodyStateCmdMgr& cmdMgr = ai->BodyCtrl()->CommandMgr();
      cmdMgr.DeliverCmd(
          CBCScriptedCmd(mAnimationId, mIsLooped, mTimedLoop, mInitialLifetime));
    }
  }
}

void CScriptActorKeyframe::Think(float dt, CStateManager& mgr) {
  if (!mIsPassive && mIsLooped && mTimedLoop && mPlaying &&
      mLifetime > 0.f) {

    mLifetime -= dt;
    if (mLifetime <= 0.f) {

      mPlaying = false;
      rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
      for (; conn != GetConnectionList().end(); ++conn) {
        if (conn->mState != kSS_Play || conn->mMsg != kSM_Play) {
          continue;
        }
        TUniqueId uid = mgr.GetIdForScript(conn->mObjId);
        CEntity* ent = mgr.ObjectById(uid);
        if (CScriptActor* act = TCastToPtr< CScriptActor >(ent)) {
          if (act->HasAnimation()) {
            if (act->AnimationData()->IsAdditiveAnimation(mAnimationId)) {
              act->AnimationData()->DelAdditiveAnimation(mAnimationId);
            } else if (act->AnimationData()->GetCurrentAnimation() == mAnimationId) {
              act->ModelData()->EnableLooping(false);
            }
          }
        } else if (CPatterned* ai = TCastToPtr< CPatterned >(ent)) {
          if (ai->AnimationData()->IsAdditiveAnimation(mAnimationId)) {
            ai->AnimationData()->DelAdditiveAnimation(mAnimationId);

          } else if (ai->BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Scripted &&
                     ai->AnimationData()->GetCurrentAnimation() == mAnimationId) {
            ai->BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
          }
        }
      }
    }
  }

  CEntity::Think(dt, mgr);
}
