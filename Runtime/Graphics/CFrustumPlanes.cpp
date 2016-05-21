#include "CFrustumPlanes.hpp"

namespace urde
{

CFrustumPlanes::CFrustumPlanes(const zeus::CTransform& cameraXf, float, float, float, bool, float)
{
}

bool CFrustumPlanes::PointInFrustumPlanes(const zeus::CVector3f& point) const
{
    return false;
}

bool CFrustumPlanes::SphereInFrustumPlanes(const zeus::CSphere& sphere) const
{
    return false;
}

bool CFrustumPlanes::BoxInFrustumPlanes(const zeus::CAABox& box) const
{
    return false;
}

bool CFrustumPlanes::BoxInFrustumPlanes(const rstl::optional_object<zeus::CAABox>& box) const
{
    return false;
}

}
