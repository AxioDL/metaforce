#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"

CScriptDock::CScriptDock(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CVector3f& position, const CVector3f& extent, int dock, TAreaId area,
                         bool active, int dockReferenceCount, bool loadConnected)
: CPhysicsActor(
      uid, active, name, info, CTransform4f::Translate(position), CModelData::CModelDataNull(),
      CMaterialList(kMT_Trigger, kMT_Immovable, kMT_AIBlock),
      CAABox(CVector3f(-(0.5f * extent.GetX()), -(0.5f * extent.GetY()), -(0.5f * extent.GetZ())),
             CVector3f(0.5f * extent.GetX(), 0.5f * extent.GetY(), 0.5f * extent.GetZ())),
      SMoverData(1.f), CActorParameters::None(), 0.3f, 0.1f)
, mDockReferenceCount(dockReferenceCount)
, mDock(dock)
, mArea(area)
, mDockState(kDS_InNextRoom)
, mDockReferenced(false)
, mLoadConnected(loadConnected)
, mAreaPostConstructed(false) {}

CScriptDock::~CScriptDock() {}

void CScriptDock::Touch(CActor& actor, CStateManager&) {
  if (mDockState == kDS_InNextRoom) {
    return;
  }
  if (TCastToPtr< CPlayer >(actor)) {
    mDockState = kDS_PlayerTouched;
  }
}

rstl::optional_object< CAABox > CScriptDock::GetTouchBounds() const {
  if (mDockState == kDS_InNextRoom) {
    return rstl::optional_object_null();
  }
  return GetBoundingBox();
}

void CScriptDock::SetLoadConnected(CStateManager& mgr, bool loadConnected) {
  CGameArea* area = mgr.World()->Area(mArea);
  const IGameArea::Dock& dock = area->GetDock(mDock);
  const bool current = dock.GetShouldLoadOther(dock.GetReferenceCount());
  if (loadConnected == current) {
    return;
  }
  area->DockNC(mDock).SetShouldLoadOther(dock.GetReferenceCount(), loadConnected);
}

void CScriptDock::AreaLoaded(CStateManager& mgr) { SetLoadConnected(mgr, mLoadConnected); }

void CScriptDock::AreaUnloaded(CStateManager&) {}

void CWorld::PropogateAreaChain(CGameArea::EOcclusionState state, CGameArea* area, CWorld* world) {
  if (!area->IsPostConstructed()) {
    return;
  }
  if (state == area->GetOcclusionState()) {
    return;
  }

  if (state == CGameArea::kOS_Visible) {
    area->SetOcclusionState(CGameArea::kOS_Visible);
  }
  for (CGameArea::CChainIterator it = world->ChainHead(kC_Alive); it != skGlobalNonConstEnd; ++it) {
    if (&*it != area && it->GetOcclusionState() == CGameArea::kOS_Visible) {
      it->OtherAreaOcclusionChanged();
    }
  }
  for (CGameArea::CChainIterator it = world->ChainHead(kC_Alive); it != skGlobalNonConstEnd; ++it) {
    if (&*it != area && it->GetOcclusionState() == CGameArea::kOS_Occluded) {
      it->OtherAreaOcclusionChanged();
    }
  }
  if (state == CGameArea::kOS_Occluded) {
    area->SetOcclusionState(CGameArea::kOS_Occluded);
  }
}

void CScriptDock::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered: {
    CGameArea* area = mgr.World()->Area(GetAreaId());
    if (area->GetDockCount() <= mDock) {
      return;
    }
    IGameArea::Dock& dock = area->DockNC(mDock);
    if (!dock.IsReferenced()) {
      dock.SetReferenceCount(mDockReferenceCount);
    }
    break;
  }
  case kSM_Deleted:
    AreaUnloaded(mgr);
    break;
  case kSM_InitializedInArea:
    AreaLoaded(mgr);
    break;
  case kSM_WorldInitialized:
    UpdateAreaActivateFlags(mgr);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        GetMaterialFilter().GetIncludeList().Union(CMaterialList(kMT_AIBlock)),
        GetMaterialFilter().GetExcludeList()));
    break;
  case kSM_SetToZero: {
    if (mgr.GetNextAreaId() != mArea) {
      return;
    }
    SetLoadConnected(mgr, false);

    const IGameArea::Dock& dock = mgr.GetWorld()->GetArea(mArea)->GetDock(mDock);
    const TAreaId connectedArea = dock.GetConnectedAreaId(dock.GetReferenceCount());
    CObjectList& objects = mgr.ObjectListById(kOL_PlatformAndDoor);
    for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
      if (CScriptDoor* door = TCastToPtr< CScriptDoor >(objects[i])) {
        if (door->IsConnectedToArea(mgr, connectedArea)) {
          door->ForceClosed(mgr);
        }
      }
    }
    break;
  }
  case kSM_SetToMax:
    if (mgr.GetNextAreaId() != mArea) {
      return;
    }
    SetLoadConnected(mgr, true);
    break;
  case kSM_Increment:
    SetLoadConnected(mgr, true);
  case kSM_Decrement: {
    TAreaId areaId = mArea;
    if (mgr.GetNextAreaId() == mArea) {
      const IGameArea::Dock& dock =
          mgr.GetWorld()->GetArea(mgr.GetNextAreaId())->GetDock(mDock);
      areaId = dock.GetConnectedAreaId(dock.GetReferenceCount());
    }
    if (areaId.Value() >= 0 && mgr.GetWorld()->GetNumAreas() > areaId.Value() &&
        mgr.GetWorld()->IsAreaValid(areaId)) {
      CWorld::PropogateAreaChain(msg == kSM_Increment ? CGameArea::kOS_Visible
                                                      : CGameArea::kOS_Occluded,
                                 mgr.World()->Area(areaId), mgr.World());
    }
    break;
  }
  default:
    CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
    break;
  }
}

void CScriptDock::Think(float dt, CStateManager& mgr) {
  if (mDockReferenced) {
    UpdateAreaActivateFlags(mgr);
    mDockReferenced = false;
  }

  const IGameArea::Dock& gameDock = mgr.GetWorld()->GetArea(mArea)->GetDock(mDock);
  const TAreaId connectedArea = gameDock.GetConnectedAreaId(gameDock.GetReferenceCount());
  if (connectedArea != kInvalidAreaId) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(connectedArea);
    if (mAreaPostConstructed != area.IsLoaded()) {
      mAreaPostConstructed = area.IsLoaded();
      if (mAreaPostConstructed) {
        SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
      } else {
        SendScriptMsgs(kSS_Zero, mgr, kSM_None);
      }
    }
  }

  if (mgr.GetNextAreaId() != mArea) {
    mDockState = kDS_InNextRoom;
  } else if (mDockState == kDS_InNextRoom) {
    mDockState = kDS_InSourceRoom;
  } else if (mDockState == kDS_PlayerTouched) {
    mDockState = kDS_EnterNextArea;
  } else if (mDockState == kDS_EnterNextArea) {
    if (HasPointCrossedDock(mgr, mgr.GetPlayer()->GetTranslation())) {
      const IGameArea::Dock& dock =
          mgr.GetWorld()->GetArea(mgr.GetNextAreaId())->GetDock(mDock);
      const TAreaId nextArea = dock.GetConnectedAreaId(dock.GetReferenceCount());
      if (nextArea != kInvalidAreaId && mgr.GetWorld()->GetArea(nextArea)->IsPostConstructed()) {
        mgr.SetCurrentAreaId(nextArea);
        const int otherDock = dock.GetOtherDockNumber(dock.GetReferenceCount());
        CObjectList& objects = *mgr.GetWorld()->GetArea(nextArea)->ObjectList();
        for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
          CEntity* object = objects[i];
          if (CScriptDock* nextDock = TCastToPtr< CScriptDock >(object)) {
            if (nextDock->GetDockId() == otherDock) {
              nextDock->SetLoadConnected(mgr, true);
              break;
            }
          }
        }
      }
    }
    mDockState = kDS_InSourceRoom;
  }
}

bool CScriptDock::HasPointCrossedDock(const CStateManager& mgr, const CVector3f& point) const {
  const IGameArea::Dock& dock =
      mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()).GetDock(mDock);
  const CVector3f& p2 = dock.GetPoint(2);
  const CVector3f& p1 = dock.GetPoint(1);
  const CVector3f& p0 = dock.GetPoint(0);
  const CPlane plane(p0, p1, p2);
  return plane.IsFacing(point);
}

CPlane CScriptDock::GetPlane(const CStateManager& mgr) const {
  const IGameArea::Dock& dock =
      mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetDock(mDock);
  const CVector3f& p2 = dock.GetPoint(2);
  const CVector3f& p1 = dock.GetPoint(1);
  const CVector3f& p0 = dock.GetPoint(0);
  const CPlane plane(p0, p1, p2);
  return plane;
}

ENTITY_ACCEPT_IMPL(CScriptDock)

int CScriptDock::GetDockReference(const CStateManager& mgr) const {
  return mgr.GetWorld()->GetAreaAlways(mArea).GetDock(mDock).GetReferenceCount();
}

void CScriptDock::SetDockReference(CStateManager& mgr, int reference) {
  mgr.World()->Area(mArea)->DockNC(mDock).SetReferenceCount(reference);
  mDockReferenced = true;
}

void CScriptDock::UpdateAreaActivateFlags(CStateManager& mgr) {
  if (mArea.Value() >= mgr.GetWorld()->GetNumAreas()) {
    return;
  }
  const CGameArea* area = mgr.GetWorld()->GetArea(mArea);
  if (mDock >= area->GetDockCount()) {
    return;
  }

  const IGameArea::Dock& dock = area->GetDock(mDock);
  const int count = dock.GetDockRefs().size();
  for (int i = 0; i < count; ++i) {
    const bool active = dock.GetReferenceCount() == i;
    const TAreaId connectedArea = dock.GetConnectedAreaId(i);
    if (connectedArea != kInvalidAreaId) {
      mgr.World()->Area(connectedArea)->SetActive(active);
    }
  }
  mgr.SetCurrentAreaId(mgr.GetNextAreaId());
}

TAreaId CScriptDock::GetCurrentConnectedAreaId(const CStateManager& mgr) const {
  if (mgr.GetWorld()->GetNumAreas() > mArea.Value()) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetAreaId());
    if (area.GetDockCount() > mDock) {
      const IGameArea::Dock& dock = area.GetDock(mDock);
      return dock.GetConnectedAreaId(dock.GetReferenceCount());
    }
  }
  return kInvalidAreaId;
}
