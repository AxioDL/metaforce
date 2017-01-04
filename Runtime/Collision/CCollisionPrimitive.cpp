#include "CCollisionPrimitive.hpp"
#include "CInternalRayCastStructure.hpp"
#include "CMaterialFilter.hpp"
#include "InternalColliders.hpp"

namespace urde
{
std::unique_ptr<std::vector<CCollisionPrimitive::Type>> CCollisionPrimitive::sCollisionTypeList;
std::unique_ptr<std::vector<ComparisonFunc>> CCollisionPrimitive::sTableOfCollidables;
std::unique_ptr<std::vector<BooleanComparisonFunc>> CCollisionPrimitive::sTableOfBooleanCollidables;
std::unique_ptr<std::vector<MovingComparisonFunc>> CCollisionPrimitive::sTableOfMovingCollidables;
bool CCollisionPrimitive::sTypesAdded = false;
bool CCollisionPrimitive::sTypesAdding = false;
bool CCollisionPrimitive::sCollidersAdded = false;
bool CCollisionPrimitive::sCollidersAdding = false;
CCollisionPrimitive::CCollisionPrimitive(const CMaterialList& list) : x8_material(list) {}

void CCollisionPrimitive::SetMaterial(const CMaterialList& material) { x8_material = material; }

const CMaterialList& CCollisionPrimitive::GetMaterial() const { return x8_material; }

CRayCastResult CCollisionPrimitive::CastRay(const zeus::CVector3f& start, const zeus::CVector3f& end, float d,
                                            const CMaterialFilter& filter, const zeus::CTransform& xf) const
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

void CCollisionPrimitive::InitEndTypes() { sTypesAdding = false; }

void CCollisionPrimitive::InitBeginColliders()
{
    sTableOfCollidables.reset(new std::vector<ComparisonFunc>());
    sTableOfBooleanCollidables.reset(new std::vector<BooleanComparisonFunc>());
    sTableOfMovingCollidables.reset(new std::vector<MovingComparisonFunc>());
    sCollidersAdding = true;
    InternalColliders::AddColliders();
}

void CCollisionPrimitive::InitAddBooleanCollider(const CCollisionPrimitive::BooleanComparison& cmp)
{
}

void CCollisionPrimitive::InitAddBooleanCollider(const BooleanComparisonFunc& cmp, const char* a, const char* b)
{
    InitAddBooleanCollider({std::move(cmp), a, b});
}

void CCollisionPrimitive::InitAddMovingCollider(const CCollisionPrimitive::MovingComparison& cmp) {}

void CCollisionPrimitive::InitAddMovingCollider(const MovingComparisonFunc& cmp, const char* a, const char* b)
{
    InitAddMovingCollider({std::move(cmp), a, b});
}

void CCollisionPrimitive::InitAddCollider(const CCollisionPrimitive::Comparison& cmp) {}

void CCollisionPrimitive::InitAddCollider(const ComparisonFunc& cmp, const char* a, const char* b)
{
    InitAddCollider({std::move(cmp), a, b});
}

CCollisionPrimitive::Type::Type(const std::function<void(u32)>& setter, const char* info)
: x0_setter(setter), x4_info(info)
{
}

const char* CCollisionPrimitive::Type::GetInfo() const { return x4_info; }

std::function<void(u32)> CCollisionPrimitive::Type::GetSetter() const { return x0_setter; }
}
