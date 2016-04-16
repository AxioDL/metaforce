#ifndef __URDE_GAMEOBJECTLISTS_HPP__
#define __URDE_GAMEOBJECTLISTS_HPP__

#include "CObjectList.hpp"

namespace urde
{

class CActorList : public CObjectList
{
public:
    CActorList();
};

class CPhysicsActorList : public CObjectList
{
public:
    CPhysicsActorList();
};

class CGameCameraList : public CObjectList
{
public:
    CGameCameraList();
};

class CListeningAiList : public CObjectList
{
public:
    CListeningAiList();
};

class CAiWaypointList : public CObjectList
{
public:
    CAiWaypointList();
};

class CPlatformAndDoorList : public CObjectList
{
public:
    CPlatformAndDoorList();
};

class CGameLightList : public CObjectList
{
public:
    CGameLightList();
};

}

#endif // __URDE_GAMEOBJECTLISTS_HPP__
