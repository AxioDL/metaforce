#ifndef __URDE_CCOLLIDABLEOBBTREE_HPP__
#define __URDE_CCOLLIDABLEOBBTREE_HPP__

#include "Collision/CCollisionPrimitive.hpp"
#include "COBBTree.hpp"

namespace urde
{
class CRayCastInfo
{
    const zeus::CMRay& x0_ray;
    const CMaterialFilter& x4_filter;
    float x8_mag;
public:
    CRayCastInfo(const zeus::CMRay& ray, const CMaterialFilter& filter, float mag)
    : x0_ray(ray), x4_filter(filter), x8_mag(mag) {}
};

class CCollidableOBBTree : public CCollisionPrimitive
{
    COBBTree* x10_tree = nullptr;
    u32 x14_ = 0;
    u32 x18_ = 0;
    u32 x1c_ = 0;
    static u32 sTableIndex;
    bool LineIntersectsLeaf(const COBBTree::CLeafData& leaf, CRayCastInfo& info) const;
public:
    CCollidableOBBTree(const COBBTree* tree, const CMaterialList& material);
    virtual ~CCollidableOBBTree() = default;
    void ResetTestStats() const;
    void ResetTestStatsRecurse(const COBBTree::CNode&) const;
    u32 GetTableIndex() const { return sTableIndex; }
    zeus::CAABox CalculateAABox(const zeus::CTransform &) const;
    zeus::CAABox CalculateLocalAABox() const;
    virtual FourCC GetPrimType() const;
    virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const;
};

}

#endif // __URDE_CCOLLIDABLEOBBTREE_HPP__
