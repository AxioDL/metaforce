#include "CSortedLists.hpp"
#include "World/CActor.hpp"

namespace urde
{

CSortedListManager::CSortedListManager()
{
    Reset();
}

void CSortedListManager::Reset()
{
    std::fill(std::begin(x0_nodes), std::end(x0_nodes), SNode());
    for (int i=0 ; i<6 ; ++i)
        xb000_sortedLists[i].Reset();
}

void CSortedListManager::AddToLinkedList(s16 nodeId, s16& headId, s16& tailId) const
{
    if (headId == -1)
    {
        const_cast<SNode&>(x0_nodes[nodeId]).x28_next = headId;
        headId = nodeId;
        tailId = nodeId;
    }
    else
    {
        if (x0_nodes[nodeId].x28_next != -1)
            return;
        if (tailId == nodeId)
            return;
        const_cast<SNode&>(x0_nodes[nodeId]).x28_next = headId;
        headId = nodeId;
    }
}

void CSortedListManager::RemoveFromList(ESortedList list, s16 idx)
{
    SSortedList& sl = xb000_sortedLists[u32(list)];
    while (idx < sl.x800_size - 1)
    {
        x0_nodes[sl.x0_ids[idx+1]].x1c_selfIdxs[int(list)] = idx;
        sl.x0_ids[idx] = sl.x0_ids[idx+1];
        ++idx;
    }
    --sl.x800_size;
}

void CSortedListManager::MoveInList(ESortedList list, s16 idx)
{
    SSortedList& sl = xb000_sortedLists[int(list)];
    while (true)
    {
        if (idx > 0 ||
            x0_nodes[sl.x0_ids[idx-1]].x4_box[idx] > x0_nodes[sl.x0_ids[idx]].x4_box[idx])
        {
            x0_nodes[sl.x0_ids[idx-1]].x1c_selfIdxs[int(list)] = idx;
            x0_nodes[sl.x0_ids[idx]].x1c_selfIdxs[int(list)] = idx - 1;
            std::swap(sl.x0_ids[idx], sl.x0_ids[idx-1]);
            --idx;
        }
        else
        {
            if (idx >= sl.x800_size - 1)
                return;
            if (x0_nodes[sl.x0_ids[idx+1]].x4_box[int(list)] >= x0_nodes[sl.x0_ids[idx]].x4_box[int(list)])
                return;
            x0_nodes[sl.x0_ids[idx+1]].x1c_selfIdxs[int(list)] = idx + 1;
            std::swap(sl.x0_ids[idx], sl.x0_ids[idx+1]);
        }
    }
}

void CSortedListManager::InsertInList(ESortedList list, SNode& node)
{
    SSortedList& sl = xb000_sortedLists[int(list)];
    int insIdx = 0;
    for (int i=sl.x800_size ; i>0 ;)
    {
        /* Binary search cycle to find insert index */
        if (x0_nodes[sl.x0_ids[insIdx+i/2]].x4_box[int(list)] < node.x4_box[int(list)])
        {
            /* Upper */
            insIdx += i / 2 + 1;
            i -= i / 2 - 1;
        }
        else
        {
            /* Lower */
            i /= 2;
        }
    }

    /* Shift ids for insert */
    for (int i=sl.x800_size ; i>insIdx ; --i)
    {
        x0_nodes[sl.x0_ids[i]].x1c_selfIdxs[int(list)] = i;
        sl.x0_ids[i] = sl.x0_ids[i-1];
    }

    /* Do insert */
    sl.x0_ids[insIdx] = node.x0_actor->GetUniqueId().Value();
    node.x1c_selfIdxs[int(list)] = insIdx;
    ++sl.x800_size;
}

s16 CSortedListManager::FindInListUpper(ESortedList list, float val) const
{
    const SSortedList& sl = xb000_sortedLists[int(list)];
    int idx = 0;
    for (int i=sl.x800_size ; i>0 ;)
    {
        /* Binary search cycle to find index */
        if (!(val < x0_nodes[sl.x0_ids[idx+i/2]].x4_box[int(list)]))
        {
            /* Upper */
            idx += i / 2 + 1;
            i -= i / 2 - 1;
        }
        else
        {
            /* Lower */
            i /= 2;
        }
    }
    return idx;
}

s16 CSortedListManager::FindInListLower(ESortedList list, float val) const
{
    const SSortedList& sl = xb000_sortedLists[int(list)];
    int idx = 0;
    for (int i=sl.x800_size ; i>0 ;)
    {
        /* Binary search cycle to find index */
        if (x0_nodes[sl.x0_ids[idx+i/2]].x4_box[int(list)] < val)
        {
            /* Upper */
            idx += i / 2 + 1;
            i -= i / 2 - 1;
        }
        else
        {
            /* Lower */
            i /= 2;
        }
    }
    return idx;
}

s16 CSortedListManager::ConstructIntersectionArray(const zeus::CAABox& aabb)
{
    int minXa = FindInListLower(ESortedList::MinX, aabb.min.x);
    int maxXa = FindInListUpper(ESortedList::MinX, aabb.max.x);
    int minXb = FindInListLower(ESortedList::MaxX, aabb.min.x);
    int maxXb = FindInListUpper(ESortedList::MaxX, aabb.max.x);
    int xEnd = std::min(int(xb000_sortedLists[3].x800_size) - maxXb, minXa) + (maxXb + (maxXa - minXa) - minXb) / 2;

    int minYa = FindInListLower(ESortedList::MinY, aabb.min.y);
    int maxYa = FindInListUpper(ESortedList::MinY, aabb.max.y);
    int minYb = FindInListLower(ESortedList::MaxY, aabb.min.y);
    int maxYb = FindInListUpper(ESortedList::MaxY, aabb.max.y);
    int yEnd = std::min(int(xb000_sortedLists[4].x800_size) - maxYb, minYa) + (maxYb + (maxYa - minYa) - minYb) / 2;

    int minZa = FindInListLower(ESortedList::MinZ, aabb.min.z);
    int maxZa = FindInListUpper(ESortedList::MinZ, aabb.max.z);
    int minZb = FindInListLower(ESortedList::MaxZ, aabb.min.z);
    int maxZb = FindInListUpper(ESortedList::MaxZ, aabb.max.z);
    int zEnd = std::min(int(xb000_sortedLists[5].x800_size) - maxZb, minZa) + (maxZb + (maxZa - minZa) - minZb) / 2;

    if (xEnd < yEnd && xEnd < zEnd)
    {
        return CalculateIntersections(ESortedList::MinX, ESortedList::MaxX, minXa, maxXa, minXb, maxXb,
                                      ESortedList::MinY, ESortedList::MaxY, ESortedList::MinZ, ESortedList::MaxZ, aabb);
    }
    else if (yEnd < zEnd)
    {
        return CalculateIntersections(ESortedList::MinY, ESortedList::MaxY, minYa, maxYa, minYb, maxYb,
                                      ESortedList::MinX, ESortedList::MaxX, ESortedList::MinZ, ESortedList::MaxZ, aabb);
    }
    else
    {
        return CalculateIntersections(ESortedList::MinZ, ESortedList::MaxZ, minZa, maxZa, minZb, maxZb,
                                      ESortedList::MinX, ESortedList::MaxX, ESortedList::MinY, ESortedList::MaxY, aabb);
    }
}

s16 CSortedListManager::CalculateIntersections(ESortedList la, ESortedList lb, s16 a, s16 b, s16 c, s16 d,
                                                     ESortedList slA, ESortedList slB, ESortedList slC, ESortedList slD,
                                                     const zeus::CAABox& aabb)
{
    s16 headId = -1;
    s16 tailId = -1;
    for (int i=a ; i<b ; ++i)
        AddToLinkedList(xb000_sortedLists[int(la)].x0_ids[i], headId, tailId);
    for (int i=c ; i<d ; ++i)
        AddToLinkedList(xb000_sortedLists[int(lb)].x0_ids[i], headId, tailId);

    if (a < xb000_sortedLists[int(lb)].x800_size - d)
    {
        for (int i=0 ; i<a ; ++i)
        {
            s16 id = xb000_sortedLists[int(la)].x0_ids[i];
            if (x0_nodes[id].x1c_selfIdxs[lb] > aabb[int(lb)])
                AddToLinkedList(id, headId, tailId);
        }
    }
    else
    {
        for (int i=d ; i<xb000_sortedLists[int(lb)].x800_size ; ++i)
        {
            s16 id = xb000_sortedLists[int(lb)].x0_ids[i];
            if (x0_nodes[id].x1c_selfIdxs[la] < aabb[int(la)])
                AddToLinkedList(id, headId, tailId);
        }
    }

    for (s16* id = &headId ; *id != -1 ;)
    {
        SNode& node = x0_nodes[*id];
        if (node.x4_box[int(slA)] > aabb[int(slB)] ||
            node.x4_box[int(slB)] < aabb[int(slA)] ||
            node.x4_box[int(slC)] > aabb[int(slD)] ||
            node.x4_box[int(slD)] < aabb[int(slC)])
        {
            /* Not intersecting; remove from chain */
            *id = node.x28_next;
            node.x28_next = -1;
            continue;
        }
        id = &node.x28_next;
    }

    return headId;
}

void CSortedListManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& out, const zeus::CVector3f& v1,
                                       const zeus::CVector3f& v2, float f1, const CMaterialFilter& filter,
                                       const CActor* actor) const
{
    if (f1 == 0.f)
        f1 = 8000.f;
    zeus::CVector3f mul = v2 * f1;
    zeus::CVector3f sum = mul + v1;
    zeus::CVector3f maxs(std::max(v1.z, sum.z), std::max(v1.y, sum.y), std::max(v1.x, sum.x));
    zeus::CVector3f mins(std::min(sum.z, v1.z), std::min(sum.y, v1.y), std::min(sum.y, v1.y));
    BuildNearList(out, zeus::CAABox(mins, maxs), filter, actor);
}

void CSortedListManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& out, const CActor& actor,
                                       const zeus::CAABox& aabb) const
{
    const CMaterialFilter& filter = actor.GetMaterialFilter();
    s16 id = const_cast<CSortedListManager&>(*this).ConstructIntersectionArray(aabb);
    while (id != -1)
    {
        const SNode& node = x0_nodes[id];
        if (&actor != node.x0_actor && filter.Passes(node.x0_actor->GetMaterialList()) &&
            node.x0_actor->GetMaterialFilter().Passes(actor.GetMaterialList()))
            out.push_back(node.x0_actor->GetUniqueId());

        id = node.x28_next;
        const_cast<SNode&>(node).x28_next = -1;
    }
}

void CSortedListManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& out, const zeus::CAABox& aabb,
                                       const CMaterialFilter& filter, const CActor* actor) const
{
    s16 id = const_cast<CSortedListManager&>(*this).ConstructIntersectionArray(aabb);
    while (id != -1)
    {
        const SNode& node = x0_nodes[id];
        if (actor != node.x0_actor && filter.Passes(node.x0_actor->GetMaterialList()))
            out.push_back(node.x0_actor->GetUniqueId());

        id = node.x28_next;
        const_cast<SNode&>(node).x28_next = -1;
    }
}

void CSortedListManager::Remove(const CActor* act)
{
    SNode& node = x0_nodes[act->GetUniqueId().Value()];
    if (!node.x2a_populated)
        return;

    RemoveFromList(ESortedList::MinX, node.x1c_selfIdxs[0]);
    RemoveFromList(ESortedList::MaxX, node.x1c_selfIdxs[3]);
    RemoveFromList(ESortedList::MinY, node.x1c_selfIdxs[1]);
    RemoveFromList(ESortedList::MaxY, node.x1c_selfIdxs[4]);
    RemoveFromList(ESortedList::MinZ, node.x1c_selfIdxs[2]);
    RemoveFromList(ESortedList::MaxZ, node.x1c_selfIdxs[5]);
    node.x2a_populated = false;
}

void CSortedListManager::Move(const CActor* act, const zeus::CAABox& aabb)
{
    SNode& node = x0_nodes[act->GetUniqueId().Value()];
    node.x4_box = aabb;

    MoveInList(ESortedList::MinX, node.x1c_selfIdxs[0]);
    MoveInList(ESortedList::MaxX, node.x1c_selfIdxs[3]);
    MoveInList(ESortedList::MinY, node.x1c_selfIdxs[1]);
    MoveInList(ESortedList::MaxY, node.x1c_selfIdxs[4]);
    MoveInList(ESortedList::MinZ, node.x1c_selfIdxs[2]);
    MoveInList(ESortedList::MaxZ, node.x1c_selfIdxs[5]);
}

void CSortedListManager::Insert(const CActor* act, const zeus::CAABox& aabb)
{
    SNode& node = x0_nodes[act->GetUniqueId().Value()];
    if (node.x2a_populated)
    {
        Move(act, aabb);
        return;
    }

    SNode newNode(act, aabb);
    InsertInList(ESortedList::MinX, newNode);
    InsertInList(ESortedList::MaxX, newNode);
    InsertInList(ESortedList::MinY, newNode);
    InsertInList(ESortedList::MaxY, newNode);
    InsertInList(ESortedList::MinZ, newNode);
    InsertInList(ESortedList::MaxZ, newNode);
    node = newNode;
}

bool CSortedListManager::ActorInLists(const CActor* act) const
{
    if (!act)
        return false;
    const SNode& node = x0_nodes[act->GetUniqueId().Value()];
    return node.x2a_populated;
}

}
