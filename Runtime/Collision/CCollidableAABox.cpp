#include "CCollidableAABox.hpp"
#include "CollisionUtil.hpp"
#include "CCollidableSphere.hpp"
#include "CCollisionInfo.hpp"
#include "CInternalRayCastStructure.hpp"

namespace urde {
const CCollisionPrimitive::Type CCollidableAABox::sType(CCollidableAABox::SetStaticTableIndex, "CCollidableAABox");
u32 CCollidableAABox::sTableIndex = -1;

CCollidableAABox::CCollidableAABox() = default;

CCollidableAABox::CCollidableAABox(const zeus::CAABox& aabox, const CMaterialList& list)
: CCollisionPrimitive(list), x10_aabox(aabox) {}

zeus::CAABox CCollidableAABox::Transform(const zeus::CTransform& xf) const {
  return {xf.origin + x10_aabox.min, xf.origin + x10_aabox.max};
}

u32 CCollidableAABox::GetTableIndex() const { return sTableIndex; }

zeus::CAABox CCollidableAABox::CalculateAABox(const zeus::CTransform& xf) const { return Transform(xf); }

zeus::CAABox CCollidableAABox::CalculateLocalAABox() const { return x10_aabox; }

FourCC CCollidableAABox::GetPrimType() const { return SBIG('AABX'); }

CRayCastResult CCollidableAABox::CastRayInternal(const CInternalRayCastStructure& rayCast) const {
  if (!rayCast.GetFilter().Passes(GetMaterial()))
    return {};
  zeus::CTransform rayCastXfInv = rayCast.GetTransform().inverse();
  zeus::CVector3f localRayStart = rayCastXfInv * rayCast.GetRay().start;
  zeus::CVector3f localRayDir = rayCastXfInv.rotate(rayCast.GetRay().dir);
  float tMin, tMax;
  int axis;
  bool sign;
  if (!CollisionUtil::BoxLineTest(x10_aabox, localRayStart, localRayDir, tMin, tMax, axis, sign) || tMin < 0.f ||
      (rayCast.GetMaxTime() > 0.f && tMin > rayCast.GetMaxTime()))
    return {};

  zeus::CVector3f planeNormal;
  planeNormal[axis] = sign ? 1.f : -1.f;
  float planeD = axis ? x10_aabox.min[axis] : -x10_aabox.max[axis];
  CRayCastResult result(tMin, localRayStart + tMin * localRayDir, zeus::CPlane(planeNormal, planeD), GetMaterial());
  result.Transform(rayCast.GetTransform());
  return result;
}

const CCollisionPrimitive::Type& CCollidableAABox::GetType() { return sType; }

void CCollidableAABox::SetStaticTableIndex(u32 index) { sTableIndex = index; }

bool CCollidableAABox::CollideMovingAABox(const CInternalCollisionStructure& collision, const zeus::CVector3f& dir,
                                          double& dOut, CCollisionInfo& infoOut) {
  const CCollidableAABox& p0 = static_cast<const CCollidableAABox&>(collision.GetLeft().GetPrim());
  const CCollidableAABox& p1 = static_cast<const CCollidableAABox&>(collision.GetRight().GetPrim());

  zeus::CAABox b0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::CAABox b1 = p1.Transform(collision.GetRight().GetTransform());

  double d;
  zeus::CVector3f point, normal;
  if (CollisionUtil::AABox_AABox_Moving(b0, b1, dir, d, point, normal) && d > 0.0 && d < dOut) {
    dOut = d;
    infoOut = CCollisionInfo(point, p0.GetMaterial(), p1.GetMaterial(), normal, -normal);
    return true;
  }

  return false;
}

bool CCollidableAABox::CollideMovingSphere(const CInternalCollisionStructure& collision, const zeus::CVector3f& dir,
                                           double& dOut, CCollisionInfo& infoOut) {
  const CCollidableAABox& p0 = static_cast<const CCollidableAABox&>(collision.GetLeft().GetPrim());
  const CCollidableSphere& p1 = static_cast<const CCollidableSphere&>(collision.GetRight().GetPrim());

  zeus::CAABox b0 = p0.CalculateAABox(collision.GetLeft().GetTransform());
  zeus::CSphere s1 = p1.Transform(collision.GetRight().GetTransform());

  double d = dOut;
  zeus::CVector3f point, normal;
  if (CollisionUtil::MovingSphereAABox(s1, b0, -dir, d, point, normal) && d < dOut) {
    point = s1.position - s1.radius * normal;
    dOut = d;
    infoOut = CCollisionInfo(point, p0.GetMaterial(), p1.GetMaterial(), -normal);
    return true;
  }

  return false;
}

namespace Collide {

bool AABox_AABox(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  const CCollidableAABox& box0 = static_cast<const CCollidableAABox&>(collision.GetLeft().GetPrim());
  const CCollidableAABox& box1 = static_cast<const CCollidableAABox&>(collision.GetRight().GetPrim());

  zeus::CAABox aabb0 = box0.Transform(collision.GetLeft().GetTransform());
  zeus::CAABox aabb1 = box1.Transform(collision.GetRight().GetTransform());

  return CollisionUtil::AABoxAABoxIntersection(aabb0, box0.GetMaterial(), aabb1, box1.GetMaterial(), list);
}

bool AABox_AABox_Bool(const CInternalCollisionStructure& collision) {
  const CCollidableAABox& box0 = static_cast<const CCollidableAABox&>(collision.GetLeft().GetPrim());
  const CCollidableAABox& box1 = static_cast<const CCollidableAABox&>(collision.GetRight().GetPrim());

  zeus::CAABox aabb0 = box0.Transform(collision.GetLeft().GetTransform());
  zeus::CAABox aabb1 = box1.Transform(collision.GetRight().GetTransform());

  return CollisionUtil::AABoxAABoxIntersection(aabb0, aabb1);
}

} // namespace Collide
} // namespace urde
