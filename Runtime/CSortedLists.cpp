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

void CSortedListManager::RemoveFromList(ESortedList list, s16 id)
{
    SSortedList& sl = xb000_sortedLists[u32(list)];
}

void CSortedListManager::Remove(const CActor* act)
{
    SNode& node = x0_nodes[act->GetUniqueId() & 0x3ff];
    if (node.x2a_full == false)
        return;

    RemoveFromList(ESortedList::Zero, node.x1c_);
    RemoveFromList(ESortedList::Three, node.x22_);
    RemoveFromList(ESortedList::One, node.x1e_);
    RemoveFromList(ESortedList::Four, node.x24_);
    RemoveFromList(ESortedList::Two, node.x20_);
    RemoveFromList(ESortedList::Five, node.x26_);
    node.x2a_full = false;
}

}
