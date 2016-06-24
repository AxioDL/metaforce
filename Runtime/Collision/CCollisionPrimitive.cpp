#include "CCollisionPrimitive.hpp"
#include "CInternalRayCastStructure.hpp"
#include "CMaterialFilter.hpp"
#include "InternalColliders.hpp"

namespace urde
{
std::unique_ptr<std::vector<CCollisionPrimitive::Type>> CCollisionPrimitive::sCollisionTypeList;
bool CCollisionPrimitive::sTypesAdding = false;
CCollisionPrimitive::CCollisionPrimitive(const CMaterialList& list)
    : x8_material(list)
{
}

void CCollisionPrimitive::SetMaterial(const CMaterialList& material)
{
    x8_material = material;
}

const CMaterialList& CCollisionPrimitive::GetMaterial() const
{
    return x8_material;

}

CRayCastResult CCollisionPrimitive::CastRay(const zeus::CVector3f& start, const zeus::CVector3f& end, float d, const
                                            CMaterialFilter& filter, const zeus::CTransform& xf) const
{
    return CastRayInternal(CInternalRayCastStructure(start, end, d, xf, filter));
}

void CCollisionPrimitive::InitBeginTypes()
{
    sCollisionTypeList.reset(new std::vector<CCollisionPrimitive::Type>());
    sCollisionTypeList->reserve(3);
    sTypesAdding = true;
    InternalColliders::AddTypes();
}

void CCollisionPrimitive::InitAddType(const CCollisionPrimitive::Type& tp)
{
    tp.GetSetter()(sCollisionTypeList->size());

    sCollisionTypeList->reserve(sCollisionTypeList->size() + 1);
    sCollisionTypeList->push_back(tp);
}

void CCollisionPrimitive::InitEndTypes()
{
    sTypesAdding = false;
}

CCollisionPrimitive::Type::Type(std::function<void (unsigned int)> setter, const char *info)
    : x0_setter(setter),
      x4_info(info)
{
}

const char *CCollisionPrimitive::Type::GetInfo() const
{
    return x4_info;
}

std::function<void (u32)> CCollisionPrimitive::Type::GetSetter() const
{
    return x0_setter;
}

}
