#ifndef __URDE_CORBITPOINTMARKER_HPP__
#define __URDE_CORBITPOINTMARKER_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "Graphics/CModel.hpp"

namespace urde
{
class CStateManager;
class COrbitPointMarker
{
    float x0_zOffset;
    bool x4_camRelZPos = true;
    float x8_lagAzimuth = 0.f;
    float xc_azimuth = 0.f;
    zeus::CVector3f x10_lagTargetPos;
    bool x1c_lastFreeOrbit = false;
    float x20_interpTimer = 0.f;
    float x24_curTime = 0.f;
    TLockedToken<CModel> x28_orbitPointModel;
    void ResetInterpolationTimer(float time) { x20_interpTimer = time; }
public:
    COrbitPointMarker();
    bool CheckLoadComplete();
    void Update(float dt, const CStateManager& mgr);
    void Draw(const CStateManager& mgr) const;
};
}
#endif // __URDE_CORBITPOINTMARKER_HPP__
