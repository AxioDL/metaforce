#include "CFrustumPlanes.hpp"

namespace urde
{

CFrustumPlanes::CFrustumPlanes(const zeus::CTransform& cameraXf, float, float, float, bool, float)
{
}

bool CFrustumPlanes::PointInFrustumPlanes(const zeus::CVector3f& point) const
{
}

bool CFrustumPlanes::SphereInFrustumPlanes(const zeus::CSphere& sphere) const
{
}

bool CFrustumPlanes::BoxInFrustumPlanes(const zeus::CAABox& box) const
{
}

bool CFrustumPlanes::BoxInFrustumPlanes(const rstl::optional_object<zeus::CAABox>& box) const
{
}

}
