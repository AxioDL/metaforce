#ifndef _CPLANE
#define _CPLANE

#include "types.h"

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CPlane {
public:
  CPlane(const CVector3f& vec, const CUnitVector3f& normal)
  : mNormal(normal), mConstant(CVector3f::Dot(vec, normal)) {}
  CPlane(const float constant, const CUnitVector3f& normal)
  : mNormal(normal), mConstant(constant) {}
  CPlane(const CVector3f&, const CVector3f&, const CVector3f&);
  // TODO

  const CUnitVector3f& GetNormal() const { return mNormal; }
  float GetConstant() const { return mConstant; }
  float GetHeight(const CVector3f& pos) const {
    return CVector3f::Dot(GetNormal(), pos) - GetConstant();
  }
  bool IsFacing(const CVector3f& vec) const {
    return CVector3f::Dot(mNormal, vec) >= GetConstant();
  }

  float ClipLineSegment(const CVector3f& start, const CVector3f& end) const;

  // TODO: Fake?
  void SetFrom(const float d, const CVector3f& vec) {
    mNormal.SetX(vec.GetX());
    mNormal.SetY(vec.GetY());
    mNormal.SetZ(vec.GetZ());
    mConstant = d;
  }

private:
  CUnitVector3f mNormal;
  float mConstant;
};
CHECK_SIZEOF(CPlane, 0x10)

#endif // _CPLANE
