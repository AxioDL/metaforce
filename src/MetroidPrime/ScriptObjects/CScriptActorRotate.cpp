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
, mRotation(rotation)
, mMaxTime(maxTime)
, mCurrentTime(0.f)
, mUpdateActors(updateActors)
, mUpdateOnCreation(updateOnCreation)
, mUpdateRotation(false)
, mUpdateSpiderBallWaypoints(false) {}

void CScriptActorRotate::UpdatePlatformRiders(rstl::vector< SRiders >& riders,
                                              CScriptPlatform& plat, const CTransform4f& xf,
                                              CStateManager& mgr) {

  for (rstl::vector< SRiders >::iterator rider = riders.begin(); rider != riders.end(); ++rider) {
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(rider->mUid))) {
      act->SetTransform(rider->mTransform);
      act->SetTransform(xf * rider->mTransform);
      act->SetTranslation(act->GetTranslation() + plat.GetTranslation());
      if (!mUpdateRotation) {
        CTransform4f riderXf(act->GetTransform());
        riderXf.SetTranslation(act->GetTranslation() - plat.GetTranslation());
        rider->mTransform = riderXf;
        if (TCastToConstPtr< CScriptSpiderBallWaypoint >(act)) {
          mUpdateSpiderBallWaypoints = true;
        }
      }

      if (CScriptPlatform* plat2 = TCastToPtr< CScriptPlatform >(mgr.ObjectById(rider->mUid))) {
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

  mUpdateSpiderBallWaypoints = false;
}

void CScriptActorRotate::Think(float dt, CStateManager& mgr) {
  if (!mUpdateRotation || !GetActive()) {
    return;
  }

  mCurrentTime += dt;
  if (mCurrentTime >= mMaxTime) {
    mUpdateRotation = false;
    mCurrentTime = mMaxTime;
  }

  const float timeOffset = mCurrentTime / mMaxTime;

  rstl::vector< rstl::pair< TUniqueId, CTransform4f > >::const_iterator it = mActors.begin();
  for (; it != mActors.end(); ++it) {
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(it->first))) {
      const CTransform4f xf =
          CTransform4f::RotateX(CRelAngle::FromDegrees(timeOffset * mRotation.GetX())) *
          CTransform4f::RotateY(CRelAngle::FromDegrees(timeOffset * mRotation.GetY())) *
          CTransform4f::RotateZ(CRelAngle::FromDegrees(timeOffset * mRotation.GetZ()));
      CTransform4f localRot = it->second * xf;
      localRot.SetTranslation(localRot.GetTranslation() + act->GetTranslation());
      act->SetTransform(localRot);

      if (CScriptPlatform* plat = TCastToPtr< CScriptPlatform >(mgr.ObjectById(it->first))) {
        UpdatePlatformRiders(*plat, xf, mgr);
      }
    }
  }

  if (!mUpdateRotation) {
    if (mUpdateSpiderBallWaypoints) {
      RebuildSpiderBallWaypoints(mgr);
    }

    if (mUpdateActors) {
      UpdateActors(false, mgr);
    }
  }
}

void CScriptActorRotate::UpdateActors(bool next, CStateManager& mgr) {
  if (mUpdateRotation) {
    return;
  }
  mActors.clear();

  rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
  for (; conn != GetConnectionList().end(); ++conn) {
    if (conn->mState != kSS_Play || conn->mMsg != kSM_Play) {
      continue;
    }

    CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->mObjId);
    if (!(search.first == search.second)) {
      mActors.reserve(mActors.size() + rstl::distance(search.first, search.second));
      for (CStateManager::TIdList::const_iterator it = search.first; it != search.second; ++it) {
        if (const CActor* act = TCastToConstPtr< CActor >(mgr.ObjectById(it->second))) {
          mActors.push_back(
              rstl::pair< TUniqueId, CTransform4f >(it->second, act->GetTransform().GetRotation()));
        }
      }
    }
  }

  SendScriptMsgs(kSS_Play, mgr, kSM_None);

  if (!mActors.empty()) {
    mUpdateRotation = true;
    if (next) {
      mCurrentTime = mMaxTime;
    } else {
      mCurrentTime = 0.f;
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
    if (!mUpdateOnCreation) {
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
