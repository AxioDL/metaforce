#include "CScriptActorRotate.hpp"
#include "CStateManager.hpp"
#include "CScriptPlatform.hpp"
#include "CScriptSpiderBallWaypoint.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptActorRotate::CScriptActorRotate(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                       const zeus::CVector3f& rotation, float maxTime, bool updateActors,
                                       bool updateOnCreation, bool active)
: CEntity(uid, info, active, name)
, x34_rotation(rotation)
, x40_maxTime(maxTime)
, x58_24_updateRotation(false)
, x58_25_skipSpiderBallWaypoints(false)
, x58_26_updateActors(updateActors)
, x58_27_updateOnCreation(updateOnCreation)
{
}

void CScriptActorRotate::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptActorRotate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::Activate)
    {
        CEntity::AcceptScriptMsg(msg, uid, mgr);
        return;
    }

    if (msg == EScriptObjectMessage::Action || msg == EScriptObjectMessage::Next
        || (msg == EScriptObjectMessage::Registered && x58_27_updateOnCreation))
        UpdateActors(msg == EScriptObjectMessage::Next, mgr);

    CEntity::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptActorRotate::Think(float dt, CStateManager& mgr)
{
    if (x58_24_updateRotation && GetActive())
    {
        x44_currentTime += dt;
        if (x44_currentTime >= x40_maxTime)
        {
            x58_24_updateRotation = false;
            x44_currentTime = x40_maxTime;
        }

        float timeOffset = x44_currentTime / x40_maxTime;

        for (const auto& actorPair : x48_actors)
        {
            if (TCastToPtr<CActor> act = mgr.ObjectById(actorPair.first))
            {
                zeus::CTransform xf =
                    zeus::CTransform::RotateX(zeus::degToRad(timeOffset * x34_rotation.x)) *
                    zeus::CTransform::RotateY(zeus::degToRad(timeOffset * x34_rotation.y)) *
                    zeus::CTransform::RotateZ(zeus::degToRad(timeOffset * x34_rotation.z));
                act->SetTransform({xf.basis, act->GetTranslation()});

                if (TCastToPtr<CScriptPlatform> plat = mgr.ObjectById(actorPair.first))
                    UpdatePlatformRiders(*plat.GetPtr(), xf, mgr);
            }
        }

        if (x58_24_updateRotation)
        {
            if (!x58_25_skipSpiderBallWaypoints)
                UpdateSpiderBallWaypoints(mgr);
            if (x58_26_updateActors)
                UpdateActors(false, mgr);
        }
    }
}

void CScriptActorRotate::UpdatePlatformRiders(CScriptPlatform& plat, const zeus::CTransform& xf, CStateManager& mgr)
{
    UpdatePlatformRiders(plat.GetX328(), plat, xf, mgr);
    UpdatePlatformRiders(plat.GetX338(), plat, xf, mgr);
}

void CScriptActorRotate::UpdatePlatformRiders(std::vector<SRiders>& riders, CScriptPlatform& plat,
                                              const zeus::CTransform& xf, CStateManager& mgr)
{
    for (SRiders& rider : riders)
    {
        if (TCastToPtr<CActor> act = mgr.ObjectById(rider.x0_uid))
        {
            zeus::CTransform& riderXf = rider.x8_transform;
            act->SetTransform(rider.x8_transform);
            act->SetTranslation(act->GetTranslation() + plat.GetTranslation());

            if (x58_24_updateRotation)
            {
                riderXf = {act->GetTransform().basis, act->GetTranslation() - plat.GetTranslation()};

                if (TCastToPtr<CScriptSpiderBallWaypoint> wp = act.GetPtr())
                    x58_25_skipSpiderBallWaypoints = true;
            }

            if (TCastToPtr<CScriptPlatform> plat2 = mgr.ObjectById(rider.x0_uid))
                UpdatePlatformRiders(*plat2.GetPtr(), xf, mgr);
        }
    }
}

void CScriptActorRotate::UpdateActors(bool next, CStateManager& mgr)
{
    if (x58_24_updateRotation)
        return;

    x48_actors.clear();

    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state == EScriptObjectState::Play && conn.x4_msg == EScriptObjectMessage::Play)
        {
            auto search = mgr.GetIdListForScript(conn.x8_objId);
            for (auto it = search.first ; it != search.second ; ++it)
            {
                if (TCastToPtr<CActor> act = mgr.ObjectById(it->second))
                    x48_actors[it->second] = act->GetTransform().getRotation();
            }
        }
    }

    SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);

    if (!x48_actors.empty())
    {
        x58_24_updateRotation = true;
        x44_currentTime = (next ? 0.f : x40_maxTime);
    }
}

void CScriptActorRotate::UpdateSpiderBallWaypoints(CStateManager& mgr)
{
    rstl::prereserved_vector<TUniqueId, 1024> waypointIds;
    CObjectList& objectList = mgr.GetAllObjectList();
    for (CEntity* ent : objectList)
    {
        if (TCastToPtr<CScriptSpiderBallWaypoint> wp = ent)
            waypointIds.push_back(wp->GetUniqueId());
    }

    for (const TUniqueId& uid : waypointIds)
    {
        CScriptSpiderBallWaypoint* wp = static_cast<CScriptSpiderBallWaypoint*>(mgr.ObjectById(uid));
        if (wp)
        {
#if 0
             wp->sub_801187B4(mgr);
             wp->xe4_27_ = true;
#endif
        }
    }

    x58_25_skipSpiderBallWaypoints = false;
}
}
