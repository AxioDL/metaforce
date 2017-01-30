#ifndef __URDE_CCAMERASPLINE_HPP__
#define __URDE_CCAMERASPLINE_HPP__

#include "World/CEntityInfo.hpp"

namespace urde
{
class CStateManager;
class CCameraSpline
{
    std::vector<zeus::CVector3f> x4_;
    std::vector<TUniqueId> x14_;
    std::vector<float> x24_;
    std::vector<zeus::CVector3f> x34_;
    float x44_ = 0.f;
    bool x48_ = false;
public:
    CCameraSpline(bool);
    void CalculateKnots(TUniqueId, const std::vector<SConnection>&, CStateManager&);
    void Initialize(TUniqueId, const std::vector<SConnection>&, CStateManager&);
};
}

#endif // __URDE_CCAMERASPLINE_HPP__
