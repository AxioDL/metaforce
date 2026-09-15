#include "Collision/CCollidableAABox.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CInternalCollisionStructure.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Collision/NormalTable.hpp"

uint CCollidableAABox::sTableIndex = -1;

CCollisionPrimitive::Type CCollidableAABox::GetType() {
  return Type(SetStaticTableIndex, "CCollidableAABox");
}

namespace Collide {
bool AABox_AABox_Bool(const CInternalCollisionStructure& collision) {
  const CCollidableAABox& box0 =
      static_cast< const CCollidableAABox& >(collision.GetLeft().GetPrim());
  const CCollidableAABox& box1 =
      static_cast< const CCollidableAABox& >(collision.GetRight().GetPrim());
  const CAABox aabb0 = box0.Transform(collision.GetLeft().GetTransform());
  const CAABox aabb1 = box1.Transform(collision.GetRight().GetTransform());

  return CollisionUtil::AABoxAABoxIntersection(aabb0, aabb1);
}
bool AABox_AABox(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  const CCollidableAABox& box0 =
      static_cast< const CCollidableAABox& >(collision.GetLeft().GetPrim());
  const CCollidableAABox& box1 =
      static_cast< const CCollidableAABox& >(collision.GetRight().GetPrim());

  CAABox aabb0 = box0.Transform(collision.GetLeft().GetTransform());
  CAABox aabb1 = box1.Transform(collision.GetRight().GetTransform());

  return CollisionUtil::AABoxAABoxIntersection(aabb0, box0.GetMaterial(), aabb1, box1.GetMaterial(),
                                               list);
}
} // namespace Collide

bool CCollidableAABox::CollideMovingAABox(const CInternalCollisionStructure& collision,
                                          const CVector3f& dir, double& dOut,
                                          CCollisionInfo& infoOut) {
  const CCollidableAABox& left =
      static_cast< const CCollidableAABox& >(collision.GetLeft().GetPrim());
  const CCollidableAABox& right =
      static_cast< const CCollidableAABox& >(collision.GetRight().GetPrim());
  const CAABox leftAABox = left.Transform(collision.GetLeft().GetTransform());
  const CAABox rightAABox = right.Transform(collision.GetRight().GetTransform());
  CVector3f point = CVector3f::Zero();
  CVector3f normal = CVector3f::Zero();
  double tmpD;
  if (CollisionUtil::AABox_ABBox_Moving(leftAABox, rightAABox, dir, tmpD, point, normal) &&
      tmpD > 0 && tmpD < dOut) {
    dOut = tmpD;
    infoOut = CCollisionInfo(point, left.GetMaterial(), right.GetMaterial(), normal, -normal);
    return true;
  }

  return false;
}

bool CCollidableAABox::CollideMovingSphere(const CInternalCollisionStructure& collision,
                                           const CVector3f& dir, double& dOut,
                                           CCollisionInfo& infoOut) {
  const CCollidableAABox& left =
      static_cast< const CCollidableAABox& >(collision.GetLeft().GetPrim());
  const CCollidableSphere& right =
      static_cast< const CCollidableSphere& >(collision.GetRight().GetPrim());
  const CAABox leftAABox(left.CalculateAABox(collision.GetLeft().GetTransform()));
  const CSphere rightSphere(right.Transform(collision.GetRight().GetTransform()));
  double tmpD = dOut;
  CVector3f point(CVector3f::Zero());
  CVector3f normal(CVector3f::Zero());

  if (CollisionUtil::MovingSphereAABox(rightSphere, leftAABox, -dir, tmpD, point, normal) &&
      tmpD < dOut) {
    point = (rightSphere.GetCenter() - rightSphere.GetRadius() * normal);
    dOut = tmpD;
    infoOut = CCollisionInfo(point, left.GetMaterial(), right.GetMaterial(), -normal);
    return true;
  }

  return false;
}

CRayCastResult CCollidableAABox::CastRayInternal(const CInternalRayCastStructure& rayCast) const {
  if (!rayCast.GetFilter().Passes(GetMaterial())) {
    return CRayCastResult::MakeInvalid();
  }

  const CTransform4f rayCastXf = rayCast.GetTransform();
  const CTransform4f rayCastXfInv = rayCast.GetTransform().GetQuickInverse();
  const CVector3f localRayStart = rayCastXfInv * rayCast.GetStart();
  const CVector3f localRayDir = rayCastXfInv.Rotate(rayCast.GetRay().GetDirection());

  const float rayMaxTime = rayCast.GetMaxTime();
  float tMin;
  float tMax;
  bool sign;
  int axis;
  if (!CollisionUtil::BoxLineTest(x10_aabb, localRayStart, localRayDir, tMin, tMax, axis, sign) ||
      tMin < 0.f || (rayMaxTime > 0.f && tMin > rayMaxTime)) {
    return CRayCastResult::MakeInvalid();
  }
  const float signValue = sign ? 1.f : -1.f;
  CVector3f planeNormal = CVector3f::Zero();
  planeNormal[axis] = signValue;
  const CUnitVector3f vec(planeNormal.GetX(), planeNormal.GetY(), planeNormal.GetZ());
  const float planeD = axis != 0 ? GetBox().GetMinPoint()[axis] : -GetBox().GetMaxPoint()[axis];
  const CPlane plane(planeD, vec);
  CRayCastResult result(tMin, localRayStart + tMin * localRayDir, plane, GetMaterial());
  result.Transform(rayCastXf);
  return result;
}

CAABox CCollidableAABox::CalculateLocalAABox() const { return x10_aabb; }

CAABox CCollidableAABox::CalculateAABox(const CTransform4f& xf) const { return Transform(xf); }

CAABox CCollidableAABox::Transform(const CTransform4f& xf) const {
  const CVector3f xfPos = xf.GetTranslation();
  return CAABox(x10_aabb.GetMinPoint() + xfPos, x10_aabb.GetMaxPoint() + xfPos);
}

FourCC CCollidableAABox::GetPrimType() const { return 'AABX'; }
uint CCollidableAABox::GetTableIndex() const { return sTableIndex; }
