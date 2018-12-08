#pragma once

#include "RetroTypes.hpp"
#include "CMaterialList.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CMatrix3f.hpp"

namespace urde {
class CCollisionInfo {
  zeus::CVector3f x0_point;
  zeus::CVector3f xc_extentX;
  zeus::CVector3f x18_extentY;
  zeus::CVector3f x24_extentZ;
  bool x30_valid = false;
  bool x31_hasExtents = false;
  CMaterialList x38_materialLeft;
  CMaterialList x40_materialRight;
  zeus::CVector3f x48_normalLeft;
  zeus::CVector3f x54_normalRight;

public:
  CCollisionInfo() = default;
  CCollisionInfo(const zeus::CVector3f& point, const CMaterialList& list1, const CMaterialList& list2,
                 const zeus::CVector3f& normalLeft, const zeus::CVector3f& normalRight)
  : x0_point(point)
  , x30_valid(true)
  , x31_hasExtents(false)
  , x38_materialLeft(list2)
  , x40_materialRight(list1)
  , x48_normalLeft(normalLeft)
  , x54_normalRight(normalRight) {}
  CCollisionInfo(const zeus::CVector3f& point, const CMaterialList& list1, const CMaterialList& list2,
                 const zeus::CVector3f& normal)
  : x0_point(point)
  , x30_valid(true)
  , x31_hasExtents(false)
  , x38_materialLeft(list2)
  , x40_materialRight(list1)
  , x48_normalLeft(normal)
  , x54_normalRight(-normal) {}
  CCollisionInfo(const zeus::CAABox& aabox, const CMaterialList& list1, const CMaterialList& list2,
                 const zeus::CVector3f& normalLeft, const zeus::CVector3f& normalRight)
  : x0_point(aabox.min)
  , xc_extentX(aabox.max.x() - aabox.min.x(), 0.f, 0.f)
  , x18_extentY(0.f, aabox.max.y() - aabox.min.y(), 0.f)
  , x24_extentZ(0.f, 0.f, aabox.max.z() - aabox.min.z())
  , x30_valid(true)
  , x31_hasExtents(true)
  , x38_materialLeft(list2)
  , x40_materialRight(list1)
  , x48_normalLeft(normalLeft)
  , x54_normalRight(normalRight) {}

  CCollisionInfo GetSwapped() const;
  bool IsValid() const { return x30_valid; }
  const CMaterialList& GetMaterialLeft() const { return x38_materialLeft; }
  const CMaterialList& GetMaterialRight() const { return x40_materialRight; }
  zeus::CVector3f GetExtreme() const;
  void Swap();
  const zeus::CVector3f& GetNormalLeft() const { return x48_normalLeft; }
  const zeus::CVector3f& GetNormalRight() const { return x54_normalRight; }
  const zeus::CVector3f& GetPoint() const { return x0_point; }
};

} // namespace urde
