#ifndef _CJOINTCOLLISIONDESCRIPTION
#define _CJOINTCOLLISIONDESCRIPTION

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/string.hpp"

struct SJointInfo {
  const char* from;
  const char* to;
  float radius;
  float separation;
};

struct SSphereJointInfo {
  const char* name;
  float radius;
};

class CJointCollisionDescription {
public:
  enum ECollisionType {
    kCT_Sphere,
    kCT_SphereSubdivide,
    kCT_AABox,
    kCT_OBBAutoSize,
    kCT_OBB,
  };

  enum EOrientationType {
    kOT_Zero,
    kOT_One,
  };

  CJointCollisionDescription(ECollisionType colType, CSegId pivotId, CSegId nextId,
                             const CVector3f& bounds, const CVector3f& pivotPoint, float radius,
                             float maxSeparation, EOrientationType orientType,
                             const rstl::string& name, float mass)
  : mColType(colType)
  , mOrientType(orientType)
  , mPivotId(pivotId)
  , mNextId(nextId)
  , mBounds(bounds)
  , mPivotPoint(pivotPoint)
  , mRadius(radius)
  , mMaxSeparation(maxSeparation)
  , mName(name)
  , mActorId(kInvalidUniqueId)
  , mMass(mass) {}

  void ScaleAllBounds(const CVector3f& scale);

  ECollisionType GetType() const { return mColType; }
  EOrientationType GetOrientationType() const { return mOrientType; }
  CSegId GetPivotId() const { return mPivotId; }
  CSegId GetNextId() const { return mNextId; }
  const CVector3f& GetBounds() const { return mBounds; }
  const CVector3f& GetPivotPoint() const { return mPivotPoint; }
  float GetRadius() const { return mRadius; }
  float GetMaxSeparation() const { return mMaxSeparation; }
  float GetMass() const { return mMass; }
  void SetCollisionActorId(TUniqueId uid) { mActorId = uid; }
  const rstl::string& GetName() const { return mName; }
  TUniqueId GetCollisionActorId() const { return mActorId; }
  static CJointCollisionDescription SphereSubdivideCollision(CSegId pivotId, CSegId nextId,
                                                             float radius, float maxSeparation,
                                                             EOrientationType orientType,
                                                             const rstl::string& name, float mass);
  static CJointCollisionDescription SphereCollision(CSegId pivotId, float radius,
                                                    const rstl::string& name, float mass);
  static CJointCollisionDescription AABoxCollision(CSegId pivotId, const CVector3f& bounds,
                                                   const rstl::string& name, float mass);
  static CJointCollisionDescription OBBAutoSizeCollision(CSegId pivotId, CSegId nextId,
                                                         const CVector3f& bounds,
                                                         EOrientationType orientType,
                                                         const rstl::string&, float mass);
  static CJointCollisionDescription OBBCollision(CSegId pivotId, const CVector3f& bounds,
                                                 const CVector3f& pivotPoint,
                                                 const rstl::string& name, float mass);

private:
  ECollisionType mColType;
  EOrientationType mOrientType;
  CSegId mPivotId;
  CSegId mNextId;
  CVector3f mBounds;
  CVector3f mPivotPoint;
  float mRadius;
  float mMaxSeparation;
  rstl::string mName;
  TUniqueId mActorId;
  float mMass;
};

CHECK_SIZEOF(CJointCollisionDescription, 0x44)

#endif // _CJOINTCOLLISIONDESCRIPTION
