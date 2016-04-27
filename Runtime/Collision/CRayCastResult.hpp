#ifndef __URDE_CRAYCASTRESULT_HPP__
#define __URDE_CRAYCASTRESULT_HPP__

#include "CMaterialList.hpp"
#include "zeus/zeus.hpp"

namespace urde
{
class CRayCastResult
{
public:
    enum class EInvalid
    {
        Zero,
        One
    };
private:
    EInvalid invalid = EInvalid::Zero;
    zeus::CVector3f x4_;
    zeus::CVector3f x10_;
    CMaterialList x28_material;
public:
    CRayCastResult();
    CRayCastResult(const CRayCastResult& other, EInvalid) {}
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
