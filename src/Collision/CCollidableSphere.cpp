#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include <Collision/CCollidableSphere.hpp>

#include <Collision/CCollidableAABox.hpp>
#include <Collision/CInternalCollisionStructure.hpp>
#include <Collision/NormalTable.hpp>

#include <Kyoto/Math/CAABox.hpp>

uint CCollidableSphere::sTableIndex = -1;

void CCollidableSphere::SetStaticTableIndex(uint idx) { sTableIndex = idx; }
CCollisionPrimitive::Type CCollidableSphere::GetType() {
  return Type(SetStaticTableIndex, "CCollidableSphere");
}

bool CCollidableSphere::Sphere_AABox_Bool(const CSphere& sphere, const CAABox& aabb) {
  float mag = 0.f;

  const CVector3f& center = sphere.GetCenter();
  const CVector3f& minPoint = aabb.GetMinPoint();
  const CVector3f& maxPoint = aabb.GetMaxPoint();

  for (int i = 0; i < 3; ++i) {
    if (center[i] < minPoint[i]) {
      float tmp = center[i] - minPoint[i];
      tmp *= tmp;
      mag += tmp;
    } else if (center[i] > maxPoint[i]) {
      float tmp = center[i] - maxPoint[i];
      tmp *= tmp;
      mag += tmp;
    }
  }

  return !(mag > sphere.GetRadius() * sphere.GetRadius());
}

namespace Collide {
bool Sphere_AABox_Bool(const CInternalCollisionStructure& collision) {
  const CAABox& primBox =
      static_cast< const CCollidableAABox& >(collision.GetRight().GetPrim()).GetBox();
  CVector3f boxOrigin = collision.GetRight().GetTransform().GetTranslation();

  CSphere sphere = static_cast< const CCollidableSphere& >(collision.GetLeft().GetPrim())
                       .Transform(collision.GetLeft().GetTransform());
  CAABox box(primBox.GetMinPoint() + boxOrigin, primBox.GetMaxPoint() + boxOrigin);
  return CCollidableSphere::Sphere_AABox_Bool(sphere, box);
}

bool Sphere_AABox(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  const CCollidableSphere& p0 =
      static_cast< const CCollidableSphere& >(collision.GetLeft().GetPrim());
  const CCollidableAABox& p1 =
      static_cast< const CCollidableAABox& >(collision.GetRight().GetPrim());
  const CAABox& primBox = p1.GetBox();
  CVector3f boxOrigin = collision.GetRight().GetTransform().GetTranslation();
  CSphere sphere = p0.Transform(collision.GetLeft().GetTransform());
  CAABox box(primBox.GetMinPoint() + boxOrigin, primBox.GetMaxPoint() + boxOrigin);

  const CVector3f center = sphere.GetCenter();
  const CVector3f min = box.GetMinPoint();
  const CVector3f max = box.GetMaxPoint();
  float distSq = 0.f;
  int flags = 0;
  for (int i = 0; i < 3; ++i) {
    if (center[i] < min[i]) {
      if (center[i] + sphere.GetRadius() >= min[i]) {
        float dist = center[i] - min[i];
        dist *= dist;
        distSq += dist;
        flags |= 1 << (2 * i);
      } else {
        return false;
      }
    } else if (center[i] > max[i]) {
      if (center[i] - sphere.GetRadius() <= max[i]) {
        float dist = center[i] - max[i];
        dist *= dist;
        distSq += dist;
        flags |= 1 << (2 * i + 1);
      } else {
        return false;
      }
    }
  }

  if (flags == 0) {
    CVector3f normal = (center - box.GetCenterPoint()).AsNormalized();
    CVector3f point = center + sphere.GetRadius() * normal;
    list.Add(CCollisionInfo(point, p0.GetMaterial(), p1.GetMaterial(), normal), false);
    return true;
  }
  if (distSq > sphere.GetRadius() * sphere.GetRadius()) {
    return false;
  }

  CVector3f point = CVector3f::Zero();
  switch (flags) {
  case 0x1a:
    point = CVector3f(box.GetMaxPoint().GetX(), box.GetMaxPoint().GetY(), box.GetMinPoint().GetZ());
    break;
  case 0x19:
    point = CVector3f(box.GetMinPoint().GetX(), box.GetMaxPoint().GetY(), box.GetMinPoint().GetZ());
    break;
  case 0x16:
    point = CVector3f(box.GetMaxPoint().GetX(), box.GetMinPoint().GetY(), box.GetMinPoint().GetZ());
    break;
  case 0x15:
    point = CVector3f(box.GetMinPoint().GetX(), box.GetMinPoint().GetY(), box.GetMinPoint().GetZ());
    break;
  case 0x2a:
    point = CVector3f(box.GetMaxPoint().GetX(), box.GetMaxPoint().GetY(), box.GetMaxPoint().GetZ());
    break;
  case 0x29:
    point = CVector3f(box.GetMinPoint().GetX(), box.GetMaxPoint().GetY(), box.GetMaxPoint().GetZ());
    break;
  case 0x26:
    point = CVector3f(box.GetMaxPoint().GetX(), box.GetMinPoint().GetY(), box.GetMaxPoint().GetZ());
    break;
  case 0x25:
    point = CVector3f(box.GetMinPoint().GetX(), box.GetMinPoint().GetY(), box.GetMaxPoint().GetZ());
    break;
  case 0x11:
    point = CVector3f(min[0], center[1], min[2]);
    break;
  case 0x12:
    point = CVector3f(max[0], center[1], min[2]);
    break;
  case 0x14:
    point = CVector3f(center[0], min[1], min[2]);
    break;
  case 0x18:
    point = CVector3f(center[0], max[1], min[2]);
    break;
  case 0x5:
    point = CVector3f(min[0], min[1], center[2]);
    break;
  case 0x6:
    point = CVector3f(max[0], min[1], center[2]);
    break;
  case 0x9:
    point = CVector3f(min[0], max[1], center[2]);
    break;
  case 0xa:
    point = CVector3f(max[0], max[1], center[2]);
    break;
  case 0x21:
    point = CVector3f(min[0], center[1], max[2]);
    break;
  case 0x22:
    point = CVector3f(max[0], center[1], max[2]);
    break;
  case 0x24:
    point = CVector3f(center[0], min[1], max[2]);
    break;
  case 0x28:
    point = CVector3f(center[0], max[1], max[2]);
    break;
  case 0x1:
    point = CVector3f(min[0], center[1], center[2]);
    break;
  case 0x2:
    point = CVector3f(max[0], center[1], center[2]);
    break;
  case 0x4:
    point = CVector3f(center[0], min[1], center[2]);
    break;
  case 0x8:
    point = CVector3f(center[0], max[1], center[2]);
    break;
  case 0x10:
    point = CVector3f(center[0], center[1], min[2]);
    break;
  case 0x20:
    point = CVector3f(center[0], center[1], max[2]);
    break;
  default:
    break;
  }

  CUnitVector3f normal(sphere.GetCenter() - point, CUnitVector3f::kN_Yes);
  list.Add(CCollisionInfo(point, p0.GetMaterial(), p1.GetMaterial(), normal), false);
  return true;
}

bool Sphere_Sphere(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  const CCollidableSphere& p0 =
      static_cast< const CCollidableSphere& >(collision.GetLeft().GetPrim());
  const CCollidableSphere& p1 =
      static_cast< const CCollidableSphere& >(collision.GetRight().GetPrim());

  CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  CSphere s1 = p1.Transform(collision.GetRight().GetTransform());
  CVector3f delta = s0.GetCenter() - s1.GetCenter();
  float deltaMagSq = delta.MagSquared();
  float radiusSum = s0.GetRadius() + s1.GetRadius();
  if (deltaMagSq <= radiusSum * radiusSum) {
    CVector3f normal = delta.CanBeNormalized()
                           ? (1.f / CMath::SqrtF(deltaMagSq)) * delta
                           : static_cast< const CVector3f& >(CVector3f::Right());
    CVector3f point = s1.GetCenter() + s1.GetRadius() * normal;
    list.Add(CCollisionInfo(point, p0.GetMaterial(), p1.GetMaterial(), normal), false);
    return true;
  }
  return false;
}

bool Sphere_Sphere_Bool(const CInternalCollisionStructure& collision) {
  const CCollidableSphere& p0 =
      static_cast< const CCollidableSphere& >(collision.GetLeft().GetPrim());
  const CCollidableSphere& p1 =
      static_cast< const CCollidableSphere& >(collision.GetRight().GetPrim());

  CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  CSphere s1 = p1.Transform(collision.GetRight().GetTransform());
  CVector3f diff = s0.GetCenter() - s1.GetCenter();
  float mag = diff.MagSquared();
  float radius = s0.GetRadius() + s1.GetRadius();
  return mag <= (radius * radius);
}
} // namespace Collide

CRayCastResult
CCollidableSphere::CastRayInternal(const CInternalRayCastStructure& internalRayCast) const {
  if (!internalRayCast.GetFilter().Passes(GetMaterial())) {
    return CRayCastResult::MakeInvalid();
  }

  const CSphere& sphere = Transform(internalRayCast.GetTransform());
  float t = 0.f;
  CVector3f point(0.f, 0.f, 0.f);
  CVector3f normal(0.f, 0.f, 1.f);
  if (CollisionUtil::RaySphereIntersection(sphere, internalRayCast.GetStart(),
                                           internalRayCast.GetNormal(),
                                           internalRayCast.GetMaxTime(), t, point)) {
    CVector3f delta = point - sphere.GetCenter();
    float mag = delta.Magnitude();
    if (mag > 0.01f) {
      normal = (1.f / mag) * delta;
    } else {
      normal = internalRayCast.GetNormal();
    }
    return CRayCastResult(t, point, CPlane(CVector3f(point), CUnitVector3f(normal)), GetMaterial());
  }
  return CRayCastResult::MakeInvalid();
}

CAABox CCollidableSphere::CalculateAABox(const CTransform4f& xf) const {
  const float radius = x10_sphere.GetRadius();
  CVector3f xfPos = xf * x10_sphere.GetCenter();
  const float x = xfPos.GetX();
  const float y = xfPos.GetY();
  const float z = xfPos.GetZ();
  return CAABox(CVector3f(x - radius, y - radius, z - radius),
                CVector3f(x + radius, y + radius, z + radius));
}

CAABox CCollidableSphere::CalculateLocalAABox() const {
  const float radius = x10_sphere.GetRadius();
  CVector3f xfPos = x10_sphere.GetCenter();
  const float x = xfPos.GetX();
  const float y = xfPos.GetY();
  const float z = xfPos.GetZ();
  return CAABox(CVector3f(x - radius, y - radius, z - radius),
                CVector3f(x + radius, y + radius, z + radius));
}

FourCC CCollidableSphere::GetPrimType() const { return 'SPHR'; }

CSphere CCollidableSphere::Transform(const CTransform4f& xf) const {
  const float radius = x10_sphere.GetRadius();
  const CVector3f xfPos = xf * x10_sphere.GetCenter();

  return CSphere(xfPos, radius);
}

bool CCollidableSphere::CollideMovingAABox(const CInternalCollisionStructure& collision,
                                           const CVector3f& dir, double& dOut,
                                           CCollisionInfo& infoOut) {
  const CCollidableSphere& p0 =
      static_cast< const CCollidableSphere& >(collision.GetLeft().GetPrim());
  const CCollidableAABox& p1 =
      static_cast< const CCollidableAABox& >(collision.GetRight().GetPrim());

  CAABox box = p1.CalculateAABox(collision.GetRight().GetTransform());
  CSphere sphere = p0.Transform(collision.GetLeft().GetTransform());
  double d = dOut;
  CVector3f point = CVector3f::Zero();
  CVector3f normal = CVector3f::Zero();
  if (CollisionUtil::MovingSphereAABox(sphere, box, dir, d, point, normal) && d < dOut) {
    dOut = d;
    infoOut = CCollisionInfo(point, p0.GetMaterial(), p1.GetMaterial(), normal);
    return true;
  }
  return false;
}

bool CCollidableSphere::CollideMovingSphere(const CInternalCollisionStructure& collision,
                                            const CVector3f& dir, double& dOut,
                                            CCollisionInfo& infoOut) {
  const CCollidableSphere& p0 =
      static_cast< const CCollidableSphere& >(collision.GetLeft().GetPrim());
  const CCollidableSphere& p1 =
      static_cast< const CCollidableSphere& >(collision.GetRight().GetPrim());

  CVector3f center = collision.GetLeft().GetTransform() * p0.GetSphere().GetCenter();
  CSphere sphere(collision.GetRight().GetTransform() * p1.GetSphere().GetCenter(),
                 p1.GetSphere().GetRadius() + p0.GetSphere().GetRadius());
  double d = dOut;
  if (CollisionUtil::RaySphereIntersection_Double(sphere, center, dir, d) && d >= 0.0 && d < dOut) {
    CVector3f movedCenter = center + float(d) * dir;
    CVector3f normal = (movedCenter - sphere.GetCenter()).AsNormalized();
    CVector3f point = sphere.GetCenter() + p1.GetSphere().GetRadius() * normal;
    dOut = d;
    infoOut = CCollisionInfo(point, p0.GetMaterial(), p1.GetMaterial(), normal);
    return true;
  }
  return false;
}

uint CCollidableSphere::GetTableIndex() const { return sTableIndex; }
