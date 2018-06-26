#ifndef __URDE_CPATHCAMERA_HPP__
#define __URDE_CPATHCAMERA_HPP__

#include "CGameCamera.hpp"
#include "CCameraSpline.hpp"

namespace urde
{

class CPathCamera : public CGameCamera
{
public:
    enum class EInitialSplinePosition
    {
        BallCamBasis,
        Negative,
        Positive,
        ClampBasis
    };
private:

    CCameraSpline x188_spline;
    float x1d4_pos = 0.f;
    float x1d8_time = 0.f;
    float x1dc_lengthExtent;
    float x1e0_;
    float x1e4_;
    EInitialSplinePosition x1e8_initPos;
    u32 x1ec_flags;
    float x1f0_;
    float x1f4_;
public:

    CPathCamera(TUniqueId, std::string_view name, const CEntityInfo& info,
                const zeus::CTransform& xf, bool, float, float, float,
                float, float, u32, EInitialSplinePosition);

    void Accept(IVisitor&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void Think(float, CStateManager&);
    void Render(const CStateManager&) const {}
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    zeus::CTransform MoveAlongSpline(float, CStateManager&);
    void ClampToClosedDoor(CStateManager&);
};

}

#endif // __URDE_CPATHCAMERA_HPP__
