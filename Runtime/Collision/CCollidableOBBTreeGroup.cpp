#include "CCollidableOBBTreeGroup.hpp"
#include "CToken.hpp"

namespace urde
{

CCollidableOBBTreeGroup::CCollidableOBBTreeGroup(CInputStream& in)
{
}

CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag &tag, CInputStream &in,
                                                const CVParamTransfer &vparms)
{
    return TToken<CCollidableOBBTreeGroup>::GetIObjObjectFor(std::make_unique<CCollidableOBBTreeGroup>(in));
}

}
