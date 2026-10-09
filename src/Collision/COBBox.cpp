#include "Collision/COBBox.hpp"

#include "Collision/CMRay.hpp"
#include "Collision/CollisionUtil.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include <float.h>
#include <math.h>

COBBox::COBBox(const CTransform4f& xf, const CVector3f& extents)
: mTransform(xf), mExtents(extents) {}

COBBox::COBBox(CInputStream& in) : mTransform(in), mExtents(in) {}

CAABox COBBox::CalculateAABox(const CTransform4f& xf) const {
  const CTransform4f transform = xf * mTransform;
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  const CVector3f& positive = mExtents * 1.f;
  const CVector3f& negative = mExtents * -1.f;

  for (int i = 0; i < 8; ++i) {
    const CVector3f& point = CVector3f(i & 1 ? positive.GetX() : negative.GetX(),
                                       i & 2 ? positive.GetY() : negative.GetY(),
                                       i & 4 ? positive.GetZ() : negative.GetZ());
    bounds.Include(transform * point);
  }

  return bounds;
}

COBBox COBBox::FromAABox(const CAABox& box, const CTransform4f& xf) {
  CVector3f center = box.GetCenterPoint();
  CVector3f mExtents = box.GetMaxPoint() - center;
  CTransform4f finalTransform = xf * CTransform4f::Translate(center);
  return COBBox(finalTransform, mExtents);
}

bool COBBox::LineIntersectsBox(const CMRay& ray, float& penetration) const {
  CAABox box(-mExtents, mExtents);
  CMRay unscaledRay = ray.GetInvUnscaledTransformRay(mTransform);
  CVector3f direction = CVector3f::Zero();
  return CollisionUtil::RayAABoxIntersection(unscaledRay, box, direction, penetration) != 0;
}

bool COBBox::AABoxIntersectsBox(const CAABox& box) const {
  return OBBIntersectsBox(FromAABox(box, CTransform4f::Identity()));
}

#pragma sym on
bool COBBox::OBBIntersectsBox(const COBBox& box) const {
  const CVector3f delta = box.mTransform.GetTranslation() - mTransform.GetTranslation();
  const CVector3f axes[3] = {mTransform.GetColumn(kDX), mTransform.GetColumn(kDY),
                             mTransform.GetColumn(kDZ)};
  const CVector3f otherAxes[3] = {box.mTransform.GetColumn(kDX), box.mTransform.GetColumn(kDY),
                                  box.mTransform.GetColumn(kDZ)};
  const CVector3f translation(CVector3f::Dot(delta, axes[0]), CVector3f::Dot(delta, axes[1]),
                              CVector3f::Dot(delta, axes[2]));
  const CVector3f& extents = mExtents;
  const CVector3f& otherExtents = box.GetSize();
  float rotation[3][3];
  float ra, rb, t;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      rotation[i][j] = CVector3f::Dot(axes[i], otherAxes[j]);
    }
  }

  for (int i = 0; i < 3; ++i) {
    ra = extents[i];
    rb = otherExtents[kDX] * CMath::AbsF(rotation[i][0]) +
         otherExtents[kDY] * CMath::AbsF(rotation[i][1]) +
         otherExtents[kDZ] * CMath::AbsF(rotation[i][2]);
    t = CMath::AbsF(translation[i]);
    if (t > ra + rb + FLT_EPSILON) {
      return false;
    }
  }

  for (int j = 0; j < 3; ++j) {
    ra = extents[kDX] * CMath::AbsF(rotation[0][j]) + extents[kDY] * CMath::AbsF(rotation[1][j]) +
         extents[kDZ] * CMath::AbsF(rotation[2][j]);
    rb = otherExtents[j];
    t = CMath::AbsF(translation[0] * rotation[0][j] + translation[1] * rotation[1][j] +
                    translation[2] * rotation[2][j]);
    if (t > ra + rb + FLT_EPSILON) {
      return false;
    }
  }

  // Test the axis A0 x B0.
  ra = extents[kDY] * CMath::AbsF(rotation[2][0]) + extents[kDZ] * CMath::AbsF(rotation[1][0]);
  rb = otherExtents[kDY] * CMath::AbsF(rotation[0][2]) +
       otherExtents[kDZ] * CMath::AbsF(rotation[0][1]);
  t = CMath::AbsF(translation[2] * rotation[1][0] - translation[1] * rotation[2][0]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  // Test the axis A0 x B1.
  ra = extents[kDY] * CMath::AbsF(rotation[2][1]) + extents[kDZ] * CMath::AbsF(rotation[1][1]);
  rb = otherExtents[kDX] * CMath::AbsF(rotation[0][2]) +
       otherExtents[kDZ] * CMath::AbsF(rotation[0][0]);
  t = CMath::AbsF(translation[2] * rotation[1][1] - translation[1] * rotation[2][1]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  // Test the axis A0 x B2.
  ra = extents[kDY] * CMath::AbsF(rotation[2][2]) + extents[kDZ] * CMath::AbsF(rotation[1][2]);
  rb = otherExtents[kDX] * CMath::AbsF(rotation[0][1]) +
       otherExtents[kDY] * CMath::AbsF(rotation[0][0]);
  t = CMath::AbsF(translation[2] * rotation[1][2] - translation[1] * rotation[2][2]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  // Test the axis A1 x B0.
  ra = extents[kDX] * CMath::AbsF(rotation[2][0]) + extents[kDZ] * CMath::AbsF(rotation[0][0]);
  rb = otherExtents[kDY] * CMath::AbsF(rotation[1][2]) +
       otherExtents[kDZ] * CMath::AbsF(rotation[1][1]);
  t = CMath::AbsF(translation[0] * rotation[2][0] - translation[2] * rotation[0][0]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  // Test the axis A1 x B1.
  ra = extents[kDX] * CMath::AbsF(rotation[2][1]) + extents[kDZ] * CMath::AbsF(rotation[0][1]);
  rb = otherExtents[kDX] * CMath::AbsF(rotation[1][2]) +
       otherExtents[kDZ] * CMath::AbsF(rotation[1][0]);
  t = CMath::AbsF(translation[0] * rotation[2][1] - translation[2] * rotation[0][1]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  // Test the axis A1 x B2.
  ra = extents[kDX] * CMath::AbsF(rotation[2][2]) + extents[kDZ] * CMath::AbsF(rotation[0][2]);
  rb = otherExtents[kDX] * CMath::AbsF(rotation[1][1]) +
       otherExtents[kDY] * CMath::AbsF(rotation[1][0]);
  t = CMath::AbsF(translation[0] * rotation[2][2] - translation[2] * rotation[0][2]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  // Test the axis A2 x B0.
  ra = extents[kDX] * CMath::AbsF(rotation[1][0]) + extents[kDY] * CMath::AbsF(rotation[0][0]);
  rb = otherExtents[kDY] * CMath::AbsF(rotation[2][2]) +
       otherExtents[kDZ] * CMath::AbsF(rotation[2][1]);
  t = CMath::AbsF(translation[1] * rotation[0][0] - translation[0] * rotation[1][0]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  // Test the axis A2 x B1.
  ra = extents[kDX] * CMath::AbsF(rotation[1][1]) + extents[kDY] * CMath::AbsF(rotation[0][1]);
  rb = otherExtents[kDX] * CMath::AbsF(rotation[2][2]) +
       otherExtents[kDZ] * CMath::AbsF(rotation[2][0]);
  t = CMath::AbsF(translation[1] * rotation[0][1] - translation[0] * rotation[1][1]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  // Test the axis A2 x B2.
  ra = extents[kDX] * CMath::AbsF(rotation[1][2]) + extents[kDY] * CMath::AbsF(rotation[0][2]);
  rb = otherExtents[kDX] * CMath::AbsF(rotation[2][1]) +
       otherExtents[kDY] * CMath::AbsF(rotation[2][0]);
  t = CMath::AbsF(translation[1] * rotation[0][2] - translation[0] * rotation[1][2]);
  if (t > ra + rb + FLT_EPSILON) {
    return false;
  }

  return true;
}
