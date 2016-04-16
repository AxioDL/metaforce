#ifndef __URDE_CPATHCAMERA_HPP__
#define __URDE_CPATHCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CPathCamera : public CGameCamera
{
public:
    enum class EInitialSplinePosition
    {
    };
    CPathCamera(TUniqueId, const std::string& name, const CEntityInfo& info,
                const zeus::CTransform& xf, bool, bool, bool, bool, bool,
                float, float, float, float, float, float, float, u32,
                EInitialSplinePosition);
};

}

#endif // __URDE_CPATHCAMERA_HPP__
