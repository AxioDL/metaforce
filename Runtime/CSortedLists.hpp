#ifndef __URDE_CSORTEDLISTS_HPP__
#define __URDE_CSORTEDLISTS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"
#include "Collision/CMaterialFilter.hpp"

namespace urde
{
enum ESortedList
{
    MinX,
    MinY,
    MinZ,
    MaxX,
    MaxY,
    MaxZ
};

struct SSortedList
{
    TUniqueId x0_ids[1024];
    u32 x800_size;
    void Reset() {std::fill(std::begin(x0_ids), std::end(x0_ids), kInvalidUniqueId);}
    SSortedList() {Reset();}
};

class CActor;
class CSortedListManager
{
    struct SNode
    {
        const CActor* x0_actor = nullptr;
        zeus::CAABox x4_box = zeus::CAABox::skNullBox;
        u16 x1c_selfIdxs[6] = {-1, -1, -1, -1, -1, -1};
        TUniqueId x28_next = kInvalidUniqueId;
        bool x2a_populated = false;
        SNode() = default;
        SNode(const CActor* act, const zeus::CAABox& aabb)
        : x0_actor(act), x4_box(aabb), x2a_populated(true) {}
    };
    SNode x0_nodes[1024];
    SSortedList xb000_sortedLists[6];
    void Reset();
    void AddToLinkedList(TUniqueId a, TUniqueId& b, TUniqueId& c) const;
    void RemoveFromList(ESortedList, s16);
    void MoveInList(ESortedList, s16);
    void InsertInList(ESortedList, SNode& node);
    s16 FindInListUpper(ESortedList, float) const;
    s16 FindInListLower(ESortedList, float) const;
    TUniqueId ConstructIntersectionArray(const zeus::CAABox&);
    TUniqueId CalculateIntersections(ESortedList, ESortedList, s16, s16, s16, s16,
                                     ESortedList, ESortedList, ESortedList, ESortedList, const zeus::CAABox&);
public:
    CSortedListManager();
    void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>&, const zeus::CVector3f&, const zeus::CVector3f&,
                       float, const CMaterialFilter&, const CActor*) const;
    void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>&, const CActor&, const zeus::CAABox&) const;
    void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>&, const zeus::CAABox&,
                       const CMaterialFilter&, const CActor*) const;
    void Remove(const CActor*);
    void Move(const CActor* act, const zeus::CAABox& aabb);
    void Insert(const CActor* act, const zeus::CAABox& aabb);
    bool ActorInLists(const CActor* act) const;
};

}

#endif // __URDE_CSORTEDLISTS_HPP__
