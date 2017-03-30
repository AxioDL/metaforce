#ifndef __URDE_CINTERNALRAYCASTSTRUCTURE_HPP__
#define __URDE_CINTERNALRAYCASTSTRUCTURE_HPP__

#include "zeus/CTransform.hpp"
#include "zeus/CMRay.hpp"
#include "CMaterialFilter.hpp"
namespace urde
{
class CInternalRayCastStructure
{
    zeus::CMRay x0_ray;
    float x38_maxTime;
    zeus::CTransform x3c_xf;
    CMaterialFilter x6c_filter;
public:
    CInternalRayCastStructure(const zeus::CVector3f& start, const zeus::CVector3f& dir, float length,
                              const zeus::CTransform& xf, const CMaterialFilter& filter)
        : x0_ray(start, dir, length),
          x3c_xf(xf),
          x6c_filter(filter)
    {
    }

    const zeus::CMRay& GetRay() const { return x0_ray; }
    const zeus::CVector3f& GetStart() const { return x0_ray.start; }
    const zeus::CVector3f& GetNormal() const { return x0_ray.end; }
    float GetMaxTime() const { return x38_maxTime; }
    const zeus::CTransform& GetTransform() const { return x3c_xf; }
    const CMaterialFilter& GetFilter() const { return x6c_filter; }
};
}

#endif // __URDE_CINTERNALRAYCASTSTRUCTURE_HPP__
