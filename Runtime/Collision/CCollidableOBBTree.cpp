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

zeus::CAABox CCollidableOBBTree::CalculateAABox(const zeus::CTransform& xf) const
{
    return x10_tree->CalculateAABox(xf);
}

zeus::CAABox CCollidableOBBTree::CalculateLocalAABox() const
{
    return x10_tree->CalculateLocalAABox();
}

}
