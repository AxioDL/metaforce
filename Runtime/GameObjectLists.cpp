#include "GameObjectLists.hpp"

namespace urde
{

CActorList::CActorList()
: CObjectList(EGameObjectList::Actor) {}

CPhysicsActorList::CPhysicsActorList()
: CObjectList(EGameObjectList::PhysicsActor) {}

CGameCameraList::CGameCameraList()
: CObjectList(EGameObjectList::GameCamera) {}

CListeningAiList::CListeningAiList()
: CObjectList(EGameObjectList::ListeningAi) {}

CAiWaypointList::CAiWaypointList()
: CObjectList(EGameObjectList::AiWaypoint) {}

CPlatformAndDoorList::CPlatformAndDoorList()
: CObjectList(EGameObjectList::PlatformAndDoor) {}

CGameLightList::CGameLightList()
: CObjectList(EGameObjectList::GameLight) {}

}
