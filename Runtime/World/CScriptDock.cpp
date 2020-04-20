#include "Runtime/World/CScriptDock.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Collision/CMaterialList.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptDoor.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

constexpr auto skDockMaterialList =
    CMaterialList{EMaterialTypes::Trigger, EMaterialTypes::Immovable, EMaterialTypes::AIBlock};

CScriptDock::CScriptDock(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CVector3f& position,
                         const zeus::CVector3f& extents, s32 dock, TAreaId area, bool active, s32 dockReferenceCount,
                         bool loadConnected)
: CPhysicsActor(uid, active, name, info, zeus::CTransform(zeus::CMatrix3f(), position), CModelData::CModelDataNull(),
                skDockMaterialList, zeus::CAABox(-extents * 0.5f, extents * 0.5f), SMoverData(1.f),
                CActorParameters::None(), 0.3f, 0.1f)
, x258_dockReferenceCount(dockReferenceCount)
, x25c_dock(dock)
, x260_area(area)
, x268_25_loadConnected(loadConnected) {}

void CScriptDock::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptDock::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    UpdateAreaActivateFlags(mgr);
    x268_24_dockReferenced = false;
  }

  const IGameArea::Dock* gameDock = mgr.GetWorld()->GetArea(x260_area)->GetDock(x25c_dock);
  TAreaId connArea = gameDock->GetConnectedAreaId(gameDock->GetReferenceCount());
  if (connArea != kInvalidAreaId) {
    bool connPostConstructed = mgr.GetWorld()->GetArea(connArea)->IsPostConstructed();
    if (x268_26_areaPostConstructed != connPostConstructed) {
      x268_26_areaPostConstructed = connPostConstructed;
      if (connPostConstructed)
        CEntity::SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
      else
        CEntity::SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
    }
  }

  if (mgr.GetNextAreaId() != x260_area)
    x264_dockState = EDockState::Three;
  else if (x264_dockState == EDockState::Three)
    x264_dockState = EDockState::Idle;
  else if (x264_dockState == EDockState::PlayerTouched)
    x264_dockState = EDockState::EnterNextArea;
  else if (x264_dockState == EDockState::EnterNextArea) {
    CPlayer& player = mgr.GetPlayer();
    if (HasPointCrossedDock(mgr, player.GetTranslation())) {
      IGameArea::Dock* dock = mgr.GetWorld()->GetArea(mgr.GetNextAreaId())->GetDock(x25c_dock);
      TAreaId aid = dock->GetConnectedAreaId(dock->GetReferenceCount());
      if (aid != kInvalidAreaId && mgr.GetWorld()->GetArea(aid)->GetActive()) {
        mgr.SetCurrentAreaId(aid);
        s32 otherDock = dock->GetOtherDockNumber(dock->GetReferenceCount());

        if (CObjectList* objs = mgr.GetWorld()->GetArea(aid)->GetAreaObjects()) {
          for (CEntity* ent : *objs) {
            const TCastToPtr<CScriptDock> dock2(ent);
            if (dock2 && dock2->GetDockId() == otherDock) {
              dock2->SetLoadConnected(mgr, true);
            }
          }
        }
      }
    }

    x264_dockState = EDockState::Idle;
  }
}

void CScriptDock::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Registered: {
    CGameArea* area = mgr.GetWorld()->GetArea(x260_area);
    if (area->GetDockCount() < x25c_dock)
      return;
    IGameArea::Dock* dock = area->GetDock(x25c_dock);
    if (!dock->IsReferenced())
      dock->SetReferenceCount(x258_dockReferenceCount);
  } break;
  case EScriptObjectMessage::Deleted:
    CleanUp();
    break;
  case EScriptObjectMessage::InitializedInArea:
    AreaLoaded(mgr);
    break;
  case EScriptObjectMessage::WorldInitialized: {
    UpdateAreaActivateFlags(mgr);
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Add(EMaterialTypes::AIBlock);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  } break;
  case EScriptObjectMessage::SetToZero: {
    if (mgr.GetNextAreaId() != x260_area)
      return;

    SetLoadConnected(mgr, false);

    CGameArea* area = mgr.GetWorld()->GetArea(x260_area);
    IGameArea::Dock* dock = area->GetDock(x25c_dock);

    TAreaId aid = dock->GetConnectedAreaId(dock->GetReferenceCount());

    CPlatformAndDoorList& lst = mgr.GetPlatformAndDoorObjectList();
    for (CEntity* ent : lst) {
      TCastToPtr<CScriptDoor> door(ent);
      if (door && door->IsConnectedToArea(mgr, aid))
        door->ForceClosed(mgr);
    }
  } break;
  case EScriptObjectMessage::SetToMax:
    if (mgr.GetNextAreaId() != x260_area)
      return;

    SetLoadConnected(mgr, true);
    break;
  case EScriptObjectMessage::Increment:
    SetLoadConnected(mgr, true);
    [[fallthrough]];
  case EScriptObjectMessage::Decrement: {
    TAreaId aid = x260_area;
    if (mgr.GetNextAreaId() == x260_area) {
      IGameArea::Dock* dock = mgr.GetWorld()->GetArea(x260_area)->GetDock(x25c_dock);
      aid = dock->GetConnectedAreaId(dock->GetReferenceCount());
    }

    if (aid >= 0 && aid < mgr.GetWorld()->GetNumAreas() && mgr.GetWorld()->GetArea(aid)->GetActive())
      CWorld::PropogateAreaChain(CGameArea::EOcclusionState(msg == EScriptObjectMessage::Increment),
                                 mgr.GetWorld()->GetArea(aid), mgr.GetWorld());
  } break;
  default:
    CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
    break;
  }
}

std::optional<zeus::CAABox> CScriptDock::GetTouchBounds() const {
  if (x264_dockState == EDockState::Three) {
    return std::nullopt;
  }

  return GetBoundingBox();
}

void CScriptDock::Touch(CActor& act, CStateManager&) {
  if (x264_dockState == EDockState::Three)
    return;

  if (TCastToPtr<CPlayer>(act))
    x264_dockState = EDockState::PlayerTouched;
}

zeus::CPlane CScriptDock::GetPlane(const CStateManager& mgr) const {
  const IGameArea::Dock* dock = mgr.GetWorld()->GetAreaAlways(x260_area)->GetDock(x25c_dock);
  return zeus::CPlane(dock->GetPoint(0), dock->GetPoint(1), dock->GetPoint(2));
}

void CScriptDock::SetDockReference(CStateManager& mgr, s32 ref) {
  mgr.GetWorld()->GetArea(x260_area)->GetDock(x25c_dock)->SetReferenceCount(ref);
  x268_24_dockReferenced = true;
}

s32 CScriptDock::GetDockReference(const CStateManager& mgr) const {
  return mgr.GetWorld()->GetAreaAlways(x260_area)->GetDock(x25c_dock)->GetReferenceCount();
}

TAreaId CScriptDock::GetCurrentConnectedAreaId(const CStateManager& mgr) const {
  if (x260_area >= mgr.GetWorld()->GetNumAreas())
    return kInvalidAreaId;
  const CGameArea* area = mgr.GetWorld()->GetAreaAlways(x260_area);
  if (x25c_dock >= area->GetDockCount())
    return kInvalidAreaId;

  const IGameArea::Dock* dock = area->GetDock(x25c_dock);
  return dock->GetConnectedAreaId(dock->GetReferenceCount());
}

void CScriptDock::UpdateAreaActivateFlags(CStateManager& mgr) {
  CWorld* world = mgr.GetWorld();
  if (x260_area >= world->GetNumAreas())
    return;

  CGameArea* area = world->GetArea(x260_area);

  if (x25c_dock >= area->GetDockCount())
    return;

  IGameArea::Dock* dock = area->GetDock(x25c_dock);

  for (s32 i = 0; i < dock->GetDockRefs().size(); ++i) {
    s32 dockRefCount = dock->GetReferenceCount();
    TAreaId aid = dock->GetConnectedAreaId(i);
    if (aid != kInvalidAreaId)
      world->GetArea(aid)->SetActive(i == dockRefCount);
  }
  mgr.SetCurrentAreaId(mgr.GetNextAreaId());
}

bool CScriptDock::HasPointCrossedDock(const CStateManager& mgr, const zeus::CVector3f& point) const {
  const zeus::CPlane plane = GetPlane(mgr);
  return plane.pointToPlaneDist(point) >= 0.f;
}

void CScriptDock::AreaLoaded(CStateManager& mgr) { SetLoadConnected(mgr, x268_25_loadConnected); }

void CScriptDock::SetLoadConnected(CStateManager& mgr, bool loadOther) {
  IGameArea::Dock* dock = mgr.GetWorld()->GetArea(x260_area)->GetDock(x25c_dock);
  bool cur = dock->GetShouldLoadOther(dock->GetReferenceCount());
  if (cur == loadOther)
    return;

  dock->SetShouldLoadOther(dock->GetReferenceCount(), loadOther);
}
} // namespace urde
