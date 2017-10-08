#include <Runtime/GameGlobalObjects.hpp>
#include "CBallCamera.hpp"
#include "TCastTo.hpp"

namespace urde
{

void CCameraSpring::Reset()
{
    x4_tardis2Sqrt = 2.f * std::sqrt(x0_tardis);
    x10_dx = 0.f;
}

float CCameraSpring::ApplyDistanceSpringNoMax(float targetX, float curX, float dt)
{
    float usePos = xc_k * x10_dx * dt + curX;
    x10_dx += xc_k * (x0_tardis * (targetX - curX) - x4_tardis2Sqrt * x10_dx) * dt;
    return std::max(usePos, targetX);
}

float CCameraSpring::ApplyDistanceSpring(float targetX, float curX, float dt)
{
    float usePos = xc_k * x10_dx * dt + curX;
    x10_dx += xc_k * (x0_tardis * (targetX - curX) - x4_tardis2Sqrt * x10_dx) * dt;
    usePos = std::max(usePos, targetX);
    if (usePos - targetX > x8_max)
        usePos = targetX + x8_max;
    return usePos;
}

CBallCamera::CBallCamera(TUniqueId uid, TUniqueId watchedId, const zeus::CTransform& xf,
                         float fovy, float znear, float zfar, float aspect)
: CGameCamera(uid, true, "Ball Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList),
              xf, fovy, znear, zfar, aspect, watchedId, false, 0), x37c_camSpline(false),
  x214_ballCameraSpring(g_tweakBall->GetBallCameraSpringTardis(),
                        g_tweakBall->GetBallCameraSpringMax(),
                        g_tweakBall->GetBallCameraSpringConstant()),
  x228_ballCameraCentroidSpring(g_tweakBall->GetBallCameraCentroidSpringTardis(),
                                g_tweakBall->GetBallCameraCentroidSpringMax(),
                                g_tweakBall->GetBallCameraCentroidSpringConstant()),
  x23c_ballCameraLookAtSpring(g_tweakBall->GetBallCameraLookAtSpringTardis(),
                              g_tweakBall->GetBallCameraLookAtSpringMax(),
                              g_tweakBall->GetBallCameraLookAtSpringConstant()),
  x250_ballCameraCentroidDistanceSpring(g_tweakBall->GetBallCameraCentroidDistanceSpringTardis(),
                                        g_tweakBall->GetBallCameraCentroidDistanceSpringMax(),
                                        g_tweakBall->GetBallCameraCentroidDistanceSpringConstant()),
  x41c_ballCameraChaseSpring(g_tweakBall->GetBallCameraChaseSpringTardis(),
                             g_tweakBall->GetBallCameraChaseSpringMax(),
                             g_tweakBall->GetBallCameraChaseSpringConstant()),
  x448_ballCameraBoostSpring(g_tweakBall->GetBallCameraBoostSpringTardis(),
                             g_tweakBall->GetBallCameraBoostSpringMax(),
                             g_tweakBall->GetBallCameraBoostSpringConstant())
{
    x18c_24_ = true;
    x18c_25_ = true;
    x18c_26_ = true;
    x18c_27_ = true;
    x18c_28_ = true;
    x18c_29_ = false;
    x18c_30_ = false;
    x18c_31_ = true;
    x18d_24_ = true;
    x18d_25_ = false;
    x18d_26_ = false;
    x18d_27_ = false;
    x18d_28_ = false;
    x18d_29_ = false;
    x18d_30_ = false;
    x18d_31_ = false;
    x18e_24_ = false;
    x18e_25_ = false;
    x18e_26_ = false;
    x18e_27_ = false;
    x18e_28_ = false;

    x190_origMinDistance = g_tweakBall->GetBallCameraMinSpeedDistance();
    x194_minDistance = g_tweakBall->GetBallCameraMinSpeedDistance();
    x198_maxDistance = g_tweakBall->GetBallCameraMaxSpeedDistance();
    x19c_backwardsDistance = g_tweakBall->GetBallCameraBackwardsDistance();

    x1a0_elevation = g_tweakBall->GetBallCameraElevation();
    x1a4_origAnglePerSecond = g_tweakBall->GetBallCameraAnglePerSecond();
    x1a8_anglePerSecond = g_tweakBall->GetBallCameraAnglePerSecond();
    x1b4_lookAtOffset = g_tweakBall->GetBallCameraOffset();

    x404_chaseElevation = g_tweakBall->GetBallCameraChaseElevation();
    x408_chaseDistance = g_tweakBall->GetBallCameraChaseDistance();
    x40c_chaseAnglePerSecond = g_tweakBall->GetBallCameraChaseAnglePerSecond();
    x410_chaseLookAtOffset = g_tweakBall->GetBallCameraChaseLookAtOffset();

    x430_boostElevation = g_tweakBall->GetBallCameraBoostElevation();
    x434_boostDistance = g_tweakBall->GetBallCameraBoostDistance();
    x438_boostAnglePerSecond = g_tweakBall->GetBallCameraBoostAnglePerSecond();
    x43c_boostLookAtOffset = g_tweakBall->GetBallCameraBoostLookAtOffset();

    x468_ = g_tweakBall->x170_;

    x480_ = std::make_unique<u32>();

    SetupColliders(x264_smallColliders, 2.31f, 2.31f, 0.1f, 3, 2.f, 0.5f, -M_PIF / 2.f);
    SetupColliders(x274_mediumColliders, 4.62f, 4.62f, 0.1f, 6, 2.f, 0.5f, -M_PIF / 2.f);
    SetupColliders(x284_largeColliders, 7.f, 7.f, 0.1f, 12, 2.f, 0.5f, -M_PIF / 2.f);
}

void CBallCamera::SetupColliders(std::vector<CCameraCollider>& out, float xMag, float zMag, float radius, int count,
                                 float tardis, float max, float startAngle)
{
    out.reserve(count);
    float theta = startAngle;
    for (int i=0 ; i<count ; ++i)
    {
        float z = std::cos(theta) * zMag;
        if (theta > M_PIF / 2.f)
            z *= 0.25f;
        out.emplace_back(radius, zeus::CVector3f{std::sin(theta) * xMag, 0.f, z}, CCameraSpring{tardis, max, 1.f}, 1.f);
        theta += 2.f * M_PIF / float(count);
    }
}

void CBallCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CBallCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
}

void CBallCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
}

void CBallCamera::SetState(EBallCameraState state, CStateManager& mgr)
{

}

bool CBallCamera::TransitionFromMorphBallState(CStateManager& mgr)
{
    return false;
}

void CBallCamera::TeleportCamera(const zeus::CVector3f& pos, CStateManager& mgr)
{

}

void CBallCamera::TeleportCamera(const zeus::CTransform& xf, CStateManager& mgr)
{

}

void CBallCamera::ResetToTweaks(CStateManager& mgr)
{

}

void CBallCamera::UpdateLookAtPosition(float offset, CStateManager& mgr)
{

}

zeus::CTransform CBallCamera::UpdateLookDirection(const zeus::CVector3f& dir, CStateManager& mgr)
{
    return {};
}

void CBallCamera::ApplyCameraHint(CStateManager& mgr)
{

}

void CBallCamera::ResetPosition()
{

}

}
