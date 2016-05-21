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
    float x0_time;
    zeus::CVector3f x4_point;
    zeus::CPlane x10_plane;
    EInvalid x20_invalid = EInvalid::Invalid;
    /*u32 x24_; */
    CMaterialList x28_material;
public:
    CRayCastResult() = default;
    CRayCastResult(const CRayCastResult& other, EInvalid invalid)
        : x0_time(other.x0_time),
          x4_point(other.x4_point),
          x10_plane(other.x10_plane),
          x20_invalid(invalid),
          x28_material(other.x28_material)
    {
    }

    CRayCastResult(float, const zeus::CVector3f&, const zeus::CPlane& plane, const CMaterialList& matList)
        : x28_material(matList)
    {}

    void MakeInvalid();

    bool IsInvalid() const;

    float GetTime() const;
    const zeus::CVector3f& GetPoint() const;
    const zeus::CPlane& GetPlane() const;
    const CMaterialList& GetMaterial() const;
    void Transform(const zeus::CTransform&);
};
}

#endif // __URDE_CRAYCASTRESULT_HPP__
