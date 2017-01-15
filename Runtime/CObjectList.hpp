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
        TUniqueId next = kInvalidUniqueId;
        TUniqueId prev = kInvalidUniqueId;
    };
    SObjectListEntry x0_list[1024];
    EGameObjectList x2004_listEnum;
    TUniqueId x2008_firstId = kInvalidUniqueId;
    u16 x200a_count = 0;
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
    iterator begin() { return iterator(*this, x2008_firstId); }
    iterator end() { return iterator(*this, kInvalidUniqueId); }

    CObjectList(EGameObjectList listEnum);

    void AddObject(CEntity& entity);
    void RemoveObject(TUniqueId uid);
    const CEntity* GetObjectById(TUniqueId uid) const;
    const CEntity* GetObjectByIndex(s32 index) const { return x0_list[index].entity; }
    CEntity* GetObjectById(TUniqueId uid);
    TUniqueId GetFirstObjectIndex() const { return x2008_firstId; }
    TUniqueId GetNextObjectIndex(TUniqueId prev) const { return x0_list[prev & 0x3ff].next; }
    virtual bool IsQualified(const CEntity&);
};

}

#endif // __URDE_COBJECTLIST_HPP__
