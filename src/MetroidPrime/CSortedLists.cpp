#include "MetroidPrime/CSortedLists.hpp"

#include "MetroidPrime/CActor.hpp"
#include "rstl/algorithm.hpp"

namespace SL {
static inline float GetPointForSL(ESortedLists list, const CAABox& box) {
  // Sorted-list axes follow the six consecutive minimum/maximum box coordinates.
  return reinterpret_cast< const float* >(&box)[list];
}

SNode::SNode()
: mActor(nullptr), mBox(CAABox::Identity()), mNext(-1), mPopulated(false) {}

SNode::SNode(CActor* actor, const CAABox& box)
: mActor(actor), mBox(box), mNext(-1), mPopulated(true) {
  for (int i = 0; i < 6; ++i) {
    mSelfIdxs[i] = -1;
  }
}

CSortedListManager::CSortedListManager() { Reset(); }

void CSortedListManager::Reset() {
  SNode node;

  for (uint i = 0; i < kMaxObjects; ++i) {
    mNodes[i] = node;
  }

  const SSortedList sorted;
  for (int i = 0; i < 6; ++i) {
    mSortedLists[i] = sorted;
  }
}

bool CSortedListManager::ActorInLists(const CActor* actor) const {
  bool ret = false;
  if (actor != nullptr && mNodes[actor->GetUniqueId().Value()].mPopulated) {
    ret = true;
  }

  return ret;
}

short CSortedListManager::FindInListLower(ESortedLists list, f32 value) const {
  const SSortedList& sorted = mSortedLists[list];
  int count = sorted.mSize;
  int half;
  int first = 0;
  while (count > 0) {
    half = count / 2;
    const int middle = first + half;
    if (GetPointForSL(list, mNodes[sorted.mIds[middle]].mBox) < value) {
      first = middle + 1;
      count = count - half - 1;
    } else {
      count = half;
    }
  }
  return first;
}

short CSortedListManager::FindInListUpper(ESortedLists list, f32 value) const {
  const SSortedList& sorted = mSortedLists[list];
  int count = sorted.mSize;
  int half;
  int first = 0;
  while (count > 0) {
    half = count / 2;
    const int middle = first + half;
    if (value < GetPointForSL(list, mNodes[sorted.mIds[middle]].mBox)) {
      count = half;
    } else {
      first = middle + 1;
      count = count - half - 1;
    }
  }
  return first;
}

void CSortedListManager::InsertInList(ESortedLists list, SNode& node) {
  SSortedList& sorted = mSortedLists[list];
  const float value = GetPointForSL(list, node.mBox);
  int count = sorted.mSize;
  int half;
  int first = 0;
  while (count > 0) {
    half = count / 2;
    const int middle = first + half;
    if (GetPointForSL(list, mNodes[sorted.mIds[middle]].mBox) < value) {
      first = middle + 1;
      count = count - half - 1;
    } else {
      count = half;
    }
  }

  for (int i = sorted.mSize; i > first; --i) {
    mNodes[sorted.mIds[i - 1]].mSelfIdxs[list] = i;
    sorted.mIds[i] = sorted.mIds[i - 1];
  }
  sorted.mIds[first] = node.mActor->GetUniqueId().Value();
  ++sorted.mSize;
  node.mSelfIdxs[list] = first;
}

void CSortedListManager::RemoveFromList(ESortedLists list, short idx) {
  SSortedList& sorted = mSortedLists[list];
  for (int i = idx; i < static_cast< int >(sorted.mSize) - 1; ++i) {
    mNodes[sorted.mIds[i + 1]].mSelfIdxs[list] = i;
    sorted.mIds[i] = sorted.mIds[i + 1];
  }
  --sorted.mSize;
}

void CSortedListManager::MoveInList(ESortedLists list, const short index) {
  SSortedList& sorted = mSortedLists[list];
  short idx = index;
  while (true) {
    if (idx > 0 && GetPointForSL(list, mNodes[sorted.mIds[idx - 1]].mBox) >
                       GetPointForSL(list, mNodes[sorted.mIds[idx]].mBox)) {
      mNodes[sorted.mIds[idx - 1]].mSelfIdxs[list] = idx;
      mNodes[sorted.mIds[idx]].mSelfIdxs[list] = idx - 1;
      rstl::swap(sorted.mIds[idx - 1], sorted.mIds[idx]);
      --idx;
    } else {
      if (idx >= static_cast< int >(sorted.mSize) - 1) {
        return;
      }
      if (!(GetPointForSL(list, mNodes[sorted.mIds[idx + 1]].mBox) <
            GetPointForSL(list, mNodes[sorted.mIds[idx]].mBox))) {
        return;
      }
      mNodes[sorted.mIds[idx + 1]].mSelfIdxs[list] = idx;
      mNodes[sorted.mIds[idx]].mSelfIdxs[list] = idx + 1;
      rstl::swap(sorted.mIds[idx + 1], sorted.mIds[idx]);
      ++idx;
    }
  }
}

void CSortedListManager::Insert(CActor* actor, const CAABox& box) {
  if (mNodes[actor->GetUniqueId().Value()].mPopulated) {
    Move(actor, box);
    return;
  }
  SNode node(actor, box);

  InsertInList(kSL_MinX, node);
  InsertInList(kSL_MaxX, node);
  InsertInList(kSL_MinY, node);
  InsertInList(kSL_MaxY, node);
  InsertInList(kSL_MinZ, node);
  InsertInList(kSL_MaxZ, node);
  mNodes[actor->GetUniqueId().Value()] = node;
}

void CSortedListManager::Remove(const CActor* actor) {
  if (actor == nullptr) {
    return;
  }

  SNode& node = mNodes[actor->GetUniqueId().Value()];
  if (node.mPopulated) {
    RemoveFromList(kSL_MinX, node.mSelfIdxs[kSL_MinX]);
    RemoveFromList(kSL_MaxX, node.mSelfIdxs[kSL_MaxX]);
    RemoveFromList(kSL_MinY, node.mSelfIdxs[kSL_MinY]);
    RemoveFromList(kSL_MaxY, node.mSelfIdxs[kSL_MaxY]);
    RemoveFromList(kSL_MinZ, node.mSelfIdxs[kSL_MinZ]);
    RemoveFromList(kSL_MaxZ, node.mSelfIdxs[kSL_MaxZ]);
    node.mPopulated = false;
  }
}

void CSortedListManager::Move(const CActor* actor, const CAABox& box) {
  SNode& node = mNodes[actor->GetUniqueId().Value()];
  node.mBox = box;
  MoveInList(kSL_MinX, node.mSelfIdxs[kSL_MinX]);
  MoveInList(kSL_MaxX, node.mSelfIdxs[kSL_MaxX]);
  MoveInList(kSL_MinY, node.mSelfIdxs[kSL_MinY]);
  MoveInList(kSL_MaxY, node.mSelfIdxs[kSL_MaxY]);
  MoveInList(kSL_MinZ, node.mSelfIdxs[kSL_MinZ]);
  MoveInList(kSL_MaxZ, node.mSelfIdxs[kSL_MaxZ]);
}

void CSortedListManager::AddToLinkedList(const short nodeId, short& headId, short& tailId) const {
  if (headId == -1) {
    mNodes[nodeId].mNext = headId;
    tailId = nodeId;
    headId = nodeId;
    return;
  }
  if (mNodes[nodeId].mNext != -1) {
    return;
  }
  if (nodeId == tailId) {
    return;
  }
  mNodes[nodeId].mNext = headId;
  headId = nodeId;
}

short CSortedListManager::CalculateIntersections(const ESortedLists la, const ESortedLists lb,
                                                 const short a, const short b, const short c,
                                                 const short d, const ESortedLists slA,
                                                 const ESortedLists slB, const ESortedLists slC,
                                                 const ESortedLists slD, const CAABox& aabb) const {
  short headId = -1;
  short tailId = -1;
  for (short i = a; i < b; ++i) {
    AddToLinkedList(mSortedLists[la].mIds[i], headId, tailId);
  }
  for (short i = c; i < d; ++i) {
    AddToLinkedList(mSortedLists[lb].mIds[i], headId, tailId);
  }

  if (a < static_cast< int >(mSortedLists[lb].mSize) - d) {
    for (short i = 0; i < a; ++i) {
      const short id = mSortedLists[la].mIds[i];
      if (GetPointForSL(lb, mNodes[id].mBox) > GetPointForSL(lb, aabb)) {
        AddToLinkedList(id, headId, tailId);
      }
    }
  } else {
    for (short i = d; i < static_cast< int >(mSortedLists[lb].mSize); ++i) {
      const short id = mSortedLists[lb].mIds[i];
      if (GetPointForSL(la, mNodes[id].mBox) < GetPointForSL(la, aabb)) {
        AddToLinkedList(id, headId, tailId);
      }
    }
  }

  for (short* id = &headId; *id != -1;) {
    const SNode& node = mNodes[*id];
    if (GetPointForSL(slA, node.mBox) > GetPointForSL(slB, aabb) ||
        GetPointForSL(slB, node.mBox) < GetPointForSL(slA, aabb) ||
        GetPointForSL(slC, node.mBox) > GetPointForSL(slD, aabb) ||
        GetPointForSL(slD, node.mBox) < GetPointForSL(slC, aabb)) {
      *id = node.mNext;
      node.mNext = -1;
      continue;
    }
    id = &node.mNext;
  }
  return headId;
}

short CSortedListManager::ConstructIntersectionArray(const CAABox& aabb) const {
  const short minXa = FindInListLower(kSL_MinX, aabb.GetMinPoint().GetX());
  const short maxXa = FindInListUpper(kSL_MinX, aabb.GetMaxPoint().GetX());
  const short minXb = FindInListLower(kSL_MaxX, aabb.GetMinPoint().GetX());
  const short maxXb = FindInListUpper(kSL_MaxX, aabb.GetMaxPoint().GetX());
  const short xOutside =
      rstl::min_val< short >(minXa, mSortedLists[kSL_MaxX].mSize - maxXb);

  const short minYa = FindInListLower(kSL_MinY, aabb.GetMinPoint().GetY());
  const short maxYa = FindInListUpper(kSL_MinY, aabb.GetMaxPoint().GetY());
  const short minYb = FindInListLower(kSL_MaxY, aabb.GetMinPoint().GetY());
  const short maxYb = FindInListUpper(kSL_MaxY, aabb.GetMaxPoint().GetY());
  const short yOutside =
      rstl::min_val< short >(minYa, mSortedLists[kSL_MaxY].mSize - maxYb);

  const short minZa = FindInListLower(kSL_MinZ, aabb.GetMinPoint().GetZ());
  const short maxZa = FindInListUpper(kSL_MinZ, aabb.GetMaxPoint().GetZ());
  const short minZb = FindInListLower(kSL_MaxZ, aabb.GetMinPoint().GetZ());
  const short maxZb = FindInListUpper(kSL_MaxZ, aabb.GetMaxPoint().GetZ());
  const short zOutside =
      rstl::min_val< short >(minZa, mSortedLists[kSL_MaxZ].mSize - maxZb);

  const int xCount = xOutside + (maxXb + (maxXa - minXa) - minXb) / 2;
  const int yCount = yOutside + (maxYb + (maxYa - minYa) - minYb) / 2;
  const int zCount = zOutside + (maxZb + (maxZa - minZa) - minZb) / 2;

  if (xCount < yCount && xCount < zCount) {
    return CalculateIntersections(kSL_MinX, kSL_MaxX, minXa, maxXa, minXb, maxXb, kSL_MinY,
                                  kSL_MaxY, kSL_MinZ, kSL_MaxZ, aabb);
  } else if (yCount < zCount) {
    return CalculateIntersections(kSL_MinY, kSL_MaxY, minYa, maxYa, minYb, maxYb, kSL_MinX,
                                  kSL_MaxX, kSL_MinZ, kSL_MaxZ, aabb);
  } else {
    return CalculateIntersections(kSL_MinZ, kSL_MaxZ, minZa, maxZa, minZb, maxZb, kSL_MinX,
                                  kSL_MaxX, kSL_MinY, kSL_MaxY, aabb);
  }
}

void CSortedListManager::BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut,
                                       const CAABox& box, const CMaterialFilter& filter,
                                       const CActor* actor) const {
  short id = ConstructIntersectionArray(box);
  while (id != -1) {
    const SNode& node = mNodes[id];
    CActor* candidate = node.mActor;
    if (actor != candidate && filter.Passes(candidate->GetMaterialList())) {
      nearListOut.push_back(candidate->GetUniqueId());
    }
    id = node.mNext;
    node.mNext = -1;
  }
}

// TODO: check how material and filters are retrieve
static inline void GetActorMaterialData(const CActor& actor, const CMaterialFilter*& filter,
                                        const CMaterialList*& materials) {
  filter = &actor.GetMaterialFilter();
  materials = &actor.GetMaterialList();
}

void CSortedListManager::BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut,
                                       const CActor& actor, const CAABox& box) const {
  const CMaterialList* materials;
  const CMaterialFilter* filter;
  GetActorMaterialData(actor, filter, materials);

  for (short id = ConstructIntersectionArray(box); id != -1;) {
    const SNode& node = mNodes[id];
    const CActor* const candidate = node.mActor;
    if (&actor != candidate && filter->Passes(candidate->GetMaterialList()) &&
        candidate->GetMaterialFilter().Passes(*materials)) {
      nearListOut.push_back(candidate->GetUniqueId());
    }
    id = node.mNext;
    node.mNext = -1;
  }
}

void CSortedListManager::BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut,
                                       const CVector3f& pos, const CVector3f& dir, const f32 mag,
                                       const CMaterialFilter& filter, const CActor* actor) const {
  const float length = mag ? mag : 8000.f;
  const CVector3f ray = dir * length;
  const CVector3f end = pos + ray;
  const CAABox box(rstl::min_val(pos.GetX(), end.GetX()), rstl::min_val(pos.GetY(), end.GetY()),
                   rstl::min_val(pos.GetZ(), end.GetZ()), rstl::max_val(pos.GetX(), end.GetX()),
                   rstl::max_val(pos.GetY(), end.GetY()), rstl::max_val(pos.GetZ(), end.GetZ()));
  BuildNearList(nearListOut, box, filter, actor);
}

} // namespace SL
