#ifndef __URDE_CCOLLIDALBESPHERE_HPP
#define __URDE_CCOLLIDALBESPHERE_HPP

#include "CCollisionPrimitive.hpp"

namespace urde
{
class CCollidableSphere : public CCollisionPrimitive
{
    static const Type sType;
    static u32 sTableIndex;
public:

    virtual u32 GetTableIndex() const;
    virtual zeus::CAABox CalculateAABox(const zeus::CTransform&) const;
    virtual zeus::CAABox CalculateLocalAABox() const;
    virtual FourCC GetPrimType() const;
    virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const;

    static const CCollisionPrimitive::Type& GetType();
    static void SetStaticTableIndex(u32 index);
};
}

#endif // __URDE_CCOLLIDALBESPHERE_HPP
