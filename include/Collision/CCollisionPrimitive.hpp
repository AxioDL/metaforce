#ifndef _CCOLLISIONPRIMITIVE
#define _CCOLLISIONPRIMITIVE

#include "types.h"

#include "Collision/CInternalCollisionStructure.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CMaterialList.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"
#include <limits.h>
#include <string.h>

#include "Collision/CRayCastResult.hpp"
class CCollisionInfoList;
class CInternalRayCastStructure;
class CCollisionInfo;

typedef void (*PrimitiveSetter)(uint);

typedef bool (*ComparisonFunc)(const CInternalCollisionStructure&, CCollisionInfoList&);
typedef bool (*BooleanComparisonFunc)(const CInternalCollisionStructure&);
typedef bool (*MovingComparisonFunc)(const CInternalCollisionStructure&, const CVector3f&, double&,
                                     CCollisionInfo&);

class CCollisionPrimitive {
public:
  class Type {
  public:
    Type(PrimitiveSetter setter, const char* info) : mSetter(setter), mInfo(info) {}

    PrimitiveSetter GetSetter() const { return mSetter; }
    const char* GetInfo() const { return mInfo; }

  private:
    PrimitiveSetter mSetter;
    const char* mInfo;
  };

  class Comparison {
  public:
    Comparison(ComparisonFunc collider, const char* type1, const char* type2)
    : mCollider(collider), mType1(type1), mType2(type2) {}

    ComparisonFunc GetCollider() const { return mCollider; }
    const char* GetType1() const { return mType1; }
    const char* GetType2() const { return mType2; }

  private:
    ComparisonFunc mCollider;
    const char* mType1;
    const char* mType2;
  };

  class BooleanComparison {
  public:
    BooleanComparison(BooleanComparisonFunc collider, const char* type1, const char* type2)
    : mCollider(collider), mType1(type1), mType2(type2) {}

    BooleanComparisonFunc GetCollider() const { return mCollider; }
    const char* GetType1() const { return mType1; }
    const char* GetType2() const { return mType2; }

  private:
    BooleanComparisonFunc mCollider;
    const char* mType1;
    const char* mType2;
  };

  class MovingComparison {
  public:
    MovingComparison(MovingComparisonFunc collider, const char* type1, const char* type2)
    : mCollider(collider), mType1(type1), mType2(type2) {}

    MovingComparisonFunc GetCollider() const { return mCollider; }
    const char* GetType1() const { return mType1; }
    const char* GetType2() const { return mType2; }

  private:
    MovingComparisonFunc mCollider;
    const char* mType1;
    const char* mType2;
  };

  CCollisionPrimitive(const CMaterialList& list);
  CCollisionPrimitive(const CCollisionPrimitive& other) : mMaterial(other.mMaterial) {}

  virtual uint GetTableIndex() const = 0;
  virtual void SetMaterial(const CMaterialList& other) { mMaterial = other; }
  virtual const CMaterialList& GetMaterial() const { return mMaterial; }
  virtual CAABox CalculateAABox(const CTransform4f&) const = 0;
  virtual CAABox CalculateLocalAABox() const = 0;
  virtual FourCC GetPrimType() const = 0;
  virtual ~CCollisionPrimitive();
  virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const = 0;
  CRayCastResult CastRay(const CVector3f& pos, const CVector3f& dir, float length,
                         const CMaterialFilter& filter, const CTransform4f& xf) const {
    return CastRayInternal(CInternalRayCastStructure(pos, dir, length, xf, filter));
  }

  static void InitBeginTypes();
  static void InitAddType(const Type& type);
  static void InitEndTypes();
  static void InitBeginColliders();
  static void InitAddCollider(const Comparison& comp);
  static void InitAddBooleanCollider(const BooleanComparison& comp);
  static void InitAddMovingCollider(const MovingComparison& comp);
  static void InitAddCollider(ComparisonFunc comp, const char*, const char*);
  static void InitAddBooleanCollider(BooleanComparisonFunc comp, const char*, const char*);
  static void InitAddMovingCollider(MovingComparisonFunc comp, const char*, const char*);
  static void InitEndColliders();
  static void Uninitialize();

  static bool Collide(const CInternalCollisionStructure::CPrimDesc& prim0,
                      const CInternalCollisionStructure::CPrimDesc& prim1,
                      CCollisionInfoList& list);
  static bool CollideBoolean(const CInternalCollisionStructure::CPrimDesc& prim0,
                             const CInternalCollisionStructure::CPrimDesc& prim1);
  static bool CollideMoving(const CInternalCollisionStructure::CPrimDesc& prim0,
                            const CInternalCollisionStructure::CPrimDesc& prim1,
                            const CVector3f& dir, double& dOut, CCollisionInfo& infoOut);

  static int TypeIndexFromTypeInfo(const char* name) {
    rstl::vector< Type >::const_iterator iter = sCollisionTypeList->begin();

    for (int i = 0; i < sCollisionTypeList->size(); ++i) {
      if (strcmp(sCollisionTypeList->at(i).GetInfo(), name) == 0) {
        return i;
      }
    }

    return -1;
  }

  static ComparisonFunc* ColliderFromTable(const int index1, const int index2);
  static BooleanComparisonFunc* BooleanColliderFromTable(const int index1, const int index2);
  static MovingComparisonFunc* MovingColliderFromTable(const int index1, const int index2);

private:
  static int sNumTypes;
  static bool sInitComplete;
  static bool sTypesAdded;
  static bool sTypesAdding;
  static bool sCollidersAdded;
  static bool sCollidersAdding;
  static rstl::single_ptr< rstl::vector< Type > > sCollisionTypeList;
  static rstl::single_ptr< ComparisonFunc > sTableOfCollidables;
  static rstl::single_ptr< BooleanComparisonFunc > sTableOfBooleanCollidables;
  static rstl::single_ptr< MovingComparisonFunc > sTableOfMovingCollidables;

  static bool InternalCollide(const CInternalCollisionStructure&, CCollisionInfoList&);
  static bool InternalCollideMoving(const CInternalCollisionStructure&, const CVector3f&, double&,
                                    CCollisionInfo&);
  static bool InternalCollideBoolean(const CInternalCollisionStructure&);

  CMaterialList mMaterial;
};
CHECK_SIZEOF(CCollisionPrimitive, 0x10)

inline CCollisionPrimitive::~CCollisionPrimitive() {}

#endif // _CCOLLISIONPRIMITIVE
