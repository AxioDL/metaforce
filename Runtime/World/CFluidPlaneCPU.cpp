#include "CFluidPlaneCPU.hpp"

namespace urde
{
CFluidPlaneCPU::CFluidPlaneCPU(u32 w1, u32 w2, u32 w3, u32 w4, u32 w5, u32 w6, float f1, u32 w7, u32 w8,
                               EFluidType fluidType, float f2, const zeus::CVector3f& v1, float f3,
                               const urde::CFluidUVMotion& motion, float f4, float f5, float f6, float f7, float f8,
                               float f9, float f10, float f11, float f12, float f13, float f14, float f15, float f16,
                               float f17)
: CFluidPlane(w1, w2, w3, fluidType, f3, motion, f17)
{
}
}
