#ifndef __URDE_CFRUSTUMPLANES_HPP__
#define __URDE_CFRUSTUMPLANES_HPP__

#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include "optional.hpp"

namespace urde
{

class CFrustumPlanes
{
public:
    CFrustumPlanes(const zeus::CTransform& cameraXf, float, float, float, bool, float);

    bool PointInFrustumPlanes(const zeus::CVector3f& point) const;
    bool SphereInFrustumPlanes(const zeus::CSphere& sphere) const;
    bool BoxInFrustumPlanes(const zeus::CAABox& box) const;
    bool BoxInFrustumPlanes(const std::experimental::optional<zeus::CAABox>& box) const;
};

}

#endif // __URDE_CFRUSTUMPLANES_HPP__
