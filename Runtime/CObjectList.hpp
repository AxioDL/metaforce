#ifndef __URDE_COBJECTLIST_HPP__
#define __URDE_COBJECTLIST_HPP__

#include "CEntity.hpp"
#include "RetroTypes.hpp"

namespace urde
{

enum class EGameObjectList
{
    All,
    Actor,
    PhysicsActor,
    GameCamera,
    GameLight,
    ListeningAi,
    AiWaypoint,
    PlatformAndDoor,
};

class CObjectList
{
    struct SObjectListEntry
    {
        CEntity* entity = nullptr;
        TUniqueId prev = -1;
        TUniqueId next = -1;
    };
    SObjectListEntry m_list[1024];
    EGameObjectList m_listEnum;
    TUniqueId m_lastId = -1;
    u16 m_count = 0;
public:
    CObjectList(EGameObjectList listEnum);

    void AddObject(CEntity& entity);
    void RemoveObject(TUniqueId uid);
    const CEntity* GetObjectById(TUniqueId uid) const;
    CEntity* GetObjectById(TUniqueId uid);
    virtual bool IsQualified();
};

}

#endif // __URDE_COBJECTLIST_HPP__
