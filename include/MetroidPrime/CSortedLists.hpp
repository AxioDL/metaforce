#ifndef _CSORTEDLISTS
#define _CSORTEDLISTS

#include "types.h"

#include "MetroidPrime/CObjectList.hpp"

#include "Kyoto/Math/CAABox.hpp"

class CActor;
class CMaterialFilter;

namespace SL {
enum ESortedLists {
  kSL_MinX,
  kSL_MinY,
  kSL_MinZ,
  kSL_MaxX,
  kSL_MaxY,
  kSL_MaxZ,
};

struct SNode {
  CActor* mActor;
  CAABox mBox;
  short mSelfIdxs[6];
  mutable short mNext;
  bool mPopulated;

  SNode();
  SNode(CActor* actor, const CAABox& box);
};
CHECK_SIZEOF(SNode, 0x2c);

struct SSortedList {
  short mIds[kMaxObjects];
  uint mSize;

  SSortedList() : mSize(0) {
    for (int i = 0; i < kMaxObjects; ++i) {
      mIds[i] = -1;
    }
  }
};
CHECK_SIZEOF(SSortedList, 0x804);

class CSortedListManager {
public:
  CSortedListManager();
  void Reset();
  bool ActorInLists(const CActor* actor) const;
  short FindInListLower(ESortedLists list, f32 value) const;
  short FindInListUpper(ESortedLists list, f32 value) const;
  void InsertInList(ESortedLists list, SNode& node);
  void RemoveFromList(ESortedLists list, short idx);
  void MoveInList(ESortedLists list, short idx);
  void Insert(CActor* actor, const CAABox& box);
  void Remove(const CActor* actor);
  void Move(const CActor* actor, const CAABox& box);
  void AddToLinkedList(const short nodeId, short& headId, short& tailId) const;
  short CalculateIntersections(ESortedLists la, ESortedLists lb, short a, short b, short c, short d,
                               ESortedLists slA, ESortedLists slB, ESortedLists slC,
                               ESortedLists slD, const CAABox& aabb) const;
  short ConstructIntersectionArray(const CAABox& aabb) const;
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut, const CAABox& box,
                     const CMaterialFilter& filter, const CActor* actor) const;

  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut, const CActor& actor,
                     const CAABox& box) const;
  void BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearListOut, const CVector3f& pos,
                     const CVector3f& dir, f32 mag, const CMaterialFilter& filter,
                     const CActor* actor) const;

private:
  SNode mNodes[kMaxObjects];
  SSortedList mSortedLists[6];
};
CHECK_SIZEOF(CSortedListManager, 0xe018);
} // namespace SL

#endif // _CSORTEDLISTS
