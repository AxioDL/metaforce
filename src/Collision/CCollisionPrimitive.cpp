#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CInternalCollisionStructure.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/InternalColliders.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include <limits.h>
#include <string.h>

int CCollisionPrimitive::sNumTypes = 0;
bool CCollisionPrimitive::sInitComplete = false;
bool CCollisionPrimitive::sTypesAdded = false;
bool CCollisionPrimitive::sTypesAdding = false;
bool CCollisionPrimitive::sCollidersAdded = false;
bool CCollisionPrimitive::sCollidersAdding = false;

rstl::single_ptr< rstl::vector< CCollisionPrimitive::Type > >
    CCollisionPrimitive::sCollisionTypeList;
rstl::single_ptr< ComparisonFunc > CCollisionPrimitive::sTableOfCollidables;
rstl::single_ptr< BooleanComparisonFunc > CCollisionPrimitive::sTableOfBooleanCollidables;
rstl::single_ptr< MovingComparisonFunc > CCollisionPrimitive::sTableOfMovingCollidables;
static ComparisonFunc sNullCollider = nullptr;
static BooleanComparisonFunc sNullBooleanCollider = nullptr;
static MovingComparisonFunc sNullMovingCollider = nullptr;

CCollisionPrimitive::CCollisionPrimitive(const CMaterialList& list) : mMaterial(list) {}


void CCollisionPrimitive::InitBeginTypes() {
  sCollisionTypeList = rs_new rstl::vector< Type >();
  sCollisionTypeList->reserve(3);
  sTypesAdding = true;
  InternalColliders::AddTypes();
}

void CCollisionPrimitive::InitAddType(const Type& type) {
  type.GetSetter()(sCollisionTypeList->size());
  sCollisionTypeList->reserve(sCollisionTypeList->size() + 1);
  sCollisionTypeList->push_back(type);
}

void CCollisionPrimitive::InitEndTypes() {
  rstl::vector< Type > types = *sCollisionTypeList;
  sCollisionTypeList = rs_new rstl::vector< Type >(types);
  sNumTypes = types.size();
  sTypesAdding = false;
  sTypesAdded = true;
}

void CCollisionPrimitive::InitBeginColliders() {
  int numColliders = sCollisionTypeList->size() * sCollisionTypeList->size();
  sTableOfCollidables = rs_new ComparisonFunc[numColliders];
  sTableOfBooleanCollidables = rs_new BooleanComparisonFunc[numColliders];
  sTableOfMovingCollidables = rs_new MovingComparisonFunc[numColliders];
  memset(sTableOfCollidables.get(), 0, numColliders * sizeof(ComparisonFunc));
  memset(sTableOfBooleanCollidables.get(), 0, numColliders * sizeof(BooleanComparisonFunc));
  memset(sTableOfMovingCollidables.get(), 0, numColliders * sizeof(MovingComparisonFunc));
  sCollidersAdding = true;
  InternalColliders::AddColliders();
}

void CCollisionPrimitive::InitEndColliders() {
  for (int i = 0; i < sNumTypes; ++i) {
    for (int j = 0; j < sNumTypes; ++j) {
      ComparisonFunc first = *ColliderFromTable(i, j);
      ComparisonFunc second = *ColliderFromTable(j, i);
    }
  }

  sCollidersAdding = false;
  sCollidersAdded = true;
  sInitComplete = true;
}

inline ComparisonFunc* CCollisionPrimitive::ColliderFromTable(const int index1, const int index2) {
  ComparisonFunc* func = nullptr;
  if (index1 == UINT_MAX || index2 == UINT_MAX) {
    func = &sNullCollider;
    sNullCollider = nullptr;
  } else {
    func = &sTableOfCollidables.get()[index1 + index2 * sNumTypes];
  }
  return func;
}

void CCollisionPrimitive::InitAddCollider(const Comparison& comp) {
  const int index1 = TypeIndexFromTypeInfo(comp.GetType1());
  const int index2 = TypeIndexFromTypeInfo(comp.GetType2());
  const bool inRange =
      (index1 < sNumTypes) && (index2 < sNumTypes) && (index1 >= 0) && (index2 >= 0);

  if (inRange) {
    ComparisonFunc* func = ColliderFromTable(index1, index2);
    *func = comp.GetCollider();
  }
}

inline BooleanComparisonFunc* CCollisionPrimitive::BooleanColliderFromTable(const int index1,
                                                                            const int index2) {
  BooleanComparisonFunc* func = nullptr;
  if (index1 == UINT_MAX || index2 == UINT_MAX) {
    func = &sNullBooleanCollider;
    sNullBooleanCollider = nullptr;
  } else {
    func = &sTableOfBooleanCollidables.get()[index1 + index2 * sNumTypes];
  }

  return func;
}

void CCollisionPrimitive::InitAddBooleanCollider(const BooleanComparison& comp) {
  const int index1 = TypeIndexFromTypeInfo(comp.GetType1());
  const int index2 = TypeIndexFromTypeInfo(comp.GetType2());
  const bool inRange =
      (index1 < sNumTypes) && (index2 < sNumTypes) && (index1 >= 0) && (index2 >= 0);

  if (inRange) {
    BooleanComparisonFunc* func = BooleanColliderFromTable(index1, index2);
    *func = comp.GetCollider();
  }
}

inline MovingComparisonFunc* CCollisionPrimitive::MovingColliderFromTable(const int index1,
                                                                          const int index2) {
  MovingComparisonFunc* func = nullptr;
  if (index1 == UINT_MAX || index2 == UINT_MAX) {
    func = &sNullMovingCollider;
    sNullMovingCollider = nullptr;
  } else {
    func = &sTableOfMovingCollidables.get()[index1 + index2 * sNumTypes];
  }
  return func;
}
void CCollisionPrimitive::InitAddMovingCollider(const MovingComparison& comp) {
  const int index1 = TypeIndexFromTypeInfo(comp.GetType1());
  const int index2 = TypeIndexFromTypeInfo(comp.GetType2());
  const bool inRange =
      (index1 < sNumTypes) && (index2 < sNumTypes) && (index1 >= 0) && (index2 >= 0);

  if (inRange) {
    MovingComparisonFunc* func = MovingColliderFromTable(index1, index2);
    *func = comp.GetCollider();
  }
}

bool CCollisionPrimitive::InternalCollide(const CInternalCollisionStructure& collision,
                                          CCollisionInfoList& list) {
  const CCollisionPrimitive& leftPrim = collision.GetLeft().GetPrim();
  const CCollisionPrimitive& rightPrim = collision.GetRight().GetPrim();
  const CMaterialFilter& leftFilter = collision.GetLeft().GetFilter();
  const CMaterialFilter& rightFilter = collision.GetRight().GetFilter();

  const int leftIndex = leftPrim.GetTableIndex();
  const int rightIndex = rightPrim.GetTableIndex();

  if (ComparisonFunc func = *ColliderFromTable(leftIndex, rightIndex)) {
    if (!leftFilter.Passes(rightPrim.GetMaterial()) ||
        !rightFilter.Passes(leftPrim.GetMaterial())) {
      return false;
    }

    return func(collision, list);
  }

  if (ComparisonFunc func = *ColliderFromTable(rightIndex, leftIndex)) {
    if (!leftFilter.Passes(rightPrim.GetMaterial()) ||
        !rightFilter.Passes(leftPrim.GetMaterial())) {
      return false;
    }

    CInternalCollisionStructure swapped = collision.GetSwapped();
    int startListCount = list.GetCount();
    if (func(swapped, list)) {
      list.Swap(startListCount);
      return true;
    } else {
      return false;
    }
  }

  return false;
}

bool CCollisionPrimitive::InternalCollideMoving(const CInternalCollisionStructure& collision,
                                                const CVector3f& dir, double& dOut,
                                                CCollisionInfo& infoOut) {
  const CCollisionPrimitive& leftPrim = collision.GetLeft().GetPrim();
  const CCollisionPrimitive& rightPrim = collision.GetRight().GetPrim();
  const CMaterialFilter& leftFilter = collision.GetLeft().GetFilter();
  const CMaterialFilter& rightFilter = collision.GetRight().GetFilter();

  const int leftIndex = leftPrim.GetTableIndex();
  const int rightIndex = rightPrim.GetTableIndex();

  MovingComparisonFunc func = *MovingColliderFromTable(leftIndex, rightIndex);
  if (func) {
    if (!leftFilter.Passes(rightPrim.GetMaterial()) ||
        !rightFilter.Passes(leftPrim.GetMaterial())) {
      return false;
    }

    return func(collision, dir, dOut, infoOut);
  }

  return false;
}

bool CCollisionPrimitive::InternalCollideBoolean(const CInternalCollisionStructure& collision) {
  const CCollisionPrimitive& leftPrim = collision.GetLeft().GetPrim();
  const CCollisionPrimitive& rightPrim = collision.GetRight().GetPrim();
  const CMaterialFilter& leftFilter = collision.GetLeft().GetFilter();
  const CMaterialFilter& rightFilter = collision.GetRight().GetFilter();

  const int leftIndex = leftPrim.GetTableIndex();
  const int rightIndex = rightPrim.GetTableIndex();

  if (BooleanComparisonFunc func = *BooleanColliderFromTable(leftIndex, rightIndex)) {
    if (!leftFilter.Passes(rightPrim.GetMaterial()) ||
        !rightFilter.Passes(leftPrim.GetMaterial())) {
      return false;
    }

    return func(collision);
  }

  if (BooleanComparisonFunc func = *BooleanColliderFromTable(rightIndex, leftIndex)) {
    if (!leftFilter.Passes(rightPrim.GetMaterial()) ||
        !rightFilter.Passes(leftPrim.GetMaterial())) {
      return false;
    }

    CInternalCollisionStructure swapped = collision.GetSwapped();
    return func(swapped) ? true : false;
  }

  CCollisionInfoList list;
  return InternalCollide(collision, list);
}

void CCollisionPrimitive::Uninitialize() {
  sInitComplete = false;
  sCollidersAdding = false;
  sCollidersAdded = false;
  sTypesAdding = false;
  sTypesAdded = false;
  sNumTypes = 0;
  sCollisionTypeList = nullptr;
  sTableOfCollidables = nullptr;
  sTableOfBooleanCollidables = nullptr;
  sTableOfMovingCollidables = nullptr;
}
bool CCollisionPrimitive::CollideBoolean(const CInternalCollisionStructure::CPrimDesc& prim0,
                                         const CInternalCollisionStructure::CPrimDesc& prim1) {
  return InternalCollideBoolean(CInternalCollisionStructure(prim0, prim1));
}

bool CCollisionPrimitive::Collide(const CInternalCollisionStructure::CPrimDesc& prim0,
                                  const CInternalCollisionStructure::CPrimDesc& prim1,
                                  CCollisionInfoList& list) {
  return InternalCollide(CInternalCollisionStructure(prim0, prim1), list);
}

bool CCollisionPrimitive::CollideMoving(const CInternalCollisionStructure::CPrimDesc& prim0,
                                        const CInternalCollisionStructure::CPrimDesc& prim1,
                                        const CVector3f& dir, double& dOut,
                                        CCollisionInfo& infoOut) {
  return InternalCollideMoving(CInternalCollisionStructure(prim0, prim1), dir, dOut, infoOut);
}

void CCollisionPrimitive::InitAddCollider(ComparisonFunc comp, const char* type1,
                                          const char* type2) {
  Comparison collider(comp, type1, type2);
  InitAddCollider(collider);
}
void CCollisionPrimitive::InitAddBooleanCollider(BooleanComparisonFunc comp, const char* type1,
                                                 const char* type2) {
  BooleanComparison collider(comp, type1, type2);
  InitAddBooleanCollider(collider);
}
void CCollisionPrimitive::InitAddMovingCollider(MovingComparisonFunc comp, const char* type1,
                                                const char* type2) {
  MovingComparison collider(comp, type1, type2);
  InitAddMovingCollider(collider);
}
