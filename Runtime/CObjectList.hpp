#ifndef __URDE_COBJECTLIST_HPP__
#define __URDE_COBJECTLIST_HPP__

#include "World/CEntity.hpp"
#include "RetroTypes.hpp"

namespace urde
{

enum class EGameObjectList
{
    Invalid = -1,
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
    friend class CGameArea;

    struct SObjectListEntry
    {
        CEntity* entity = nullptr;
        TUniqueId next = -1;
        TUniqueId prev = -1;
    };
    SObjectListEntry m_list[1024];
    EGameObjectList m_listEnum;
    TUniqueId m_firstId = kInvalidUniqueId;
    u16 m_count = 0;
    int m_areaIdx = 0;
public:
    class iterator
    {
        friend class CObjectList;
        CObjectList& m_list;
        TUniqueId m_id;
        iterator(CObjectList& list, TUniqueId id) : m_list(list), m_id(id) {}
    public:
        iterator& operator++() { m_id = m_list.GetNextObjectIndex(m_id); return *this; }
        bool operator!=(const iterator& other) const { return m_id != other.m_id; }
        CEntity* operator*() const { return m_list.GetObjectById(m_id); }
    };
    iterator begin() { return iterator(*this, m_firstId); }
    iterator end() { return iterator(*this, kInvalidUniqueId); }

    CObjectList(EGameObjectList listEnum);

    void AddObject(CEntity& entity);
    void RemoveObject(TUniqueId uid);
    const CEntity* GetObjectById(TUniqueId uid) const;
    CEntity* GetObjectById(TUniqueId uid);
    TUniqueId GetFirstObjectIndex() const { return m_firstId; }
    TUniqueId GetNextObjectIndex(TUniqueId prev) const { return m_list[prev].next; }
    virtual bool IsQualified();
};

}

#endif // __URDE_COBJECTLIST_HPP__
