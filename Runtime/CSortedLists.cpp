#include "CSortedLists.hpp"

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

}
