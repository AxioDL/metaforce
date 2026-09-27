#include "Collision/CCollisionInfo.hpp"

#include "Kyoto/Math/CAABox.hpp"

#include "rstl/algorithm.hpp"

CCollisionInfo::CCollisionInfo(const CVector3f& point, const CMaterialList& rightMat,
                               const CMaterialList& leftMat, const CVector3f& normal)
: mPoint(point)
, mExtentX(CVector3f::Zero())
, mExtentY(CVector3f::Zero())
, mExtentZ(CVector3f::Zero())
, mValid(true)
, mHasExtents(false)
, mMaterialLeft(leftMat)
, mMaterialRight(rightMat)
, mNormalLeft(normal)
, mNormalRight(-normal) {}

CCollisionInfo::CCollisionInfo(const CVector3f& point, const CMaterialList& rightMat,
                               const CMaterialList& leftMat, const CVector3f& leftNormal,
                               const CVector3f& rightNormal)
: mPoint(point)
, mExtentX(CVector3f::Zero())
, mExtentY(CVector3f::Zero())
, mExtentZ(CVector3f::Zero())
, mValid(true)
, mHasExtents(false)
, mMaterialLeft(leftMat)
, mMaterialRight(rightMat)
, mNormalLeft(leftNormal)
, mNormalRight(rightNormal) {}

CCollisionInfo::CCollisionInfo(const CAABox& aabox, const CMaterialList& rightMat,
                               const CMaterialList& leftMat, const CVector3f& leftNormal,
                               const CVector3f& rightNormal)
: mPoint(aabox.GetMinPoint())
, mExtentX(aabox.GetMaxPoint().GetX() - aabox.GetMinPoint().GetX(), 0.f, 0.f)
, mExtentY(0.f, aabox.GetMaxPoint().GetY() - aabox.GetMinPoint().GetY(), 0.f)
, mExtentZ(0.f, 0.f, aabox.GetMaxPoint().GetZ() - aabox.GetMinPoint().GetZ())
, mValid(true)
, mHasExtents(true)
, mMaterialLeft(leftMat)
, mMaterialRight(rightMat)
, mNormalLeft(leftNormal)
, mNormalRight(rightNormal) {}

CCollisionInfo CCollisionInfo::GetSwapped(ESwapMaterials swap) const { return CCollisionInfo(*this, swap); }

CCollisionInfo::CCollisionInfo(EInvalid invalid)
: mPoint(0.f, 0.f, 0.f)
, mExtentX(0.f, 0.f, 0.f)
, mExtentY(0.f, 0.f, 0.f)
, mExtentZ(0.f, 0.f, 0.f)
, mValid(false)
, mHasExtents(false)
, mMaterialLeft(CMaterialList())
, mMaterialRight(CMaterialList())
, mNormalLeft(0.f, 0.f, 0.f)
, mNormalRight(0.f, 0.f, 0.f) {}

CVector3f CCollisionInfo::GetExtreme() const {
  return mPoint + mExtentX + mExtentY + mExtentZ;
}

void CCollisionInfo::Swap() {
  mNormalLeft = -mNormalLeft;
  mNormalRight = -mNormalRight;
  rstl::swap(mMaterialLeft, mMaterialRight);
}
