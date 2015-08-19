#ifndef __RETRO_COBJECTLIST_HPP__
#define __RETRO_COBJECTLIST_HPP__

#include "CEntity.hpp"
#include "RetroTypes.hpp"

namespace Retro
{

enum EGameObjectList
{
    ListAll,
    ListActor,
    ListPhysicsActor,
    ListGameCamera,
    ListGameLight,
    ListListeningAi,
    ListAiWaypoint,
    ListPlatformAndDoor,
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
    CObjectList(EGameObjectList listEnum)
    : m_listEnum(listEnum)
    {}

    void AddObject(CEntity& entity)
    {
        if (IsQualified())
        {
            if (m_lastId != -1)
                m_list[m_lastId].next = entity.m_uid & 0x3ff;
            TUniqueId prevLast = m_lastId;
            m_lastId = entity.m_uid & 0x3ff;
            SObjectListEntry& newEnt = m_list[m_lastId];
            newEnt.entity = &entity;
            newEnt.prev = prevLast;
            newEnt.next = -1;
            ++m_count;
        }
    }

    void RemoveObject(TUniqueId uid)
    {
        uid = uid & 0x3ff;
        SObjectListEntry& ent = m_list[uid];
        if (!ent.entity || ent.entity->m_uid != uid)
            return;
        if (uid == m_lastId)
        {
            m_lastId = ent.prev;
            if (ent.prev != -1)
                m_list[ent.prev].next = -1;
        }
        else
        {
            if (ent.prev != -1)
                m_list[ent.prev].next = -1;
            m_list[ent.next].prev = -1;
        }
        ent.entity = nullptr;
        ent.prev = -1;
        ent.next = -1;
        --m_count;
    }

    const CEntity* GetObjectById(TUniqueId uid) const
    {
        if (!uid)
            return nullptr;
        return m_list[uid & 0x3ff].entity;
    }

    CEntity* GetObjectById(TUniqueId uid)
    {
        if (!uid)
            return nullptr;
        return m_list[uid & 0x3ff].entity;
    }

    virtual bool IsQualified() {return true;}
};

}

#endif // __RETRO_COBJECTLIST_HPP__
