#ifndef _CCOLLISIONINFO
#define _CCOLLISIONINFO

#include "types.h"

#include "Collision/CMaterialList.hpp"

#include "Kyoto/Math/CVector3f.hpp"

class CAABox;

class CCollisionInfo {
public:
  enum EInvalid {
    kI_Invalid,
    kI_Valid,
  };
  enum ESwapMaterials { kSM_Swap };

  CCollisionInfo(EInvalid valid = kI_Invalid);
  CCollisionInfo(const CVector3f& point, const CMaterialList& leftMat,
                 const CMaterialList& rightMat, const CVector3f& normal);
  CCollisionInfo(const CVector3f& point, const CMaterialList& leftMat,
                 const CMaterialList& rightMat, const CVector3f& leftNormal,
                 const CVector3f& rightNormal);
  CCollisionInfo(const CAABox& aabox, const CMaterialList& leftMat, const CMaterialList& rightMat,
                 const CVector3f& leftNormal, const CVector3f& rightNormal);
  CCollisionInfo(const CCollisionInfo& other, ESwapMaterials)
  : mPoint(other.mPoint)
  , mExtentX(other.mExtentX)
  , mExtentY(other.mExtentY)
  , mExtentZ(other.mExtentZ)
  , mValid(other.mValid)
  , mHasExtents(other.mHasExtents)
  , mMaterialLeft(other.mMaterialRight)
  , mMaterialRight(other.mMaterialLeft)
  , mNormalLeft(other.mNormalRight)
  , mNormalRight(other.mNormalLeft) {}

  CCollisionInfo GetSwapped(ESwapMaterials) const;
  bool IsValid() const { return mValid; }
  const CMaterialList& GetMaterialLeft() const { return mMaterialLeft; }
  const CMaterialList& GetMaterialRight() const { return mMaterialRight; }
  CVector3f GetExtreme() const;
  void Swap();
  const CVector3f& GetNormalLeft() const { return mNormalLeft; }
  const CVector3f& GetNormalRight() const { return mNormalRight; }
  const CVector3f& GetPoint() const { return mPoint; }

  // TODO: hack for CollisionUtil::AccumulateCollisionInfo. Maybe it's a static function here?
  CMaterialList& MaterialLeft() { return mMaterialLeft; }
  CMaterialList& MaterialRight() { return mMaterialRight; }
  CVector3f& NormalLeft() { return mNormalLeft; }
  CVector3f& NormalRight() { return mNormalRight; }
  CVector3f& Point() { return mPoint; }

private:
  CVector3f mPoint;
  CVector3f mExtentX;
  CVector3f mExtentY;
  CVector3f mExtentZ;
  bool mValid;
  bool mHasExtents;
  CMaterialList mMaterialLeft;
  CMaterialList mMaterialRight;
  CVector3f mNormalLeft;
  CVector3f mNormalRight;
};
CHECK_SIZEOF(CCollisionInfo, 0x60)

#endif // _CCOLLISIONINFO
