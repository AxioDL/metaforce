#include "WorldFormat/CCollisionSurface.hpp"

CCollisionSurface::CCollisionSurface(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                     uint flags)
: mA(a), mB(b), mC(c), mFlags(flags) {}

CUnitVector3f CCollisionSurface::GetNormal() const {
  CVector3f baDiff = mB - mA;
  CVector3f caDiff = mC - mA;
  CVector3f tmp = CVector3f::Cross(baDiff, caDiff);
  return tmp;
}

CPlane CCollisionSurface::GetPlane() const {
  const CUnitVector3f norm = GetNormal();
  return CPlane(CVector3f::Dot(norm, mA), norm);
}
