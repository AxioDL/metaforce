#ifndef _CRAYCASTRESULT
#define _CRAYCASTRESULT

#include "Kyoto/Math/CTransform4f.hpp"
#include "types.h"

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Collision/CMaterialList.hpp"

class CTransform4f;
class CRayCastResult {
public:
  enum EInvalid {
    kI_Invalid,
    kI_Valid,
  };
  CRayCastResult(const float time, const CVector3f& point, const CPlane& plane,
                 const CMaterialList& list)
  : mTime(time), mPoint(point), mPlane(plane), mValid(kI_Valid), mMaterial(list) {}
  CRayCastResult(const EInvalid = kI_Invalid)
  : mTime(0)
  , mPoint(0.f, 0.f, 0.f)
  , mPlane(0.f, CUnitVector3f(CVector3f(1.f, 0.f, 0.f), CUnitVector3f::kN_Yes))
  , mValid(kI_Invalid) {}

  float GetTime() const { return mTime; }
  const CVector3f& GetPoint() const { return mPoint; }
  const CPlane& GetPlane() const { return mPlane; }
  const bool IsValid() const { return mValid; }
  bool GetValid() const { return mValid; }
  // TODO: figure out what's going on here
  bool IsInvalid() const { return mValid == kI_Invalid; }
  const CMaterialList& GetMaterial() const { return mMaterial; }
  void Transform(const CTransform4f& xf);

  static CRayCastResult MakeInvalid() { return CRayCastResult(); }

private:
  float mTime;
  CVector3f mPoint;
  CPlane mPlane;
  bool mValid;
  CMaterialList mMaterial;
};

CHECK_SIZEOF(CRayCastResult, 0x30)

#endif // _CRAYCASTRESULT
