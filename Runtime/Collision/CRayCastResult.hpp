#ifndef __URDE_CRAYCASTRESULT_HPP__
#define __URDE_CRAYCASTRESULT_HPP__

#include "CMaterialList.hpp"
#include "zeus/zeus.hpp"

namespace urde
{
class CRayCastResult
{
public:
    enum class EInvalid : u8
    {
        Invalid,
        Valid
    };
private:
    float x0_t = 0.f;
    zeus::CVector3f x4_point;
    zeus::CPlane x10_plane;
    EInvalid x20_invalid = EInvalid::Invalid;
    CMaterialList x28_material;
public:
    CRayCastResult() = default;
    CRayCastResult(const CRayCastResult& other, EInvalid invalid)
        : x0_t(other.x0_t),
          x4_point(other.x4_point),
          x10_plane(other.x10_plane),
          x20_invalid(invalid),
          x28_material(other.x28_material)
    {
    }

    CRayCastResult(float t, const zeus::CVector3f& point,
                   const zeus::CPlane& plane, const CMaterialList& matList)
    : x0_t(t), x4_point(point), x10_plane(plane),
      x20_invalid(EInvalid::Valid), x28_material(matList)
    {}

    void MakeInvalid();

    bool IsInvalid() const { return x20_invalid == EInvalid::Invalid; }
    bool IsValid() const { return x20_invalid == EInvalid::Valid; }
    float GetT() const { return x0_t; }
    const zeus::CVector3f& GetPoint() const { return x4_point; }
    const zeus::CPlane& GetPlane() const { return x10_plane; }
    const CMaterialList& GetMaterial() const { return x28_material; }

    void Transform(const zeus::CTransform&);
};
}

#endif // __URDE_CRAYCASTRESULT_HPP__
