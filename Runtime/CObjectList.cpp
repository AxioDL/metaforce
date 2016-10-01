#include "CObjectList.hpp"

namespace urde
{

CObjectList::CObjectList(EGameObjectList listEnum)
: m_listEnum(listEnum)
{}

void CObjectList::AddObject(CEntity& entity)
{
    if (IsQualified(entity))
    {
        if (m_firstId != -1)
            m_list[m_firstId].prev = entity.GetUniqueId() & 0x3ff;
        TUniqueId prevFirst = m_firstId;
        m_firstId = entity.GetUniqueId() & 0x3ff;
        SObjectListEntry& newEnt = m_list[m_firstId];
        newEnt.entity = &entity;
        newEnt.next = prevFirst;
        newEnt.prev = -1;
        ++m_count;
    }
}

void CObjectList::RemoveObject(TUniqueId uid)
{
    uid = uid & 0x3ff;
    SObjectListEntry& ent = m_list[uid];
    if (!ent.entity || ent.entity->GetUniqueId() != uid)
        return;
    if (uid == m_firstId)
    {
        m_firstId = ent.next;
        if (ent.next != -1)
            m_list[ent.next].prev = -1;
    }
    else
    {
        if (ent.next != -1)
            m_list[ent.next].prev = -1;
        m_list[ent.prev].next = -1;
    }
    ent.entity = nullptr;
    ent.next = -1;
    ent.prev = -1;
    --m_count;
}

const CEntity* CObjectList::GetObjectById(TUniqueId uid) const
{
    if (!uid)
        return nullptr;
    const SObjectListEntry& ent = m_list[uid & 0x3ff];
    if (ent.entity->x30_26_scriptingBlocked)
        return nullptr;
    return ent.entity;
}

CEntity* CObjectList::GetObjectById(TUniqueId uid)
{
    if (!uid)
        return nullptr;
    SObjectListEntry& ent = m_list[uid & 0x3ff];
    if (ent.entity->x30_26_scriptingBlocked)
        return nullptr;
    return ent.entity;
}

bool CObjectList::IsQualified(const CEntity&) {return true;}

}
