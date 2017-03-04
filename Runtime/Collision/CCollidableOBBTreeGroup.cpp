#include "CCollidableOBBTreeGroup.hpp"
#include "CCollidableOBBTree.hpp"
#include "CToken.hpp"

namespace urde
{
const CCollisionPrimitive::Type CCollidableOBBTreeGroup::sType(CCollidableOBBTreeGroup::SetStaticTableIndex, "CCollidableOBBTreeGroup");
u32 CCollidableOBBTreeGroup::sTableIndex = -1;

CCollidableOBBTreeGroupContainer::CCollidableOBBTreeGroupContainer(CInputStream& in)
{
    u32 treeCount = in.readUint32Big();
    x0_trees.reserve(treeCount);

    for (u32 i = 0 ; i < treeCount ; i++)
    {
        std::unique_ptr<COBBTree> tree(new COBBTree(in));
        x0_trees.push_back(std::move(tree));
    }

    x10_aabbs.reserve(x0_trees.size());

    for (const std::unique_ptr<COBBTree>& tree : x0_trees)
        x10_aabbs.push_back(CCollidableOBBTree(tree.get(), CMaterialList()).CalculateLocalAABox());
}

CCollidableOBBTreeGroupContainer::CCollidableOBBTreeGroupContainer(const zeus::CVector3f &, const zeus::CVector3f &)
{
}

CCollidableOBBTreeGroup::CCollidableOBBTreeGroup(const CCollidableOBBTreeGroupContainer* container,
                                                 const CMaterialList& matList)
: CCollisionPrimitive(matList)
, x10_container(container)
{
}

void CCollidableOBBTreeGroup::ResetTestStats() const
{

}

u32 CCollidableOBBTreeGroup::GetTableIndex() const
{
    return sTableIndex;
}

zeus::CAABox CCollidableOBBTreeGroup::CalculateAABox(const zeus::CTransform& xf) const
{
    return x10_container->x20_aabox.getTransformedAABox(xf);
}

zeus::CAABox CCollidableOBBTreeGroup::CalculateLocalAABox() const
{
    return x10_container->x20_aabox;
}

FourCC CCollidableOBBTreeGroup::GetPrimType() const
{
    return SBIG('OBTG');
}

CRayCastResult CCollidableOBBTreeGroup::CastRayInternal(const CInternalRayCastStructure&) const
{
    return {};
}

const CCollisionPrimitive::Type& CCollidableOBBTreeGroup::GetType()
{
    return sType;
}

void CCollidableOBBTreeGroup::SetStaticTableIndex(u32 index)
{
    sTableIndex = index;
}

bool CCollidableOBBTreeGroup::SphereCollide(const CInternalCollisionStructure &, CCollisionInfoList &)
{
    return false;
}

bool CCollidableOBBTreeGroup::SphereCollideBoolean(const CInternalCollisionStructure &)
{
    return false;
}

bool CCollidableOBBTreeGroup::CollideMovingSphere(const CInternalCollisionStructure &, const zeus::CVector3f &, double &, CCollisionInfo &)
{
    return false;
}

bool CCollidableOBBTreeGroup::AABoxCollide(const CInternalCollisionStructure &, CCollisionInfoList &)
{
    return false;
}

bool CCollidableOBBTreeGroup::AABoxCollideBoolean(const CInternalCollisionStructure &)
{
    return false;
}

bool CCollidableOBBTreeGroup::CollideMovingAABox(const CInternalCollisionStructure &, const zeus::CVector3f &, double &, CCollisionInfo &)
{
    return false;
}

CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag &tag, CInputStream& in,
                                                const CVParamTransfer& vparms,
                                                CObjectReference* selfRef)
{
    return TToken<CCollidableOBBTreeGroupContainer>::GetIObjObjectFor(std::make_unique<CCollidableOBBTreeGroupContainer>(in));
}

}
