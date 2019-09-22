#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CEntity.hpp"

namespace urde {

enum class EGameObjectList {
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

class CObjectList {
  friend class CGameArea;

  struct SObjectListEntry {
    CEntity* entity = nullptr;
    s16 next = -1;
    s16 prev = -1;
  };
  SObjectListEntry x0_list[1024]; // was an rstl::prereserved_vector
  EGameObjectList x2004_listEnum;
  s16 x2008_firstId = -1;
  u16 x200a_count = 0;

public:
  class iterator {
    friend class CObjectList;
    CObjectList& m_list;
    s16 m_id;
    iterator(CObjectList& list, s16 id) : m_list(list), m_id(id) {}

  public:
    iterator& operator++() {
      m_id = m_list.GetNextObjectIndex(m_id);
      return *this;
    }
    bool operator!=(const iterator& other) const { return m_id != other.m_id; }
    CEntity* operator*() const { return m_list.GetObjectByIndex(m_id); }
  };
  iterator begin() { return iterator(*this, x2008_firstId); }
  iterator end() { return iterator(*this, -1); }

  class const_iterator {
    friend class CObjectList;
    const CObjectList& m_list;
    s16 m_id;
    const_iterator(const CObjectList& list, s16 id) : m_list(list), m_id(id) {}

  public:
    const_iterator& operator++() {
      m_id = m_list.GetNextObjectIndex(m_id);
      return *this;
    }
    bool operator!=(const iterator& other) const { return m_id != other.m_id; }
    const CEntity* operator*() const { return m_list.GetObjectByIndex(m_id); }
  };
  const_iterator cbegin() const { return const_iterator(*this, x2008_firstId); }
  const_iterator cend() const { return const_iterator(*this, -1); }

  CObjectList(EGameObjectList listEnum);
  virtual ~CObjectList() = default;

  void AddObject(CEntity& entity);
  void RemoveObject(TUniqueId uid);
  const CEntity* operator[](size_t i) const;
  CEntity* operator[](size_t i);
  const CEntity* GetObjectById(TUniqueId uid) const;
  const CEntity* GetObjectByIndex(s16 index) const { return x0_list[index].entity; }
  CEntity* GetObjectByIndex(s16 index) { return x0_list[index].entity; }
  CEntity* GetObjectById(TUniqueId uid);
  const CEntity* GetValidObjectById(TUniqueId uid) const;
  CEntity* GetValidObjectById(TUniqueId uid);
  s16 GetFirstObjectIndex() const { return x2008_firstId; }
  s16 GetNextObjectIndex(s16 prev) const { return x0_list[prev].next; }
  virtual bool IsQualified(const CEntity&);
  u16 size() const { return x200a_count; }
};

} // namespace urde
