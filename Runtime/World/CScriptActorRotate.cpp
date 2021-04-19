#include "Runtime/World/CScriptActorRotate.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptSpiderBallWaypoint.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {
CScriptActorRotate::CScriptActorRotate(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                       const zeus::CVector3f& rotation, float maxTime, bool updateActors,
                                       bool updateOnCreation, bool active)
: CEntity(uid, info, active, name)
, x34_rotation(rotation)
, x40_maxTime(maxTime)
, x58_26_updateActors(updateActors)
, x58_27_updateOnCreation(updateOnCreation) {}

void CScriptActorRotate::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptActorRotate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Activate) {
    CEntity::AcceptScriptMsg(msg, uid, mgr);
    return;
  }

  if (msg == EScriptObjectMessage::Action || msg == EScriptObjectMessage::Next ||
      (msg == EScriptObjectMessage::Registered && x58_27_updateOnCreation)) {
    UpdateActors(msg == EScriptObjectMessage::Next, mgr);
  }

  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptActorRotate::Think(float dt, CStateManager& mgr) {
  if (x58_24_updateRotation && GetActive()) {
    x44_currentTime += dt;
    if (x44_currentTime >= x40_maxTime) {
      x58_24_updateRotation = false;
      x44_currentTime = x40_maxTime;
    }

    const float timeOffset = x44_currentTime / x40_maxTime;

    for (const auto& actorPair : x48_actors) {
      if (const TCastToPtr<CActor> act = mgr.ObjectById(actorPair.first)) {
        const zeus::CTransform xf = zeus::CTransform::RotateX(zeus::degToRad(timeOffset * x34_rotation.x())) *
                                    zeus::CTransform::RotateY(zeus::degToRad(timeOffset * x34_rotation.y())) *
                                    zeus::CTransform::RotateZ(zeus::degToRad(timeOffset * x34_rotation.z()));
        zeus::CTransform localRot = actorPair.second * xf;
        localRot.origin += act->GetTranslation();
        act->SetTransform(localRot);

        if (const TCastToPtr<CScriptPlatform> plat = mgr.ObjectById(actorPair.first)) {
          UpdatePlatformRiders(*plat.GetPtr(), xf, mgr);
        }
      }
    }

    if (!x58_24_updateRotation) {
      if (x58_25_updateSpiderBallWaypoints) {
        UpdateSpiderBallWaypoints(mgr);
      }

      if (x58_26_updateActors) {
        UpdateActors(false, mgr);
      }
    }
  }
}

void CScriptActorRotate::UpdatePlatformRiders(CScriptPlatform& plat, const zeus::CTransform& xf, CStateManager& mgr) {
  UpdatePlatformRiders(plat.GetStaticSlaves(), plat, xf, mgr);
  UpdatePlatformRiders(plat.GetDynamicSlaves(), plat, xf, mgr);
}

void CScriptActorRotate::UpdatePlatformRiders(std::vector<SRiders>& riders, CScriptPlatform& plat,
                                              const zeus::CTransform& xf, CStateManager& mgr) {
  for (SRiders& rider : riders) {
    if (const TCastToPtr<CActor> act = mgr.ObjectById(rider.x0_uid)) {
      zeus::CTransform& riderXf = rider.x8_transform;
      act->SetTransform(rider.x8_transform);
      act->SetTranslation(act->GetTranslation() + plat.GetTranslation());

      if (!x58_24_updateRotation) {
        riderXf = {act->GetTransform().basis, act->GetTranslation() - plat.GetTranslation()};

        if (TCastToConstPtr<CScriptSpiderBallWaypoint>(act.GetPtr())) {
          x58_25_updateSpiderBallWaypoints = true;
        }
      }

      if (const TCastToPtr<CScriptPlatform> plat2 = mgr.ObjectById(rider.x0_uid)) {
        UpdatePlatformRiders(*plat2.GetPtr(), xf, mgr);
      }
    }
  }
}

void CScriptActorRotate::UpdateActors(bool next, CStateManager& mgr) {
  if (x58_24_updateRotation) {
    return;
  }

  x48_actors.clear();

  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Play) {
      continue;
    }

    auto search = mgr.GetIdListForScript(conn.x8_objId);
    for (auto it = search.first; it != search.second; ++it) {
      if (const TCastToConstPtr<CActor> act = mgr.ObjectById(it->second)) {
        x48_actors.insert_or_assign(it->second, act->GetTransform().getRotation());
      }
    }
  }

  SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);

  if (!x48_actors.empty()) {
    x58_24_updateRotation = true;
    x44_currentTime = (next ? x40_maxTime : 0.f);
  }
}

void CScriptActorRotate::UpdateSpiderBallWaypoints(CStateManager& mgr) {
  rstl::reserved_vector<TUniqueId, 1024> waypointIds;
  CObjectList& objectList = mgr.GetAllObjectList();
  for (CEntity* ent : objectList) {
    if (const TCastToPtr<CScriptSpiderBallWaypoint> wp = ent) {
      waypointIds.push_back(wp->GetUniqueId());
      wp->ClearWaypoints();
    }
  }

  for (const TUniqueId& uid : waypointIds) {
    auto* wp = static_cast<CScriptSpiderBallWaypoint*>(mgr.ObjectById(uid));
    if (wp) {
      wp->BuildWaypointListAndBounds(mgr);
      wp->SetNotInSortedLists(false);
    }
  }

  x58_25_updateSpiderBallWaypoints = false;
}
} // namespace metaforce
