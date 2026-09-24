#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include <Kyoto/Math/CFrustumPlanes.hpp>

#include <Kyoto/Math/CAABox.hpp>
#include <Kyoto/Math/CSphere.hpp>
#include <Kyoto/Math/CTransform4f.hpp>
#include <math.h>

static CUnitVector3f CreateNormal(const CVector3f& a, const CVector3f& b, const CVector3f& c) {
  return CVector3f::Cross(b - a, c - a);
}

CFrustumPlanes::CFrustumPlanes(const CTransform4f& xf, float fov, float aspect, float nearZ,
                               bool useFarPlane, float farZ) {
  float halfFov = fov / 2.f;
  const float cosV = CCast::ToReal32(cos(halfFov));
  const float sinV = CCast::ToReal32(sin(halfFov));
  const float verticalLength = nearZ / cosV;
  const float height = verticalLength * sinV;
#if defined(TARGET_PC)
  const float width = height * aspect;
#else
  halfFov *= aspect;
  const float cosH = CCast::ToReal32(cos(halfFov));
  const float sinH = CCast::ToReal32(sin(halfFov));
  float width = nearZ / cosH;
  width *= sinH;
#endif

  CVector3f corners[4] = {CVector3f(width, nearZ, height), CVector3f(width, nearZ, -height),
                          CVector3f(-width, nearZ, -height), CVector3f(-width, nearZ, height)};
  CVector3f worldCorners[4] = {xf.Rotate(corners[0]), xf.Rotate(corners[1]), xf.Rotate(corners[2]),
                               xf.Rotate(corners[3])};
  CVector3f pos = xf.GetTranslation();
  CVector3f nearPos = xf * CVector3f(0.f, nearZ, 0.f);

  x0_planes.push_back(
      CPlane(nearPos, CreateNormal(worldCorners[0], worldCorners[2], worldCorners[1])));
  x0_planes.push_back(
      CPlane(pos, CreateNormal(CVector3f::Zero(), worldCorners[1], worldCorners[0])));
  x0_planes.push_back(
      CPlane(pos, CreateNormal(CVector3f::Zero(), worldCorners[3], worldCorners[2])));
  x0_planes.push_back(
      CPlane(pos, CreateNormal(CVector3f::Zero(), worldCorners[0], worldCorners[3])));
  x0_planes.push_back(
      CPlane(pos, CreateNormal(CVector3f::Zero(), worldCorners[2], worldCorners[1])));
  if (useFarPlane) {
    x0_planes.push_back(CPlane(farZ - x0_planes[0].GetConstant(), -x0_planes[0].GetNormal()));
  }
}

bool CFrustumPlanes::BoxInFrustumPlanes(const CAABox& box) const {
  for (int i = 0; i < x0_planes.size(); ++i) {
    if (!box.InsidePlane(x0_planes[i])) {
      return false;
    }
  }
  return true;
}

bool CFrustumPlanes::BoxInFrustumPlanes(const rstl::optional_object< CAABox >& box) const {
  if (x0_planes.empty()) {
    return true;
  }

  if (!box.valid()) {
    return false;
  }

  return BoxInFrustumPlanes(*box);
}

int CFrustumPlanes::BoxFrustumPlanesCheck(const CAABox& box) const {
  int ret = 1;

  for (int i = 0; i < x0_planes.size(); ++i) {
    CVector3f closestPoint = box.ClosestPointAlongVector(x0_planes[i].GetNormal());

    if (x0_planes[i].IsFacing(closestPoint)) {
      return 0;
    }

    if (ret == 1) {
      CVector3f furthestPoint = box.FurthestPointAlongVector(x0_planes[i].GetNormal());
      if (x0_planes[i].IsFacing(furthestPoint)) {
        ret = 2;
      }
    }
  }
  return ret;
}

bool CFrustumPlanes::SphereInFrustumPlanes(const CSphere& sphere) const {
  float radius = sphere.GetRadius();
  CVector3f pos = sphere.GetCenter();
  for (int i = 0; i < x0_planes.size(); ++i) {
    if (x0_planes[i].GetHeight(pos) - radius > 0.f) {
      return false;
    }
  }
  return true;
}

bool CFrustumPlanes::PointInFrustumPlanes(const CVector3f& point) const {
  for (int i = 0; i < x0_planes.size(); ++i) {
    if (x0_planes[i].IsFacing(point)) {
      return false;
    }
  }
  return true;
}
