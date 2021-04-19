#pragma once

#include <array>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Collision/CMaterialFilter.hpp"

#include <zeus/CAABox.hpp>

namespace metaforce {
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
  void AddToLinkedList(s16 nodeId, s16& headId, s16& tailId);
  void RemoveFromList(ESortedList list, s16 idx);
  void MoveInList(ESortedList list, s16 idx);
  void InsertInList(ESortedList list, SNode& node);
  s16 FindInListUpper(ESortedList list, float value) const;
  s16 FindInListLower(ESortedList list, float value) const;
  s16 ConstructIntersectionArray(const zeus::CAABox& aabb);
  s16 CalculateIntersections(ESortedList la, ESortedList lb, s16 a, s16 b, s16 c, s16 d, ESortedList slA,
                             ESortedList slB, ESortedList slC, ESortedList slD, const zeus::CAABox& aabb);

public:
  CSortedListManager();
  void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& out, const zeus::CVector3f& pos,
                     const zeus::CVector3f& dir, float mag, const CMaterialFilter& filter, const CActor* actor);
  void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& out, const CActor& actor, const zeus::CAABox& aabb);
  void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& out, const zeus::CAABox& aabb,
                     const CMaterialFilter& filter, const CActor* actor);
  void Remove(const CActor* actor);
  void Move(const CActor* actor, const zeus::CAABox& aabb);
  void Insert(const CActor* actor, const zeus::CAABox& aabb);
  bool ActorInLists(const CActor* actor) const;
};

} // namespace metaforce
