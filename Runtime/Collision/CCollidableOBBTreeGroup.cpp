#include "CCollidableOBBTreeGroup.hpp"
#include "COBBTree.hpp"
#include "CToken.hpp"

namespace urde
{

CCollidableOBBTreeGroup::CCollidableOBBTreeGroup(CInputStream& in)
{
    u32 treeCount = in.readUint32Big();
    x0_trees.reserve(treeCount);

    for (u32 i = 0 ; i < treeCount ; i++)
        x0_trees.push_back(in);
}

CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag &tag, CInputStream &in,
                                                const CVParamTransfer &vparms)
{
    return TToken<CCollidableOBBTreeGroup>::GetIObjObjectFor(std::make_unique<CCollidableOBBTreeGroup>(in));
}

}
