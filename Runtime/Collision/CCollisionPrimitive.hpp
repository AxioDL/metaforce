#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "Runtime/Collision/CMaterialList.hpp"
#include "Runtime/Collision/CRayCastResult.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CCollisionPrimitive;
class CMaterialFilter;

class CInternalCollisionStructure {
public:
  class CPrimDesc {
    const CCollisionPrimitive& x0_prim;
    const CMaterialFilter& x4_filter;
    zeus::CTransform x8_xf;

  public:
    CPrimDesc(const CCollisionPrimitive& prim, const CMaterialFilter& filter, const zeus::CTransform& xf)
    : x0_prim(prim), x4_filter(filter), x8_xf(xf) {}
    const CCollisionPrimitive& GetPrim() const { return x0_prim; }
    const CMaterialFilter& GetFilter() const { return x4_filter; }
    const zeus::CTransform& GetTransform() const { return x8_xf; }
  };

private:
  CPrimDesc x0_p0;
  CPrimDesc x38_p1;

public:
  CInternalCollisionStructure(const CPrimDesc& p0, const CPrimDesc& p1) : x0_p0(p0), x38_p1(p1) {}
  const CPrimDesc& GetLeft() const { return x0_p0; }
  const CPrimDesc& GetRight() const { return x38_p1; }
};

class COBBTree;
class CCollisionInfo;
class CCollisionInfoList;
class CInternalRayCastStructure;

using BooleanComparisonFunc = bool (*)(const CInternalCollisionStructure&);
using ComparisonFunc = bool (*)(const CInternalCollisionStructure&, CCollisionInfoList&);
using MovingComparisonFunc = bool (*)(const CInternalCollisionStructure&, const zeus::CVector3f&, double&,
                                      CCollisionInfo&);
using PrimitiveSetter = void (*)(u32);

class CCollisionPrimitive {
public:
  class Type {
    PrimitiveSetter x0_setter;
    const char* x4_info;

  public:
    Type() = default;
    Type(PrimitiveSetter setter, const char* info) : x0_setter(setter), x4_info(info) {}

    const char* GetInfo() const { return x4_info; }
    PrimitiveSetter GetSetter() const { return x0_setter; }
  };

  class Comparison {
    ComparisonFunc x0_collider;
    const char* x4_type1;
    const char* x8_type2;

  public:
    Comparison(ComparisonFunc collider, const char* type1, const char* type2)
    : x0_collider(collider), x4_type1(type1), x8_type2(type2) {}

    ComparisonFunc GetCollider() const { return x0_collider; }
    const char* GetType1() const { return x4_type1; }
    const char* GetType2() const { return x8_type2; }
  };

  class MovingComparison {
    MovingComparisonFunc x0_collider;
    const char* x4_type1;
    const char* x8_type2;

  public:
    MovingComparison(MovingComparisonFunc collider, const char* type1, const char* type2)
    : x0_collider(collider), x4_type1(type1), x8_type2(type2) {}

    MovingComparisonFunc GetCollider() const { return x0_collider; }
    const char* GetType1() const { return x4_type1; }
    const char* GetType2() const { return x8_type2; }
  };

  class BooleanComparison {
    BooleanComparisonFunc x0_collider;
    const char* x4_type1;
    const char* x8_type2;

  public:
    BooleanComparison(BooleanComparisonFunc collider, const char* type1, const char* type2)
    : x0_collider(collider), x4_type1(type1), x8_type2(type2) {}

    BooleanComparisonFunc GetCollider() const { return x0_collider; }
    const char* GetType1() const { return x4_type1; }
    const char* GetType2() const { return x8_type2; }
  };

private:
  CMaterialList x8_material;
  static s32 sNumTypes;
  static bool sInitComplete;
  static bool sTypesAdded;
  static bool sTypesAdding;
  static bool sCollidersAdded;
  static bool sCollidersAdding;
  static std::unique_ptr<std::vector<Type>> sCollisionTypeList;
  static std::unique_ptr<std::vector<ComparisonFunc>> sTableOfCollidables;
  static std::unique_ptr<std::vector<BooleanComparisonFunc>> sTableOfBooleanCollidables;
  static std::unique_ptr<std::vector<MovingComparisonFunc>> sTableOfMovingCollidables;
  static ComparisonFunc sNullCollider;
  static BooleanComparisonFunc sNullBooleanCollider;
  static MovingComparisonFunc sNullMovingCollider;

  // Attempts to locate an entry within the collision type list that matches the supplied name.
  // Returns the end iterator in the event of no matches.
  static std::vector<Type>::const_iterator FindCollisionType(const char* name);

  static bool InternalCollide(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
  static bool InternalCollideBoolean(const CInternalCollisionStructure& collision);
  static bool InternalCollideMoving(const CInternalCollisionStructure& collision, const zeus::CVector3f& dir,
                                    double& dOut, CCollisionInfo& infoOut);

public:
  CCollisionPrimitive() = default;
  CCollisionPrimitive(const CMaterialList& list);
  virtual u32 GetTableIndex() const = 0;
  virtual void SetMaterial(const CMaterialList&);
  virtual const CMaterialList& GetMaterial() const;
  virtual zeus::CAABox CalculateAABox(const zeus::CTransform&) const = 0;
  virtual zeus::CAABox CalculateLocalAABox() const = 0;
  virtual FourCC GetPrimType() const = 0;
  virtual ~CCollisionPrimitive() = default;
  virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const = 0;
  CRayCastResult CastRay(const zeus::CVector3f& start, const zeus::CVector3f& dir, float length,
                         const CMaterialFilter& filter, const zeus::CTransform& xf) const;

  static bool Collide(const CInternalCollisionStructure::CPrimDesc& prim0,
                      const CInternalCollisionStructure::CPrimDesc& prim1, CCollisionInfoList& list);
  static bool CollideBoolean(const CInternalCollisionStructure::CPrimDesc& prim0,
                             const CInternalCollisionStructure::CPrimDesc& prim1);
  static bool CollideMoving(const CInternalCollisionStructure::CPrimDesc& prim0,
                            const CInternalCollisionStructure::CPrimDesc& prim1, const zeus::CVector3f& dir,
                            double& dOut, CCollisionInfo& infoOut);

  static void InitBeginTypes();
  static void InitAddType(const Type& tp);
  static void InitEndTypes();

  static void InitBeginColliders();
  static void InitAddBooleanCollider(const BooleanComparison& cmp);
  static void InitAddBooleanCollider(BooleanComparisonFunc, const char*, const char*);
  static void InitAddMovingCollider(const MovingComparison& cmp);
  static void InitAddMovingCollider(MovingComparisonFunc, const char*, const char*);
  static void InitAddCollider(const Comparison& cmp);
  static void InitAddCollider(ComparisonFunc, const char*, const char*);
  static void InitEndColliders();

  static void Uninitialize();
};
} // namespace urde
