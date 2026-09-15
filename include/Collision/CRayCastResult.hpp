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
  : x0_time(time), x4_point(point), x10_plane(plane), x20_valid(kI_Valid), x28_material(list) {}
  CRayCastResult(const EInvalid = kI_Invalid)
  : x0_time(0)
  , x4_point(0.f, 0.f, 0.f)
  , x10_plane(0.f, CUnitVector3f(CVector3f(1.f, 0.f, 0.f), CUnitVector3f::kN_Yes))
  , x20_valid(kI_Invalid) {}

  float GetTime() const { return x0_time; }
  const CVector3f& GetPoint() const { return x4_point; }
  const CPlane& GetPlane() const { return x10_plane; }
  const bool IsValid() const { return x20_valid; }
  bool GetValid() const { return x20_valid; }
  // TODO: figure out what's going on here
  bool IsInvalid() const { return x20_valid == kI_Invalid; }
  const CMaterialList& GetMaterial() const { return x28_material; }
  void Transform(const CTransform4f& xf);

  static CRayCastResult MakeInvalid() { return CRayCastResult(); }

private:
  float x0_time;
  CVector3f x4_point;
  CPlane x10_plane;
  bool x20_valid;
  CMaterialList x28_material;
};

CHECK_SIZEOF(CRayCastResult, 0x30)

#endif // _CRAYCASTRESULT
