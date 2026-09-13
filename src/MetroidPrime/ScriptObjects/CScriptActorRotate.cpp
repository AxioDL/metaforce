#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/algorithm.hpp"

CScriptActorRotate::CScriptActorRotate(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CVector3f& rotation,
                                       float maxTime, const bool updateActors,
                                       const bool updateOnCreation, const bool active)
: CEntity(uid, info, active, name)
, x34_rotation(rotation)
, x40_maxTime(maxTime)
, x44_currentTime(0.f)
, x58_26_updateActors(updateActors)
, x58_27_updateOnCreation(updateOnCreation)
, x58_24_updateRotation(false)
, x58_25_updateSpiderBallWaypoints(false) {}

void CScriptActorRotate::UpdatePlatformRiders(rstl::vector< SRiders >& riders,
                                              CScriptPlatform& plat, const CTransform4f& xf,
                                              CStateManager& mgr) {

  for (rstl::vector< SRiders >::iterator rider = riders.begin(); rider != riders.end(); ++rider) {
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(rider->x0_uid))) {
      act->SetTransform(rider->x8_transform);
      act->SetTransform(xf * rider->x8_transform);
      act->SetTranslation(act->GetTranslation() + plat.GetTranslation());
      if (!x58_24_updateRotation) {
        CTransform4f riderXf(act->GetTransform());
        riderXf.SetTranslation(act->GetTranslation() - plat.GetTranslation());
        rider->x8_transform = riderXf;
        if (TCastToConstPtr< CScriptSpiderBallWaypoint >(act)) {
          x58_25_updateSpiderBallWaypoints = true;
        }
      }

      if (CScriptPlatform* plat2 = TCastToPtr< CScriptPlatform >(mgr.ObjectById(rider->x0_uid))) {
        UpdatePlatformRiders(*plat2, xf, mgr);
      }
    }
  }
}

void CScriptActorRotate::UpdatePlatformRiders(CScriptPlatform& plat, const CTransform4f& xf,
                                              CStateManager& mgr) {
  UpdatePlatformRiders(plat.GetStaticSlaves(), plat, xf, mgr);
  UpdatePlatformRiders(plat.GetDynamicSlaves(), plat, xf, mgr);
}

void CScriptActorRotate::RebuildSpiderBallWaypoints(CStateManager& mgr) {
  TEntityList waypointIds;
  CObjectList& objectList = mgr.ObjectListById(kOL_All);
  for (int i = 0; i != waypointIds.capacity(); ++i) {
    if (CScriptSpiderBallWaypoint* wp = TCastToPtr< CScriptSpiderBallWaypoint >(objectList[i])) {
      waypointIds.push_back(wp->GetUniqueId());
      wp->ClearWaypoints();
    }
  }

  for (TEntityList::iterator it = waypointIds.begin(); it != waypointIds.end(); ++it) {
    CScriptSpiderBallWaypoint* wp = static_cast< CScriptSpiderBallWaypoint* >(mgr.ObjectById(*it));
    if (wp) {
      wp->BuildWaypointListAndBounds(mgr);
      wp->SetTransformDirty(true);
    }
  }

  x58_25_updateSpiderBallWaypoints = false;
}

void CScriptActorRotate::Think(float dt, CStateManager& mgr) {
  if (!x58_24_updateRotation || !GetActive()) {
    return;
  }

  x44_currentTime += dt;
  if (x44_currentTime >= x40_maxTime) {
    x58_24_updateRotation = false;
    x44_currentTime = x40_maxTime;
  }

  const float timeOffset = x44_currentTime / x40_maxTime;

  rstl::vector< rstl::pair< TUniqueId, CTransform4f > >::const_iterator it = x48_actors.begin();
  for (; it != x48_actors.end(); ++it) {
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(it->first))) {
      const CTransform4f xf =
          CTransform4f::RotateX(CRelAngle::FromDegrees(timeOffset * x34_rotation.GetX())) *
          CTransform4f::RotateY(CRelAngle::FromDegrees(timeOffset * x34_rotation.GetY())) *
          CTransform4f::RotateZ(CRelAngle::FromDegrees(timeOffset * x34_rotation.GetZ()));
      CTransform4f localRot = it->second * xf;
      localRot.SetTranslation(localRot.GetTranslation() + act->GetTranslation());
      act->SetTransform(localRot);

      if (CScriptPlatform* plat = TCastToPtr< CScriptPlatform >(mgr.ObjectById(it->first))) {
        UpdatePlatformRiders(*plat, xf, mgr);
      }
    }
  }

  if (!x58_24_updateRotation) {
    if (x58_25_updateSpiderBallWaypoints) {
      RebuildSpiderBallWaypoints(mgr);
    }

    if (x58_26_updateActors) {
      UpdateActors(false, mgr);
    }
  }
}

void CScriptActorRotate::UpdateActors(bool next, CStateManager& mgr) {
  if (x58_24_updateRotation) {
    return;
  }
  x48_actors.clear();

  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->x0_state != kSS_Play || conn->x4_msg != kSM_Play) {
      continue;
    }

    CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->x8_objId);
    if (!(search.first == search.second)) {
      x48_actors.reserve(x48_actors.size() + rstl::distance(search.first, search.second));
      for (CStateManager::TIdList::const_iterator it = search.first; it != search.second; ++it) {
        if (const CActor* act = TCastToConstPtr< CActor >(mgr.ObjectById(it->second))) {
          x48_actors.push_back(
              rstl::pair< TUniqueId, CTransform4f >(it->second, act->GetTransform().GetRotation()));
        }
      }
    }
  }

  SendScriptMsgs(kSS_Play, mgr, kSM_None);

  if (!x48_actors.empty()) {
    x58_24_updateRotation = true;
    if (next) {
      x44_currentTime = x40_maxTime;
    } else {
      x44_currentTime = 0.f;
    }
  }
}

void CScriptActorRotate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                         CStateManager& mgr) {
  bool didAccept = false;
  switch (msg) {
  case kSM_Activate:
    CEntity::AcceptScriptMsg(msg, uid, mgr);
    didAccept = true;

  case kSM_Registered:
    if (!x58_27_updateOnCreation) {
      break;
    }
  case kSM_Action:
  case kSM_Next:
    if (GetActive()) {
      UpdateActors(msg == kSM_Next, mgr);
    }
    break;
  }
  if (!didAccept) {
    CEntity::AcceptScriptMsg(msg, uid, mgr);
  }
}

ENTITY_ACCEPT_IMPL(CScriptActorRotate)

CScriptActorRotate::~CScriptActorRotate() {}
