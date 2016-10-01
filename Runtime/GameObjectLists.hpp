#ifndef __URDE_GAMEOBJECTLISTS_HPP__
#define __URDE_GAMEOBJECTLISTS_HPP__

#include "CObjectList.hpp"

namespace urde
{

class CActorList : public CObjectList
{
public:
    CActorList();

    bool IsQualified(const CEntity&);
};

class CPhysicsActorList : public CObjectList
{
public:
    CPhysicsActorList();
    bool IsQualified(const CEntity&);
};

class CGameCameraList : public CObjectList
{
public:
    CGameCameraList();
    bool IsQualified(const CEntity&);
};

class CListeningAiList : public CObjectList
{
public:
    CListeningAiList();

    bool IsQualified(const CEntity&);
};

class CAiWaypointList : public CObjectList
{
public:
    CAiWaypointList();
    bool IsQualified(const CEntity&);
};

class CPlatformAndDoorList : public CObjectList
{
public:
    CPlatformAndDoorList();

    bool IsQualified(const CEntity&);
    bool IsDoor(const CEntity&);
    bool IsPlatform(const CEntity&);
};

class CGameLightList : public CObjectList
{
public:
    CGameLightList();

    bool IsQualified(const CEntity&);
};

}

#endif // __URDE_GAMEOBJECTLISTS_HPP__
