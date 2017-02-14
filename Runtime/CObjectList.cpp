#include "CObjectList.hpp"

namespace urde
{

CObjectList::CObjectList(EGameObjectList listEnum)
: x2004_listEnum(listEnum)
{}

void CObjectList::AddObject(CEntity& entity)
{
    if (IsQualified(entity))
    {
        if (x2008_firstId != -1)
            x0_list[x2008_firstId].prev = entity.GetUniqueId() & 0x3ff;
        TUniqueId prevFirst = x2008_firstId;
        x2008_firstId = entity.GetUniqueId() & 0x3ff;
        SObjectListEntry& newEnt = x0_list[x2008_firstId];
        newEnt.entity = &entity;
        newEnt.next = prevFirst;
        newEnt.prev = -1;
        ++x200a_count;
    }
}

void CObjectList::RemoveObject(TUniqueId uid)
{
    uid = uid & 0x3ff;
    SObjectListEntry& ent = x0_list[uid];
    if (!ent.entity || ent.entity->GetUniqueId() != uid)
        return;
    if (uid == x2008_firstId)
    {
        x2008_firstId = ent.next;
        if (ent.next != -1)
            x0_list[ent.next].prev = -1;
    }
    else
    {
        if (ent.next != -1)
            x0_list[ent.next].prev = -1;
        x0_list[ent.prev].next = -1;
    }
    ent.entity = nullptr;
    ent.next = -1;
    ent.prev = -1;
    --x200a_count;
}

const CEntity* CObjectList::operator[](size_t i) const
{
    const SObjectListEntry& ent = x0_list[i];
    if (ent.entity->x30_26_scriptingBlocked)
        return nullptr;
    return ent.entity;
}

CEntity* CObjectList::operator[](size_t i)
{
    SObjectListEntry& ent = x0_list[i];
    if (ent.entity->x30_26_scriptingBlocked)
        return nullptr;
    return ent.entity;
}

const CEntity* CObjectList::GetObjectById(TUniqueId uid) const
{
    if (!uid)
        return nullptr;
    const SObjectListEntry& ent = x0_list[uid & 0x3ff];
    if (ent.entity->x30_26_scriptingBlocked)
        return nullptr;
    return ent.entity;
}

CEntity* CObjectList::GetObjectById(TUniqueId uid)
{
    if (!uid)
        return nullptr;
    SObjectListEntry& ent = x0_list[uid & 0x3ff];
    if (ent.entity->x30_26_scriptingBlocked)
        return nullptr;
    return ent.entity;
}

bool CObjectList::IsQualified(const CEntity&) {return true;}

}
