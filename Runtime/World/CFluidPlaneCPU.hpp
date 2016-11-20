#ifndef __URDE_CFLUIDPLANECPU_HPP__
#define __URDE_CFLUIDPLANECPU_HPP__

#include "CFluidPlane.hpp"

namespace urde
{
class CFluidUVMotion;
class CFluidPlaneCPU : public CFluidPlane
{
public:
    CFluidPlaneCPU(u32, u32, u32, u32, u32, u32, float, u32, u32, EFluidType, float, const zeus::CVector3f&,
                   float, const CFluidUVMotion&, float, float, float, float, float, float, float, float, float, float,
                   float, float, float, float);
};
}

#endif // __URDE_CFLUIDPLANECPU_HPP__
