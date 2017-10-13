#ifndef __URDE_CCAMERASPLINE_HPP__
#define __URDE_CCAMERASPLINE_HPP__

#include "World/CEntityInfo.hpp"

namespace urde
{
class CStateManager;
class CCameraSpline
{
    friend class CBallCamera;
    std::vector<zeus::CVector3f> x4_positions;
    std::vector<TUniqueId> x14_;
    std::vector<float> x24_t;
    std::vector<zeus::CVector3f> x34_directions;
    float x44_length = 0.f;
    bool x48_closedLoop = false;
    bool GetSurroundingPoints(int idx, rstl::reserved_vector<zeus::CVector3f, 4>& positions,
                              rstl::reserved_vector<zeus::CVector3f, 4>& directions) const;
public:
    CCameraSpline(bool);
    void CalculateKnots(TUniqueId, const std::vector<SConnection>&, CStateManager&);
    void Initialize(TUniqueId, const std::vector<SConnection>&, CStateManager&);
    void Reset(int size);
    void AddKnot(const zeus::CVector3f& pos, const zeus::CVector3f& dir);
    void SetKnotPosition(int idx, const zeus::CVector3f& pos);
    const zeus::CVector3f& GetKnotPosition(int idx) const;
    float GetKnotT(int idx) const;
    float CalculateSplineLength();
    void UpdateSplineLength() { x44_length = CalculateSplineLength(); }
    zeus::CTransform GetInterpolatedSplinePointByLength(float pos);
    zeus::CVector3f GetInterpolatedSplinePointByTime(float time, float range);
};
}

#endif // __URDE_CCAMERASPLINE_HPP__
