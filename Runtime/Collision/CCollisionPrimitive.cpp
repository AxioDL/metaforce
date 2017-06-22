#include "CCollisionPrimitive.hpp"
#include "CInternalRayCastStructure.hpp"
#include "CMaterialFilter.hpp"
#include "InternalColliders.hpp"
#include "CCollisionInfoList.hpp"

namespace urde
{
s32 CCollisionPrimitive::sNumTypes = 0;
bool CCollisionPrimitive::sInitComplete = false;
bool CCollisionPrimitive::sTypesAdded = false;
bool CCollisionPrimitive::sTypesAdding = false;
bool CCollisionPrimitive::sCollidersAdded = false;
bool CCollisionPrimitive::sCollidersAdding = false;
std::unique_ptr<std::vector<CCollisionPrimitive::Type>> CCollisionPrimitive::sCollisionTypeList;
std::unique_ptr<std::vector<ComparisonFunc>> CCollisionPrimitive::sTableOfCollidables;
std::unique_ptr<std::vector<BooleanComparisonFunc>> CCollisionPrimitive::sTableOfBooleanCollidables;
std::unique_ptr<std::vector<MovingComparisonFunc>> CCollisionPrimitive::sTableOfMovingCollidables;
ComparisonFunc CCollisionPrimitive::sNullCollider = {};
BooleanComparisonFunc CCollisionPrimitive::sNullBooleanCollider = {};
MovingComparisonFunc CCollisionPrimitive::sNullMovingCollider = {};
CCollisionPrimitive::CCollisionPrimitive(const CMaterialList& list) : x8_material(list) {}

void CCollisionPrimitive::SetMaterial(const CMaterialList& material) { x8_material = material; }

const CMaterialList& CCollisionPrimitive::GetMaterial() const { return x8_material; }

CRayCastResult CCollisionPrimitive::CastRay(const zeus::CVector3f& start, const zeus::CVector3f& dir, float length,
                                            const CMaterialFilter& filter, const zeus::CTransform& xf) const
{
    return CastRayInternal(CInternalRayCastStructure(start, dir, length, xf, filter));
}

bool CCollisionPrimitive::InternalCollide(const CInternalCollisionStructure& collision,
                                          CCollisionInfoList& list)
{
    u32 idx0 = collision.GetLeft().GetPrim().GetTableIndex();
    u32 idx1 = collision.GetRight().GetPrim().GetTableIndex();

    ComparisonFunc func;
    if (idx0 == -1 || idx1 == -1)
    {
        sNullCollider = nullptr;
        func = sNullCollider;
    }
    else
    {
        func = (*sTableOfCollidables)[sNumTypes * idx1 + idx0];
    }

    if (func)
    {
        if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
            !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
            return false;
        return func(collision, list);
    }

    if (idx0 == -1 || idx1 == -1)
    {
        sNullCollider = nullptr;
        func = sNullCollider;
    }
    else
    {
        func = (*sTableOfCollidables)[sNumTypes * idx0 + idx1];
    }

    if (func)
    {
        if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
            !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
            return false;
        CInternalCollisionStructure swappedCollision(collision.GetRight(), collision.GetLeft());
        u32 startListCount = list.GetCount();
        if (func(swappedCollision, list))
        {
            for (auto it = list.begin() + startListCount ; it != list.end() ; ++it)
                it->Swap();
            return true;
        }
    }

    return false;
}

bool CCollisionPrimitive::Collide(CInternalCollisionStructure::CPrimDesc& prim0,
                                  CInternalCollisionStructure::CPrimDesc& prim1,
                                  CCollisionInfoList& list)
{
    return InternalCollide({prim0, prim1}, list);
}

bool CCollisionPrimitive::InternalCollideBoolean(const CInternalCollisionStructure& collision)
{
    u32 idx0 = collision.GetLeft().GetPrim().GetTableIndex();
    u32 idx1 = collision.GetRight().GetPrim().GetTableIndex();

    BooleanComparisonFunc func;
    if (idx0 == -1 || idx1 == -1)
    {
        sNullBooleanCollider = nullptr;
        func = sNullBooleanCollider;
    }
    else
    {
        func = (*sTableOfBooleanCollidables)[sNumTypes * idx1 + idx0];
    }

    if (func)
    {
        if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
            !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
            return false;
        return func(collision);
    }

    if (idx0 == -1 || idx1 == -1)
    {
        sNullBooleanCollider = nullptr;
        func = sNullBooleanCollider;
    }
    else
    {
        func = (*sTableOfBooleanCollidables)[sNumTypes * idx0 + idx1];
    }

    if (func)
    {
        if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
            !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
            return false;
        CInternalCollisionStructure swappedCollision(collision.GetRight(), collision.GetLeft());
        return func(swappedCollision);
    }

    CCollisionInfoList list;
    return InternalCollide(collision, list);
}

bool CCollisionPrimitive::CollideBoolean(CInternalCollisionStructure::CPrimDesc& prim0,
                                         CInternalCollisionStructure::CPrimDesc& prim1)
{
    return InternalCollideBoolean({prim0, prim1});
}

bool CCollisionPrimitive::CollideMoving(CInternalCollisionStructure::CPrimDesc& prim0,
                                        CInternalCollisionStructure::CPrimDesc& prim1,
                                        const zeus::CVector3f& vec,
                                        double& dOut,
                                        CCollisionInfo& infoOut)
{
    // TODO: Finish
    return false;
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
    sCollisionTypeList->push_back(tp);
}

void CCollisionPrimitive::InitEndTypes()
{
    sCollisionTypeList->shrink_to_fit();
    sNumTypes = sCollisionTypeList->size();
    sTypesAdding = false;
    sTypesAdded = true;
}

void CCollisionPrimitive::InitBeginColliders()
{
    sTableOfCollidables.reset(new std::vector<ComparisonFunc>());
    sTableOfBooleanCollidables.reset(new std::vector<BooleanComparisonFunc>());
    sTableOfMovingCollidables.reset(new std::vector<MovingComparisonFunc>());
    size_t tableSz = sCollisionTypeList->size() * sCollisionTypeList->size();
    sTableOfCollidables->resize(tableSz);
    sTableOfBooleanCollidables->resize(tableSz);
    sTableOfMovingCollidables->resize(tableSz);
    sCollidersAdding = true;
    InternalColliders::AddColliders();
}

void CCollisionPrimitive::InitAddBooleanCollider(const CCollisionPrimitive::BooleanComparison& cmp)
{
    int idx0 = -1;
    for (int i=0 ; i<sCollisionTypeList->size() ; ++i)
    {
        if (!strcmp(cmp.GetType1(), (*sCollisionTypeList)[i].GetInfo()))
        {
            idx0 = i;
            break;
        }
    }

    int idx1 = -1;
    for (int i=0 ; i<sCollisionTypeList->size() ; ++i)
    {
        if (!strcmp(cmp.GetType2(), (*sCollisionTypeList)[i].GetInfo()))
        {
            idx1 = i;
            break;
        }
    }

    if (idx0 < 0 || idx1 < 0 || idx0 >= sNumTypes || idx1 >= sNumTypes)
        return;

    BooleanComparisonFunc& funcOut =
        (idx0 == -1 || idx1 == -1) ? sNullBooleanCollider : (*sTableOfBooleanCollidables)[idx1 * sNumTypes + idx0];
    funcOut = cmp.GetCollider();
}

void CCollisionPrimitive::InitAddBooleanCollider(BooleanComparisonFunc cmp, const char* a, const char* b)
{
    InitAddBooleanCollider({cmp, a, b});
}

void CCollisionPrimitive::InitAddMovingCollider(const CCollisionPrimitive::MovingComparison& cmp)
{
    int idx0 = -1;
    for (int i=0 ; i<sCollisionTypeList->size() ; ++i)
    {
        if (!strcmp(cmp.GetType1(), (*sCollisionTypeList)[i].GetInfo()))
        {
            idx0 = i;
            break;
        }
    }

    int idx1 = -1;
    for (int i=0 ; i<sCollisionTypeList->size() ; ++i)
    {
        if (!strcmp(cmp.GetType2(), (*sCollisionTypeList)[i].GetInfo()))
        {
            idx1 = i;
            break;
        }
    }

    if (idx0 < 0 || idx1 < 0 || idx0 >= sNumTypes || idx1 >= sNumTypes)
        return;

    MovingComparisonFunc& funcOut =
        (idx0 == -1 || idx1 == -1) ? sNullMovingCollider : (*sTableOfMovingCollidables)[idx1 * sNumTypes + idx0];
    funcOut = cmp.GetCollider();
}

void CCollisionPrimitive::InitAddMovingCollider(MovingComparisonFunc cmp, const char* a, const char* b)
{
    InitAddMovingCollider({cmp, a, b});
}

void CCollisionPrimitive::InitAddCollider(const CCollisionPrimitive::Comparison& cmp)
{
    int idx0 = -1;
    for (int i=0 ; i<sCollisionTypeList->size() ; ++i)
    {
        if (!strcmp(cmp.GetType1(), (*sCollisionTypeList)[i].GetInfo()))
        {
            idx0 = i;
            break;
        }
    }

    int idx1 = -1;
    for (int i=0 ; i<sCollisionTypeList->size() ; ++i)
    {
        if (!strcmp(cmp.GetType2(), (*sCollisionTypeList)[i].GetInfo()))
        {
            idx1 = i;
            break;
        }
    }

    if (idx0 < 0 || idx1 < 0 || idx0 >= sNumTypes || idx1 >= sNumTypes)
        return;

    ComparisonFunc& funcOut =
        (idx0 == -1 || idx1 == -1) ? sNullCollider : (*sTableOfCollidables)[idx1 * sNumTypes + idx0];
    funcOut = cmp.GetCollider();
}

void CCollisionPrimitive::InitAddCollider(ComparisonFunc cmp, const char* a, const char* b)
{
    InitAddCollider({cmp, a, b});
}

void CCollisionPrimitive::InitEndColliders()
{
    sCollidersAdding = false;
    sCollidersAdded = true;
    sInitComplete = true;
}

void CCollisionPrimitive::Uninitialize()
{
    sInitComplete = false;
    sCollidersAdding = false;
    sCollidersAdded = false;
    sTypesAdding = false;
    sTypesAdded = false;
    sNumTypes = 0;
    sCollisionTypeList.reset();
    sTableOfCollidables.reset();
    sTableOfMovingCollidables.reset();
    sTableOfBooleanCollidables.reset();
}

}
