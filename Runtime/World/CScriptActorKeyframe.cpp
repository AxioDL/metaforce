#include "Runtime/World/CScriptActorKeyframe.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CScriptActor.hpp"
#include "Runtime/World/CScriptPlatform.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptActorKeyframe::CScriptActorKeyframe(TUniqueId uid, std::string_view name, const CEntityInfo& info, s32 animId,
                                           bool looping, float lifetime, bool isPassive, u32 fadeOut, bool active,
                                           float totalPlayback)
: CEntity(uid, info, active, name)
, x34_animationId(animId)
, x38_initialLifetime(lifetime)
, x3c_playbackRate(totalPlayback)
, x40_lifetime(lifetime)
, x44_24_looping(looping)
, x44_25_isPassive(isPassive)
, x44_26_fadeOut(fadeOut != 0u)
, x44_27_timedLoop(fadeOut != 0u) {}

void CScriptActorKeyframe::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptActorKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Action) {
    if (GetActive()) {
      if (!x44_25_isPassive) {
        for (const SConnection& conn : x20_conns) {
          if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Play)
            continue;

          auto search = mgr.GetIdListForScript(conn.x8_objId);
          for (auto it = search.first; it != search.second; ++it)
            UpdateEntity(it->second, mgr);
        }
      }

      x44_28_playing = true;
      SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);
    }
  } else if (msg == EScriptObjectMessage::InitializedInArea) {
    if (x34_animationId == -1)
      x34_animationId = 0;
  }

  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptActorKeyframe::Think(float dt, CStateManager& mgr) {
  if (x44_25_isPassive || !x44_24_looping || !x44_27_timedLoop || !x44_28_playing || x40_lifetime <= 0.f) {
    CEntity::Think(dt, mgr);
    return;
  }

  x40_lifetime -= dt;
  if (x40_lifetime > 0.f) {
    CEntity::Think(dt, mgr);
    return;
  }

  x44_28_playing = false;
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Play)
      continue;
    CEntity* ent = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId));
    if (TCastToPtr<CScriptActor> act = ent) {
      if (act->HasModelData() && act->GetModelData()->HasAnimData()) {
        CAnimData* animData = act->GetModelData()->GetAnimationData();
        if (animData->IsAdditiveAnimation(x34_animationId))
          animData->DelAdditiveAnimation(x34_animationId);

        if (animData->GetDefaultAnimation() == x34_animationId)
          animData->EnableLooping(false);
      }
    } else if (TCastToPtr<CPatterned> ai = ent) {
      CAnimData* animData = ai->GetModelData()->GetAnimationData();
      if (animData->IsAdditiveAnimation(x34_animationId)) {
        animData->DelAdditiveAnimation(x34_animationId);
      } else if (ai->GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Scripted &&
                 animData->GetDefaultAnimation() == x34_animationId) {
        ai->GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
      }
    }
  }

  CEntity::Think(dt, mgr);
}

void CScriptActorKeyframe::UpdateEntity(TUniqueId uid, CStateManager& mgr) {
  CEntity* ent = mgr.ObjectById(uid);
  CActor* act = nullptr;
  if (TCastToPtr<CScriptActor> tmp = ent)
    act = tmp;
  else if (TCastToPtr<CScriptPlatform> tmp = ent)
    act = tmp;

  if (act) {
    if (!act->GetActive())
      mgr.SendScriptMsg(act, GetUniqueId(), EScriptObjectMessage::Activate);
    act->SetDrawFlags({0, 0, 3, zeus::skWhite});
    if (act->HasModelData() && act->GetModelData()->HasAnimData()) {
      CAnimData* animData = act->GetModelData()->GetAnimationData();
      if (animData->IsAdditiveAnimation(x34_animationId)) {
        animData->AddAdditiveAnimation(x34_animationId, 1.f, x44_24_looping, x44_26_fadeOut);
      } else {
        animData->SetAnimation(CAnimPlaybackParms(x34_animationId, -1, 1.f, true), false);
        act->GetModelData()->EnableLooping(x44_24_looping);
        animData->MultiplyPlaybackRate(x3c_playbackRate);
      }
    }
  } else if (TCastToPtr<CPatterned> ai = ent) {
    CAnimData* animData = ai->GetModelData()->GetAnimationData();
    if (animData->IsAdditiveAnimation(x34_animationId)) {
      animData->AddAdditiveAnimation(x34_animationId, 1.f, x44_24_looping, x44_26_fadeOut);
    } else {
      ai->GetBodyController()->GetCommandMgr().DeliverCmd(
          CBCScriptedCmd(x34_animationId, x44_24_looping, x44_27_timedLoop, x38_initialLifetime));
    }
  }
}
} // namespace urde
