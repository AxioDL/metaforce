#include "GameObjectLists.hpp"
#include "World/CGameLight.hpp"
#include "World/CScriptDoor.hpp"
#include "World/CScriptPlatform.hpp"
#include "World/CScriptCoverPoint.hpp"
#include "World/CScriptAiJumpPoint.hpp"
#include "World/CPatterned.hpp"
#include "Camera/CGameCamera.hpp"

namespace urde
{

CActorList::CActorList()
    : CObjectList(EGameObjectList::Actor) {}

bool CActorList::IsQualified(const CEntity& ent)
{
    return static_cast<const CActor*>(&ent) != nullptr;
}

CPhysicsActorList::CPhysicsActorList()
    : CObjectList(EGameObjectList::PhysicsActor) {}

bool CPhysicsActorList::IsQualified(const CEntity& ent)
{
    return static_cast<const CPhysicsActor*>(&ent) != nullptr;
}

CGameCameraList::CGameCameraList()
    : CObjectList(EGameObjectList::GameCamera) {}

bool CGameCameraList::IsQualified(const CEntity& ent)
{
    return static_cast<const CGameCamera*>(&ent) != nullptr;
}

CListeningAiList::CListeningAiList()
    : CObjectList(EGameObjectList::ListeningAi) {}

bool CListeningAiList::IsQualified(const CEntity& ent)
{
    return (static_cast<const CPatterned*>(&ent) != nullptr);
}

CAiWaypointList::CAiWaypointList()
    : CObjectList(EGameObjectList::AiWaypoint) {}

bool CAiWaypointList::IsQualified(const CEntity& ent)
{
    return static_cast<const CScriptCoverPoint*>(&ent) != nullptr ||
           static_cast<const CScriptAiJumpPoint*>(&ent) != nullptr;
}

CPlatformAndDoorList::CPlatformAndDoorList()
    : CObjectList(EGameObjectList::PlatformAndDoor) {}

bool CPlatformAndDoorList::IsQualified(const CEntity& ent)
{
    return IsDoor(ent) || IsPlatform(ent);
}

bool CPlatformAndDoorList::IsDoor(const CEntity& ent)
{
    return static_cast<const CScriptDoor*>(&ent) != nullptr;
}

bool CPlatformAndDoorList::IsPlatform(const CEntity& ent)
{
    return static_cast<const CScriptPlatform*>(&ent) != nullptr;
}

CGameLightList::CGameLightList()
    : CObjectList(EGameObjectList::GameLight) {}

bool CGameLightList::IsQualified(const CEntity& lt)
{
    return static_cast<const CGameLight*>(&lt) != nullptr;
}

}
