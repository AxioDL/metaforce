#include "CCollidableOBBTree.hpp"

namespace urde
{

CCollidableOBBTree::CCollidableOBBTree(const COBBTree* tree, const urde::CMaterialList& material)
    : CCollisionPrimitive(material),
      x10_tree((COBBTree*)tree)
{

}

FourCC CCollidableOBBTree::GetPrimType() const
{
    return SBIG('OBBT');
}

CRayCastResult CCollidableOBBTree::CastRayInternal(const CInternalRayCastStructure&) const
{

}

zeus::CAABox CCollidableOBBTree::CalculateAABox(const zeus::CTransform&) const
{
    return x10_tree->CalculateLocalAABox();
}

zeus::CAABox CCollidableOBBTree::CalculateLocalAABox() const
{

}

}
