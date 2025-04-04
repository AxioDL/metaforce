#include "Runtime/CObjectList.hpp"
#ifndef NDEBUG
#include "Runtime/Logging.hpp"
#endif

namespace metaforce {

CObjectList::CObjectList(EGameObjectList listEnum) : x2004_listEnum(listEnum) {}

void CObjectList::AddObject(CEntity& entity) {
  if (IsQualified(entity)) {
#ifndef NDEBUG
    if (x0_list[entity.GetUniqueId().Value()].entity != nullptr &&
        x0_list[entity.GetUniqueId().Value()].entity != &entity)
      spdlog::fatal("INVALID USAGE DETECTED: Attempting to assign entity '{} ({})' to existing node '{}'!!!",
                    entity.GetName(), entity.GetEditorId(), entity.GetUniqueId().Value());
#endif
    s16 prevFirst = -1;
    if (x2008_firstId != -1) {
      x0_list[x2008_firstId].prev = entity.GetUniqueId().Value();
      prevFirst = x2008_firstId;
    }
    x2008_firstId = entity.GetUniqueId().Value();
    SObjectListEntry& newEnt = x0_list[x2008_firstId];
    newEnt.entity = &entity;
    newEnt.next = prevFirst;
    newEnt.prev = -1;
    ++x200a_count;
  }
}

void CObjectList::RemoveObject(TUniqueId uid) {
  SObjectListEntry& ent = x0_list[uid.Value()];
  if (!ent.entity || ent.entity->GetUniqueId() != uid)
    return;
  if (uid.Value() == x2008_firstId) {
    x2008_firstId = ent.next;
    if (ent.next != -1)
      x0_list[ent.next].prev = -1;
  } else {
    x0_list[ent.prev].next = ent.next;
    if (ent.next != -1)
      x0_list[ent.next].prev = ent.prev;
  }
  ent.entity = nullptr;
  ent.next = -1;
  ent.prev = -1;
  --x200a_count;
}

const CEntity* CObjectList::operator[](size_t i) const {
  const SObjectListEntry& ent = x0_list[i];
  if (!ent.entity || ent.entity->x30_26_scriptingBlocked)
    return nullptr;
  return ent.entity;
}

CEntity* CObjectList::operator[](size_t i) {
  SObjectListEntry& ent = x0_list[i];
  if (!ent.entity || ent.entity->x30_26_scriptingBlocked)
    return nullptr;
  return ent.entity;
}

const CEntity* CObjectList::GetObjectById(TUniqueId uid) const {
  if (uid == kInvalidUniqueId)
    return nullptr;
  const SObjectListEntry& ent = x0_list[uid.Value()];
  if (!ent.entity || ent.entity->x30_26_scriptingBlocked)
    return nullptr;
  return ent.entity;
}

CEntity* CObjectList::GetObjectById(TUniqueId uid) {
  if (uid == kInvalidUniqueId)
    return nullptr;
  SObjectListEntry& ent = x0_list[uid.Value()];
  if (!ent.entity || ent.entity->x30_26_scriptingBlocked)
    return nullptr;
  return ent.entity;
}

const CEntity* CObjectList::GetValidObjectById(TUniqueId uid) const {
  if (uid == kInvalidUniqueId)
    return nullptr;
  const SObjectListEntry& ent = x0_list[uid.Value()];
  if (!ent.entity)
    return nullptr;
  if (ent.entity->GetUniqueId() != uid)
    return nullptr;
  return ent.entity;
}

CEntity* CObjectList::GetValidObjectById(TUniqueId uid) {
  if (uid == kInvalidUniqueId)
    return nullptr;
  SObjectListEntry& ent = x0_list[uid.Value()];
  if (!ent.entity)
    return nullptr;
  if (ent.entity->GetUniqueId() != uid)
    return nullptr;
  return ent.entity;
}

bool CObjectList::IsQualified(const CEntity&) const { return true; }

} // namespace metaforce
