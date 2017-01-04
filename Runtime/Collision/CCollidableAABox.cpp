#include "CCollidableAABox.hpp"

namespace urde
{
const CCollisionPrimitive::Type CCollidableAABox::sType(CCollidableAABox::SetStaticTableIndex, "CCollidableAABox");
u32 CCollidableAABox::sTableIndex = -1;

CCollidableAABox::CCollidableAABox()
{
}

CCollidableAABox::CCollidableAABox(const zeus::CAABox& aabox, const CMaterialList& list)
    : CCollisionPrimitive(list)
    , x10_aabox(aabox)
{
}

zeus::CAABox CCollidableAABox::Transform(const zeus::CTransform& xf) const
{
    return {xf.origin + x10_aabox.min, xf.origin + x10_aabox.max};
}

u32 CCollidableAABox::GetTableIndex() const
{
    return sTableIndex;
}

zeus::CAABox CCollidableAABox::CalculateAABox(const zeus::CTransform& xf) const
{
    return Transform(xf);
}

zeus::CAABox CCollidableAABox::CalculateLocalAABox() const
{
    return x10_aabox;
}

FourCC CCollidableAABox::GetPrimType() const
{
    return SBIG('AABX');
}

CRayCastResult CCollidableAABox::CastRayInternal(const CInternalRayCastStructure &) const
{
    return {};
}

const CCollisionPrimitive::Type& CCollidableAABox::GetType()
{
    return sType;
}

void CCollidableAABox::SetStaticTableIndex(u32 index)
{
    sTableIndex = index;
}

bool CCollidableAABox::CollideMovingAABox(const CInternalCollisionStructure &, const zeus::CVector3f &, double &, CCollisionInfo &)
{
    return false;
}

bool CCollidableAABox::CollideMovingSphere(const CInternalCollisionStructure &, const zeus::CVector3f &, double &, CCollisionInfo &)
{
    return false;
}

namespace Collide
{

bool AABox_AABox(const CInternalCollisionStructure &, CCollisionInfoList &)
{
    return false;
}

bool AABox_AABox_Bool(const CInternalCollisionStructure &)
{
    return false;
}

}
}
