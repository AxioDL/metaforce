#include "Runtime/GameObjectLists.hpp"

#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CScriptAiJumpPoint.hpp"
#include "Runtime/World/CScriptCoverPoint.hpp"
#include "Runtime/World/CScriptDoor.hpp"
#include "Runtime/World/CScriptPlatform.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CActorList::CActorList() : CObjectList(EGameObjectList::Actor) {}

bool CActorList::IsQualified(const CEntity& ent) const { return TCastToConstPtr<CActor>(ent).IsValid(); }

CPhysicsActorList::CPhysicsActorList() : CObjectList(EGameObjectList::PhysicsActor) {}

bool CPhysicsActorList::IsQualified(const CEntity& ent) const { return TCastToConstPtr<CPhysicsActor>(ent).IsValid(); }

CGameCameraList::CGameCameraList() : CObjectList(EGameObjectList::GameCamera) {}

bool CGameCameraList::IsQualified(const CEntity& ent) const { return TCastToConstPtr<CGameCamera>(ent).IsValid(); }

CListeningAiList::CListeningAiList() : CObjectList(EGameObjectList::ListeningAi) {}

bool CListeningAiList::IsQualified(const CEntity& ent) const {
  const TCastToConstPtr<CAi> ai(ent);
  return ai && ai->IsListening();
}

CAiWaypointList::CAiWaypointList() : CObjectList(EGameObjectList::AiWaypoint) {}

bool CAiWaypointList::IsQualified(const CEntity& ent) const {
  return TCastToConstPtr<CScriptCoverPoint>(ent) || TCastToConstPtr<CScriptAiJumpPoint>(ent);
}

CPlatformAndDoorList::CPlatformAndDoorList() : CObjectList(EGameObjectList::PlatformAndDoor) {}

bool CPlatformAndDoorList::IsQualified(const CEntity& ent) const { return IsDoor(ent) || IsPlatform(ent); }

bool CPlatformAndDoorList::IsDoor(const CEntity& ent) const { return TCastToConstPtr<CScriptDoor>(ent).IsValid(); }

bool CPlatformAndDoorList::IsPlatform(const CEntity& ent) const {
  return TCastToConstPtr<CScriptPlatform>(ent).IsValid();
}

CGameLightList::CGameLightList() : CObjectList(EGameObjectList::GameLight) {}

bool CGameLightList::IsQualified(const CEntity& lt) const { return TCastToConstPtr<CGameLight>(lt).IsValid(); }

} // namespace urde
