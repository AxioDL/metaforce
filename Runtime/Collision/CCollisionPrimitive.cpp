#include "Runtime/Collision/CCollisionPrimitive.hpp"

#include <algorithm>
#include <climits>
#include <cstring>
#include <iterator>

#include "Runtime/Collision/CCollisionInfoList.hpp"
#include "Runtime/Collision/CInternalRayCastStructure.hpp"
#include "Runtime/Collision/CMaterialFilter.hpp"
#include "Runtime/Collision/InternalColliders.hpp"

namespace urde {
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
                                            const CMaterialFilter& filter, const zeus::CTransform& xf) const {
  return CastRayInternal(CInternalRayCastStructure(start, dir, length, xf, filter));
}

std::vector<CCollisionPrimitive::Type>::const_iterator CCollisionPrimitive::FindCollisionType(const char* name) {
  return std::find_if(sCollisionTypeList->cbegin(), sCollisionTypeList->cend(),
                      [name](const auto& type) { return std::strcmp(name, type.GetInfo()) == 0; });
}

bool CCollisionPrimitive::InternalCollide(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  u32 idx0 = collision.GetLeft().GetPrim().GetTableIndex();
  u32 idx1 = collision.GetRight().GetPrim().GetTableIndex();

  ComparisonFunc func;
  if (idx0 == UINT32_MAX || idx1 == UINT32_MAX) {
    sNullCollider = nullptr;
    func = sNullCollider;
  } else {
    func = (*sTableOfCollidables)[sNumTypes * idx1 + idx0];
  }

  if (func) {
    if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
        !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
      return false;
    return func(collision, list);
  }

  if (idx0 == UINT32_MAX || idx1 == UINT32_MAX) {
    sNullCollider = nullptr;
    func = sNullCollider;
  } else {
    func = (*sTableOfCollidables)[sNumTypes * idx0 + idx1];
  }

  if (func) {
    if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
        !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
      return false;
    CInternalCollisionStructure swappedCollision(collision.GetRight(), collision.GetLeft());
    u32 startListCount = list.GetCount();
    if (func(swappedCollision, list)) {
      for (auto it = list.begin() + startListCount; it != list.end(); ++it)
        it->Swap();
      return true;
    }
  }

  return false;
}

bool CCollisionPrimitive::Collide(const CInternalCollisionStructure::CPrimDesc& prim0,
                                  const CInternalCollisionStructure::CPrimDesc& prim1, CCollisionInfoList& list) {
  return InternalCollide({prim0, prim1}, list);
}

bool CCollisionPrimitive::InternalCollideBoolean(const CInternalCollisionStructure& collision) {
  u32 idx0 = collision.GetLeft().GetPrim().GetTableIndex();
  u32 idx1 = collision.GetRight().GetPrim().GetTableIndex();

  BooleanComparisonFunc func;
  if (idx0 == UINT32_MAX || idx1 == UINT32_MAX) {
    sNullBooleanCollider = nullptr;
    func = sNullBooleanCollider;
  } else {
    func = (*sTableOfBooleanCollidables)[sNumTypes * idx1 + idx0];
  }

  if (func) {
    if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
        !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
      return false;
    return func(collision);
  }

  if (idx0 == UINT32_MAX || idx1 == UINT32_MAX) {
    sNullBooleanCollider = nullptr;
    func = sNullBooleanCollider;
  } else {
    func = (*sTableOfBooleanCollidables)[sNumTypes * idx0 + idx1];
  }

  if (func) {
    if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
        !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
      return false;
    CInternalCollisionStructure swappedCollision(collision.GetRight(), collision.GetLeft());
    return func(swappedCollision);
  }

  CCollisionInfoList list;
  return InternalCollide(collision, list);
}

bool CCollisionPrimitive::CollideBoolean(const CInternalCollisionStructure::CPrimDesc& prim0,
                                         const CInternalCollisionStructure::CPrimDesc& prim1) {
  return InternalCollideBoolean({prim0, prim1});
}

bool CCollisionPrimitive::InternalCollideMoving(const CInternalCollisionStructure& collision,
                                                const zeus::CVector3f& dir, double& dOut, CCollisionInfo& infoOut) {
  u32 idx0 = collision.GetLeft().GetPrim().GetTableIndex();
  u32 idx1 = collision.GetRight().GetPrim().GetTableIndex();

  MovingComparisonFunc func;
  if (idx0 == UINT32_MAX || idx1 == UINT32_MAX) {
    sNullMovingCollider = nullptr;
    func = sNullMovingCollider;
  } else {
    func = (*sTableOfMovingCollidables)[sNumTypes * idx1 + idx0];
  }

  if (func) {
    if (!collision.GetLeft().GetFilter().Passes(collision.GetRight().GetPrim().GetMaterial()) ||
        !collision.GetRight().GetFilter().Passes(collision.GetLeft().GetPrim().GetMaterial()))
      return false;
    return func(collision, dir, dOut, infoOut);
  }

  return false;
}

bool CCollisionPrimitive::CollideMoving(const CInternalCollisionStructure::CPrimDesc& prim0,
                                        const CInternalCollisionStructure::CPrimDesc& prim1, const zeus::CVector3f& dir,
                                        double& dOut, CCollisionInfo& infoOut) {
  return InternalCollideMoving({prim0, prim1}, dir, dOut, infoOut);
}

void CCollisionPrimitive::InitBeginTypes() {
  sCollisionTypeList = std::make_unique<std::vector<Type>>();
  sCollisionTypeList->reserve(3);
  sTypesAdding = true;
  InternalColliders::AddTypes();
}

void CCollisionPrimitive::InitAddType(const Type& tp) {
  tp.GetSetter()(sCollisionTypeList->size());
  sCollisionTypeList->push_back(tp);
}

void CCollisionPrimitive::InitEndTypes() {
  sCollisionTypeList->shrink_to_fit();
  sNumTypes = sCollisionTypeList->size();
  sTypesAdding = false;
  sTypesAdded = true;
}

void CCollisionPrimitive::InitBeginColliders() {
  const size_t tableSz = sCollisionTypeList->size() * sCollisionTypeList->size();
  sTableOfCollidables = std::make_unique<std::vector<ComparisonFunc>>(tableSz);
  sTableOfBooleanCollidables = std::make_unique<std::vector<BooleanComparisonFunc>>(tableSz);
  sTableOfMovingCollidables = std::make_unique<std::vector<MovingComparisonFunc>>(tableSz);
  sCollidersAdding = true;
  InternalColliders::AddColliders();
}

void CCollisionPrimitive::InitAddBooleanCollider(const BooleanComparison& cmp) {
  const auto iter1 = FindCollisionType(cmp.GetType1());
  const auto iter2 = FindCollisionType(cmp.GetType2());
  const auto index1 = std::distance(sCollisionTypeList->cbegin(), iter1);
  const auto index2 = std::distance(sCollisionTypeList->cbegin(), iter2);
  const bool hasReachedEnd = iter1 == sCollisionTypeList->cend() || iter2 == sCollisionTypeList->cend();

  if (index1 >= sNumTypes || index2 >= sNumTypes || hasReachedEnd) {
    return;
  }

  BooleanComparisonFunc& funcOut =
      hasReachedEnd ? sNullBooleanCollider : (*sTableOfBooleanCollidables)[index2 * sNumTypes + index1];
  funcOut = cmp.GetCollider();
}

void CCollisionPrimitive::InitAddBooleanCollider(BooleanComparisonFunc cmp, const char* a, const char* b) {
  InitAddBooleanCollider({cmp, a, b});
}

void CCollisionPrimitive::InitAddMovingCollider(const MovingComparison& cmp) {
  const auto iter1 = FindCollisionType(cmp.GetType1());
  const auto iter2 = FindCollisionType(cmp.GetType2());
  const auto index1 = std::distance(sCollisionTypeList->cbegin(), iter1);
  const auto index2 = std::distance(sCollisionTypeList->cbegin(), iter2);
  const bool hasReachedEnd = iter1 == sCollisionTypeList->cend() || iter2 == sCollisionTypeList->cend();

  if (index1 >= sNumTypes || index2 >= sNumTypes || hasReachedEnd) {
    return;
  }

  MovingComparisonFunc& funcOut =
      hasReachedEnd ? sNullMovingCollider : (*sTableOfMovingCollidables)[index2 * sNumTypes + index1];
  funcOut = cmp.GetCollider();
}

void CCollisionPrimitive::InitAddMovingCollider(MovingComparisonFunc cmp, const char* a, const char* b) {
  InitAddMovingCollider({cmp, a, b});
}

void CCollisionPrimitive::InitAddCollider(const Comparison& cmp) {
  const auto iter1 = FindCollisionType(cmp.GetType1());
  const auto iter2 = FindCollisionType(cmp.GetType2());
  const auto index1 = std::distance(sCollisionTypeList->cbegin(), iter1);
  const auto index2 = std::distance(sCollisionTypeList->cbegin(), iter2);
  const bool hasReachedEnd = iter1 == sCollisionTypeList->cend() || iter2 == sCollisionTypeList->cend();

  if (index1 >= sNumTypes || index2 >= sNumTypes || hasReachedEnd) {
    return;
  }

  ComparisonFunc& funcOut = hasReachedEnd ? sNullCollider : (*sTableOfCollidables)[index2 * sNumTypes + index1];
  funcOut = cmp.GetCollider();
}

void CCollisionPrimitive::InitAddCollider(ComparisonFunc cmp, const char* a, const char* b) {
  InitAddCollider({cmp, a, b});
}

void CCollisionPrimitive::InitEndColliders() {
  sCollidersAdding = false;
  sCollidersAdded = true;
  sInitComplete = true;
}

void CCollisionPrimitive::Uninitialize() {
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

} // namespace urde
