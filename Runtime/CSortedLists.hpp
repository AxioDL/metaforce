#pragma once

#include <array>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Collision/CMaterialFilter.hpp"

#include <zeus/CAABox.hpp>

namespace urde {
enum class ESortedList { MinX, MinY, MinZ, MaxX, MaxY, MaxZ };

struct SSortedList {
  std::array<s16, 1024> x0_ids;
  u32 x800_size = 0;
  void Reset() { x0_ids.fill(-1); }
  SSortedList() { Reset(); }
};

class CActor;
class CSortedListManager {
  struct SNode {
    const CActor* x0_actor = nullptr;
    zeus::CAABox x4_box = zeus::skNullBox;
    std::array<s16, 6> x1c_selfIdxs{-1, -1, -1, -1, -1, -1};
    s16 x28_next = -1;
    bool x2a_populated = false;
    SNode() = default;
    SNode(const CActor* act, const zeus::CAABox& aabb) : x0_actor(act), x4_box(aabb), x2a_populated(true) {}
  };
  std::array<SNode, 1024> x0_nodes;
  std::array<SSortedList, 6> xb000_sortedLists;
  void Reset();
  void AddToLinkedList(s16 a, s16& b, s16& c) const;
  void RemoveFromList(ESortedList, s16);
  void MoveInList(ESortedList, s16);
  void InsertInList(ESortedList, SNode& node);
  s16 FindInListUpper(ESortedList, float) const;
  s16 FindInListLower(ESortedList, float) const;
  s16 ConstructIntersectionArray(const zeus::CAABox&);
  s16 CalculateIntersections(ESortedList, ESortedList, s16, s16, s16, s16, ESortedList, ESortedList, ESortedList,
                             ESortedList, const zeus::CAABox&);

public:
  CSortedListManager();
  void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>&, const zeus::CVector3f&, const zeus::CVector3f&, float,
                     const CMaterialFilter&, const CActor*) const;
  void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>&, const CActor&, const zeus::CAABox&) const;
  void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>&, const zeus::CAABox&, const CMaterialFilter&,
                     const CActor*) const;
  void Remove(const CActor*);
  void Move(const CActor* act, const zeus::CAABox& aabb);
  void Insert(const CActor* act, const zeus::CAABox& aabb);
  bool ActorInLists(const CActor* act) const;
};

} // namespace urde
