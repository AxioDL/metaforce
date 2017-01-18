#include "GameObjectLists.hpp"
#include "World/CGameLight.hpp"
#include "World/CScriptDoor.hpp"
#include "World/CScriptPlatform.hpp"
#include "World/CScriptCoverPoint.hpp"
#include "World/CScriptAiJumpPoint.hpp"
#include "World/CPatterned.hpp"
#include "Camera/CGameCamera.hpp"
#include "TCastTo.hpp"

namespace urde
{

CActorList::CActorList()
    : CObjectList(EGameObjectList::Actor) {}

bool CActorList::IsQualified(const CEntity& ent)
{
    return TCastToConstPtr<CActor>(ent);
}

CPhysicsActorList::CPhysicsActorList()
    : CObjectList(EGameObjectList::PhysicsActor) {}

bool CPhysicsActorList::IsQualified(const CEntity& ent)
{
    return TCastToConstPtr<CPhysicsActor>(ent);
}

CGameCameraList::CGameCameraList()
    : CObjectList(EGameObjectList::GameCamera) {}

bool CGameCameraList::IsQualified(const CEntity& ent)
{
    return TCastToConstPtr<CGameCamera>(ent);
}

CListeningAiList::CListeningAiList()
    : CObjectList(EGameObjectList::ListeningAi) {}

bool CListeningAiList::IsQualified(const CEntity& ent)
{
    TCastToConstPtr<CAi> ai(ent);
    return ai && ai->IsListening();
}

CAiWaypointList::CAiWaypointList()
    : CObjectList(EGameObjectList::AiWaypoint) {}

bool CAiWaypointList::IsQualified(const CEntity& ent)
{
    return TCastToConstPtr<CScriptCoverPoint>(ent) ||
           TCastToConstPtr<CScriptAiJumpPoint>(ent);
}

CPlatformAndDoorList::CPlatformAndDoorList()
    : CObjectList(EGameObjectList::PlatformAndDoor) {}

bool CPlatformAndDoorList::IsQualified(const CEntity& ent)
{
    return IsDoor(ent) || IsPlatform(ent);
}

bool CPlatformAndDoorList::IsDoor(const CEntity& ent)
{
    return TCastToConstPtr<CScriptDoor>(ent);
}

bool CPlatformAndDoorList::IsPlatform(const CEntity& ent)
{
    return TCastToConstPtr<CScriptPlatform>(ent);
}

CGameLightList::CGameLightList()
    : CObjectList(EGameObjectList::GameLight) {}

bool CGameLightList::IsQualified(const CEntity& lt)
{
    return TCastToConstPtr<CGameLight>(lt);
}

}
