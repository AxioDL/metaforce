#include "CScriptActorKeyframe.hpp"
#include "CStateManager.hpp"
#include "World/CScriptActor.hpp"
#include "World/CScriptPlatform.hpp"
#include "World/CAi.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptActorKeyframe::CScriptActorKeyframe(TUniqueId uid, std::string_view name, const CEntityInfo& info, s32 animId,
                                           bool looping, float lifetime, bool b2, u32 w2, bool active, float totalPlayback)
: CEntity(uid, info, active, name)
, x34_animationId(animId)
, x38_initialLifetime(lifetime)
, x3c_playbackRate(totalPlayback)
, x40_lifetime(lifetime)
{
    x44_24_looping = looping;
    x44_25_disableUpdate = b2;
    x44_26_ = w2;
    x44_27_ = w2;
    x44_28_ = false;
    x44_29_ = false;
}

void CScriptActorKeyframe::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptActorKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{

    if (msg == EScriptObjectMessage::Action)
    {
        if (GetActive())
        {
            if (!x44_25_disableUpdate)
            {
                for (const SConnection& conn : x20_conns)
                {
                    if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Play)
                        continue;

                    auto search = mgr.GetIdListForScript(conn.x8_objId);
                    for (auto it = search.first; it != search.second; ++it)
                        UpdateEntity(it->second, mgr);
                }
            }

            x44_28_ = true;
            SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);
        }
    }
    else if (msg == EScriptObjectMessage::InitializedInArea)
    {
        if (x34_animationId == -1)
            x34_animationId = 0;
    }

    CEntity::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptActorKeyframe::Think(float dt, CStateManager& mgr)
{
    if (!x44_25_disableUpdate || !x44_24_looping || !x44_27_ || !x44_28_ || x40_lifetime <= 0.f)
    {
        CEntity::Think(dt, mgr);
        return;
    }

    x40_lifetime -= dt;
    if (x40_lifetime <= 0.f)
    {
        CEntity::Think(dt, mgr);
        return;
    }

    x44_28_ = false;
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg!= EScriptObjectMessage::Play)
            continue;
        CEntity* ent = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId));
        if (TCastToPtr<CScriptActor> act = ent)
        {
            if (act->HasModelData() && act->GetModelData()->HasAnimData())
            {
                CAnimData* animData = act->ModelData()->AnimationData();
                if (animData->IsAdditiveAnimation(x34_animationId))
                    animData->DelAdditiveAnimation(x34_animationId);

                if (animData->GetDefaultAnimation() == x34_animationId)
                    animData->EnableLooping(false);
            }
        }
        else if (TCastToPtr<CAi> ai = ent)
        {
            CAnimData* animData = act->ModelData()->AnimationData();
            if (animData->IsAdditiveAnimation(x34_animationId))
                animData->DelAdditiveAnimation(x34_animationId);
            /* TODO: Finish */
        }
    }

    CEntity::Think(dt, mgr);
}

void CScriptActorKeyframe::UpdateEntity(TUniqueId uid, CStateManager& mgr)
{
    CEntity* ent = mgr.ObjectById(uid);
    CActor* act = nullptr;
    if (TCastToPtr<CScriptActor> tmp = ent)
        act = tmp;
    else if (TCastToPtr<CScriptPlatform> tmp = ent)
        act = tmp;

    if (act)
    {
        if (!act->GetActive())
            mgr.SendScriptMsg(act, GetUniqueId(), EScriptObjectMessage::Activate);
        act->SetDrawFlags({0, 0, 3, zeus::CColor::skWhite});
        if (act->HasModelData() && act->GetModelData()->HasAnimData())
        {
            CAnimData* animData = act->ModelData()->AnimationData();
            if (animData->IsAdditiveAnimation(x34_animationId))
            {
                animData->AddAdditiveAnimation(x34_animationId, 1.f, x44_24_looping, x44_26_);
            }
            else
            {
                animData->SetAnimation(CAnimPlaybackParms(x34_animationId, -1, 1.f, true), false);
                act->ModelData()->EnableLooping(x44_24_looping);
                animData->MultiplyPlaybackRate(x3c_playbackRate);
            }
        }
    }
    /* TODO: Finish */
}
}
