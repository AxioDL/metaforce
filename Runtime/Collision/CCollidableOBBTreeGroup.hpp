#ifndef __URDE_CCOLLIDABLEOBBTREEGROUP_HPP__
#define __URDE_CCOLLIDABLEOBBTREEGROUP_HPP__

#include "IOStreams.hpp"
#include "CFactoryMgr.hpp"
#include "COBBTree.hpp"
#include "zeus/CAABox.hpp"
#include "CCollisionPrimitive.hpp"

namespace urde
{
class CCollidableOBBTreeGroup : public CCollisionPrimitive
{
    static const Type sType;
    static u32 sTableIndex;
    std::vector<std::unique_ptr<COBBTree>> x0_trees;
    std::vector<zeus::CAABox> x10_aabbs;
public:
    CCollidableOBBTreeGroup(CInputStream& in);
    virtual ~CCollidableOBBTreeGroup() {}

    void ResetTestStats() const;
    virtual u32 GetTableIndex() const;
    virtual zeus::CAABox CalculateAABox(const zeus::CTransform&) const;
    virtual zeus::CAABox CalculateLocalAABox() const;
    virtual FourCC GetPrimType() const;
    virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const;

    static const Type& GetType();
    static void SetStaticTableIndex(u32 index);
};

CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag &tag, CInputStream &in,
                                                const CVParamTransfer &vparms);

}

#endif // __URDE_CCOLLIDABLEOBBTREEGROUP_HPP__
