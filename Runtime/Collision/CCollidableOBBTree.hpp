#ifndef __URDE_CCOLLIDABLEOBBTREE_HPP__
#define __URDE_CCOLLIDABLEOBBTREE_HPP__

#include "Collision/CCollisionPrimitive.hpp"
#include "COBBTree.hpp"

namespace urde
{
class CCollidableOBBTree : public CCollisionPrimitive
{
    COBBTree* x10_tree = nullptr;
    u32 x14_ = 0;
    u32 x18_ = 0;
    u32 x1c_ = 0;
public:
    CCollidableOBBTree(const COBBTree* tree, const CMaterialList& material);
    virtual ~CCollidableOBBTree() {}
    void ResetTestStats() const;
    void ResetTestStatsRecurse(const COBBTree::CNode&) const;
    u32 GetTableIndex() const { return -1; }
    zeus::CAABox CalculateAABox(const zeus::CTransform &) const;
    zeus::CAABox CalculateLocalAABox() const;
    virtual FourCC GetPrimType() const;
    virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const;
};

}

#endif // __URDE_CCOLLIDABLEOBBTREE_HPP__
