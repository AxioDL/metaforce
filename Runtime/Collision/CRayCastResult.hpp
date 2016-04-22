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
    CRayCastResult(float, const zeus::CVector3f&, const zeus::CPlane, const CMaterialList& matList)
        : x28_material(matList)
    {}
};
}

#endif // __URDE_CRAYCASTRESULT_HPP__
