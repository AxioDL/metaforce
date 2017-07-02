#include "CCollidableOBBTree.hpp"

namespace urde
{

u32 CCollidableOBBTree::sTableIndex = 0;

CCollidableOBBTree::CCollidableOBBTree(const COBBTree* tree, const urde::CMaterialList& material)
    : CCollisionPrimitive(material),
      x10_tree((COBBTree*)tree) {}

bool CCollidableOBBTree::LineIntersectsLeaf(const COBBTree::CLeafData& leaf, CRayCastInfo& info) const
{
    return false;
}

FourCC CCollidableOBBTree::GetPrimType() const
{
    return SBIG('OBBT');
}

CRayCastResult CCollidableOBBTree::CastRayInternal(const CInternalRayCastStructure&) const
{
    return {};
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
