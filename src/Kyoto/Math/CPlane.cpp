#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CMath.hpp"

CPlane::CPlane(const CVector3f& a, const CVector3f& b, const CVector3f& c)
: mNormal(CVector3f::Cross(b - a, c - a)), mConstant(CVector3f::Dot(mNormal, a)) {}

float CPlane::ClipLineSegment(const CVector3f& start, const CVector3f& end) const {
  float dist = -(CVector3f::Dot(start, GetNormal()) - GetConstant()) /
               CVector3f::Dot(end - start, GetNormal());
  return dist <= 0.f ? 0.f : (dist >= 1.f ? 1.f : dist);
}
