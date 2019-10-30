#include "CCollidableSphere.hpp"
#include "CCollisionInfoList.hpp"
#include "CCollidableAABox.hpp"
#include "CollisionUtil.hpp"
#include "CInternalRayCastStructure.hpp"

namespace urde {
constexpr CCollisionPrimitive::Type sType(CCollidableSphere::SetStaticTableIndex, "CCollidableSphere");
u32 CCollidableSphere::sTableIndex = -1;

namespace Collide {

bool Sphere_AABox(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableAABox& p1 = static_cast<const CCollidableAABox&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::CAABox b1 = p1.Transform(collision.GetRight().GetTransform());

  float distSq = 0.f;
  int flags = 0;
  for (int i = 0; i < 3; ++i) {
    if (s0.position[i] < b1.min[i]) {
      if (s0.position[i] + s0.radius >= b1.min[i]) {
        float dist = s0.position[i] - b1.min[i];
        distSq += dist * dist;
        flags |= 1 << (2 * i);
      } else {
        return false;
      }
    } else if (s0.position[i] > b1.max[i]) {
      if (s0.position[i] - s0.radius <= b1.max[i]) {
        float dist = s0.position[i] - b1.max[i];
        distSq += dist * dist;
        flags |= 1 << (2 * i + 1);
      } else {
        return false;
      }
    }
  }

  if (!flags) {
    zeus::CVector3f normal = (s0.position - b1.center()).normalized();
    zeus::CVector3f point = s0.position + normal * s0.radius;
    CCollisionInfo info(point, p0.GetMaterial(), p1.GetMaterial(), normal);
    list.Add(info, false);
    return true;
  }

  if (distSq > s0.radius * s0.radius)
    return false;

  zeus::CVector3f point;
  switch (flags) {
  case 0x1a:
    point = zeus::CVector3f(b1.max.x(), b1.max.y(), b1.min.z());
    break;
  case 0x19:
    point = zeus::CVector3f(b1.min.x(), b1.max.y(), b1.min.z());
    break;
  case 0x16:
    point = zeus::CVector3f(b1.max.x(), b1.min.y(), b1.min.z());
    break;
  case 0x15:
    point = zeus::CVector3f(b1.min.x(), b1.min.y(), b1.min.z());
    break;
  case 0x2a:
    point = zeus::CVector3f(b1.max.x(), b1.max.y(), b1.max.z());
    break;
  case 0x29:
    point = zeus::CVector3f(b1.min.x(), b1.max.y(), b1.max.z());
    break;
  case 0x26:
    point = zeus::CVector3f(b1.max.x(), b1.min.y(), b1.max.z());
    break;
  case 0x25:
    point = zeus::CVector3f(b1.min.x(), b1.min.y(), b1.max.z());
    break;
  case 0x11:
    point = zeus::CVector3f(b1.min.x(), s0.position.y(), b1.min.z());
    break;
  case 0x12:
    point = zeus::CVector3f(b1.max.x(), s0.position.y(), b1.min.z());
    break;
  case 0x14:
    point = zeus::CVector3f(s0.position.x(), b1.min.y(), b1.min.z());
    break;
  case 0x18:
    point = zeus::CVector3f(s0.position.x(), b1.max.y(), b1.min.z());
    break;
  case 0x5:
    point = zeus::CVector3f(b1.min.x(), b1.min.y(), s0.position.z());
    break;
  case 0x6:
    point = zeus::CVector3f(b1.max.x(), b1.min.y(), s0.position.z());
    break;
  case 0x9:
    point = zeus::CVector3f(b1.min.x(), b1.max.y(), s0.position.z());
    break;
  case 0xa:
    point = zeus::CVector3f(b1.max.x(), b1.max.y(), s0.position.z());
    break;
  case 0x21:
    point = zeus::CVector3f(b1.min.x(), s0.position.y(), b1.max.z());
    break;
  case 0x22:
    point = zeus::CVector3f(b1.max.x(), s0.position.y(), b1.max.z());
    break;
  case 0x24:
    point = zeus::CVector3f(s0.position.x(), b1.min.y(), b1.max.z());
    break;
  case 0x28:
    point = zeus::CVector3f(s0.position.x(), b1.max.y(), b1.max.z());
    break;
  case 0x1:
    point = zeus::CVector3f(b1.min.x(), s0.position.y(), s0.position.z());
    break;
  case 0x2:
    point = zeus::CVector3f(b1.max.x(), s0.position.y(), s0.position.z());
    break;
  case 0x4:
    point = zeus::CVector3f(s0.position.x(), b1.min.y(), s0.position.z());
    break;
  case 0x8:
    point = zeus::CVector3f(s0.position.x(), b1.max.y(), s0.position.z());
    break;
  case 0x10:
    point = zeus::CVector3f(s0.position.x(), s0.position.y(), b1.min.z());
    break;
  case 0x20:
    point = zeus::CVector3f(s0.position.x(), s0.position.y(), b1.max.z());
    break;
  default:
    break;
  }

  CCollisionInfo info(point, p0.GetMaterial(), p1.GetMaterial(), (s0.position - point).normalized());
  list.Add(info, false);
  return true;
}

bool Sphere_AABox_Bool(const CInternalCollisionStructure& collision) {
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableAABox& p1 = static_cast<const CCollidableAABox&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::CAABox b1 = p1.Transform(collision.GetRight().GetTransform());

  return CCollidableSphere::Sphere_AABox_Bool(s0, b1);
}

bool Sphere_Sphere(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableSphere& p1 = static_cast<const CCollidableSphere&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::CSphere s1 = p1.Transform(collision.GetRight().GetTransform());

  float radiusSum = s0.radius + s1.radius;
  zeus::CVector3f delta = s0.position - s1.position;
  float deltaMagSq = delta.magSquared();
  if (deltaMagSq <= radiusSum * radiusSum) {
    zeus::CVector3f deltaNorm =
        delta.canBeNormalized() ? (1.f / std::sqrt(deltaMagSq)) * delta : zeus::skRight;
    zeus::CVector3f collisionPoint = deltaNorm * s1.radius + s1.position;
    CCollisionInfo info(collisionPoint, p0.GetMaterial(), p1.GetMaterial(), deltaNorm);
    list.Add(info, false);

    return true;
  }

  return false;
}

bool Sphere_Sphere_Bool(const CInternalCollisionStructure& collision) {
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableSphere& p1 = static_cast<const CCollidableSphere&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::CSphere s1 = p1.Transform(collision.GetRight().GetTransform());

  float radiusSum = s0.radius + s1.radius;
  return (s0.position - s1.position).magSquared() <= radiusSum * radiusSum;
}

} // namespace Collide

CCollidableSphere::CCollidableSphere(const zeus::CSphere& sphere, const CMaterialList& list)
: CCollisionPrimitive(list), x10_sphere(sphere) {}

zeus::CSphere CCollidableSphere::Transform(const zeus::CTransform& xf) const {
  return zeus::CSphere(xf * x10_sphere.position, x10_sphere.radius);
}

u32 CCollidableSphere::GetTableIndex() const { return sTableIndex; }

zeus::CAABox CCollidableSphere::CalculateAABox(const zeus::CTransform& xf) const {
  zeus::CVector3f xfPos = xf * x10_sphere.position;
  return {xfPos - x10_sphere.radius, xfPos + x10_sphere.radius};
}

zeus::CAABox CCollidableSphere::CalculateLocalAABox() const {
  return {x10_sphere.position - x10_sphere.radius, x10_sphere.position + x10_sphere.radius};
}

FourCC CCollidableSphere::GetPrimType() const { return SBIG('SPHR'); }

CRayCastResult CCollidableSphere::CastRayInternal(const CInternalRayCastStructure& rayCast) const {
  if (!rayCast.GetFilter().Passes(GetMaterial()))
    return {};

  zeus::CSphere xfSphere = Transform(rayCast.GetTransform());
  float t = 0.f;
  zeus::CVector3f point;
  if (CollisionUtil::RaySphereIntersection(xfSphere, rayCast.GetRay().start, rayCast.GetRay().dir, rayCast.GetMaxTime(),
                                           t, point)) {
    zeus::CVector3f delta = point - xfSphere.position;
    float deltaMag = delta.magnitude();
    zeus::CUnitVector3f planeNormal = (deltaMag > 0.01f) ? delta * (1.f / deltaMag) : rayCast.GetRay().dir;
    float planeD = point.dot(planeNormal);
    return CRayCastResult(t, point, zeus::CPlane(planeNormal, planeD), GetMaterial());
  }

  return {};
}

const CCollisionPrimitive::Type& CCollidableSphere::GetType() { return sType; }

bool CCollidableSphere::CollideMovingAABox(const CInternalCollisionStructure& collision, const zeus::CVector3f& dir,
                                           double& dOut, CCollisionInfo& infoOut) {
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableAABox& p1 = static_cast<const CCollidableAABox&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::CAABox b1 = p1.CalculateAABox(collision.GetRight().GetTransform());

  double d = dOut;
  zeus::CVector3f point, normal;
  if (CollisionUtil::MovingSphereAABox(s0, b1, dir, d, point, normal) && d < dOut) {
    dOut = d;
    infoOut = CCollisionInfo(point, p0.GetMaterial(), p1.GetMaterial(), normal);
    return true;
  }

  return false;
}

bool CCollidableSphere::CollideMovingSphere(const CInternalCollisionStructure& collision, const zeus::CVector3f& dir,
                                            double& dOut, CCollisionInfo& infoOut) {
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableSphere& p1 = static_cast<const CCollidableSphere&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::CSphere s1 = p1.Transform(collision.GetRight().GetTransform());

  double d = dOut;
  if (CollisionUtil::RaySphereIntersection_Double(zeus::CSphere(s1.position, s0.radius + s1.radius), s0.position, dir,
                                                  d) &&
      d >= 0.0 && d < dOut) {
    dOut = d;
    zeus::CVector3f normal = (s0.position + float(d) * dir - s1.position).normalized();
    infoOut = CCollisionInfo(s1.position + s1.radius * normal, p0.GetMaterial(), p1.GetMaterial(), normal);
    return true;
  }

  return false;
}

bool CCollidableSphere::Sphere_AABox_Bool(const zeus::CSphere& sphere, const zeus::CAABox& aabb) {
  float mag = 0.f;

  for (int i = 0; i < 3; ++i) {
    if (sphere.position[i] < aabb.min[i]) {
      float tmp = sphere.position[i] - aabb.min[i];
      mag += tmp * tmp;
    } else if (sphere.position[i] > aabb.max[i]) {
      float tmp = sphere.position[i] - aabb.max[i];
      mag += tmp * tmp;
    }
  }

  return mag <= sphere.radius * sphere.radius;
}
} // namespace urde
