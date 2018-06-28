#include "CPathCamera.hpp"
#include "CCameraManager.hpp"
#include "CStateManager.hpp"
#include "CBallCamera.hpp"
#include "World/CScriptCameraHint.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptDoor.hpp"
#include "GameGlobalObjects.hpp"
#include "TCastTo.hpp"

namespace urde
{

CPathCamera::CPathCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                         const zeus::CTransform& xf, bool active, float lengthExtent, float filterMag,
                         float filterProportion, float minEaseDist, float maxEaseDist, u32 flags,
                         EInitialSplinePosition initPos)
: CGameCamera(uid, active, name, info, xf,
              CCameraManager::ThirdPersonFOV(),
              CCameraManager::NearPlane(),
              CCameraManager::FarPlane(),
              CCameraManager::Aspect(), kInvalidUniqueId, 0, 0)
, x188_spline(flags & 1), x1dc_lengthExtent(lengthExtent), x1e0_filterMag(filterMag)
, x1e4_filterProportion(filterProportion), x1e8_initPos(initPos), x1ec_flags(flags)
, x1f0_minEaseDist(minEaseDist), x1f4_maxEaseDist(maxEaseDist)
{
}

void CPathCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CPathCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CGameCamera::AcceptScriptMsg(msg, uid, mgr);

    if (GetActive() && msg == EScriptObjectMessage::InitializedInArea)
        x188_spline.Initialize(GetUniqueId(), GetConnectionList(), mgr);
}

void CPathCamera::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    if (mgr.GetCameraManager()->GetPathCameraId() != GetUniqueId())
        return;

    if (x188_spline.GetSize() <= 0)
        return;

    zeus::CTransform xf = GetTransform();
    zeus::CVector3f ballLook = mgr.GetCameraManager()->GetBallCamera()->GetLookPos();
    if ((x1ec_flags & 0x10))
    {
        if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr))
            ballLook.z = hint->GetTranslation().z;
    }

    if (!mgr.GetPlayer().GetVelocity().canBeNormalized() && (ballLook - GetTranslation()).canBeNormalized())
    {
        if (x1ec_flags & 4)
            SetTransform(x188_spline.GetInterpolatedSplinePointByLength(x1d4_pos));
        else
            SetTransform(zeus::lookAt(GetTranslation(), ballLook));
        return;
    }

    xf = MoveAlongSpline(dt, mgr);
    SetTranslation(xf.origin);

    if (x1ec_flags & 0x20)
        ClampToClosedDoor(mgr);

    zeus::CVector3f tmp = ballLook - GetTranslation();
    tmp.z = 0.f;
    if (tmp.canBeNormalized())
        SetTransform(zeus::lookAt(GetTranslation(), ballLook));

    if (x1ec_flags & 4)
        SetTransform(xf);
}

void CPathCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
    // Empty
}

static const CMaterialFilter kLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough});

void CPathCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
    CPlayer& player = mgr.GetPlayer();
    zeus::CVector3f playerPt = player.GetTranslation() +
                               zeus::CVector3f(0.f, 0.f, g_tweakPlayer->GetPlayerBallHalfExtent());
    float closestLength = x188_spline.FindClosestLengthOnSpline(0.f, playerPt);

    float negLength = std::max(0.f, closestLength - x1dc_lengthExtent);
    zeus::CVector3f negPoint = x188_spline.GetInterpolatedSplinePointByLength(negLength).origin;

    float posLength = std::min(x188_spline.GetLength(), closestLength + x1dc_lengthExtent);
    zeus::CVector3f posPoint = x188_spline.GetInterpolatedSplinePointByLength(posLength).origin;

    zeus::CTransform camXf = mgr.GetCameraManager()->GetBallCamera()->GetTransform();
    if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
        camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);

    bool neg = false;
    if (x1e8_initPos == EInitialSplinePosition::BallCamBasis)
    {
        zeus::CVector3f tmp = playerPt - negPoint;
        if (tmp.canBeNormalized())
        {
            if (tmp.normalized().dot(camXf.basis[1]) > 0.f)
                neg = true;
        }
    }
    else
    {
        neg = x1e8_initPos == EInitialSplinePosition::Negative;
    }

#if 0
    zeus::CVector3f camToSpline = splinePt - camXf.origin;
    mgr.RayStaticIntersection(camXf.origin, camToSpline.normalized(), camToSpline.magnitude(), kLineOfSightFilter);
    zeus::CVector3f camToSpline2 = splinePt2 - camXf.origin;
    mgr.RayStaticIntersection(camXf.origin, camToSpline2.normalized(), camToSpline2.magnitude(), kLineOfSightFilter);
#endif

    zeus::CVector3f viewPoint;
    if (neg)
    {
        x1d4_pos = negLength;
        viewPoint = negPoint;
    }
    else
    {
        x1d4_pos = posLength;
        viewPoint = posPoint;
    }

    if (x1e8_initPos == EInitialSplinePosition::ClampBasis)
    {
        if (x188_spline.ClampLength(playerPt, false, kLineOfSightFilter, mgr) <= negLength)
        {
            x1d4_pos = negLength;
            viewPoint = negPoint;
        }
        else
        {
            x1d4_pos = posLength;
            viewPoint = posPoint;
        }
    }

    SetTransform(zeus::lookAt(viewPoint, mgr.GetCameraManager()->GetBallCamera()->GetFixedLookPos()));
}

zeus::CTransform CPathCamera::MoveAlongSpline(float t, CStateManager& mgr)
{
    zeus::CTransform ret = x34_transform;
    x1d8_time = x188_spline.FindClosestLengthOnSpline(x1d8_time, mgr.GetPlayer().GetTranslation());
    float f30 = x1dc_lengthExtent;
    if (x1ec_flags & 0x8)
    {
        zeus::CVector3f splineToPlayer = mgr.GetPlayer().GetTranslation() -
            x188_spline.GetInterpolatedSplinePointByLength(x1d8_time).origin;
        float distToPlayer = 0.f;
        splineToPlayer.z = 0.f;
        if (splineToPlayer.canBeNormalized())
            distToPlayer = splineToPlayer.magnitude();
        float easedDistT = (distToPlayer - x1f0_minEaseDist) / (x1f4_maxEaseDist - x1f0_minEaseDist);
        f30 *= 1.f - std::sin(zeus::degToRad(zeus::clamp(0.f, easedDistT, 1.f) * 90.f));
    }

    float newPos;
    if (x188_spline.IsClosedLoop())
    {
        float lenA = x188_spline.ValidateLength(x1d8_time + f30);
        newPos = x188_spline.ValidateLength(x1d8_time - f30);
        float disp = std::fabs(x1d4_pos - x1d8_time);
        float remLen = x188_spline.GetLength() - disp;
        if (x1d4_pos > x1d8_time)
        {
            if (disp <= remLen)
                newPos = lenA;
        }
        else
        {
            if (disp > remLen)
                newPos = lenA;
        }
    }
    else
    {
        if (x1d4_pos > x1d8_time)
            newPos = x188_spline.ValidateLength(x1d8_time + f30);
        else
            newPos = x188_spline.ValidateLength(x1d8_time - f30);
    }

    if (x1ec_flags & 0x2)
    {
        x1d4_pos = newPos;
        ret = x188_spline.GetInterpolatedSplinePointByLength(x1d4_pos);
    }
    else
    {
        if (x188_spline.IsClosedLoop())
        {
            float absDelta = std::fabs(newPos - x1d4_pos);
            absDelta = std::min(absDelta, x188_spline.GetLength() - absDelta);
            float tBias = zeus::clamp(-1.f, absDelta / x1e4_filterProportion, 1.f) * x1e0_filterMag * t;
            float tmpAbs = std::fabs(x1d4_pos - newPos);
            float absDelta2 = x188_spline.GetLength() - tmpAbs;
            if (x1d4_pos > newPos)
            {
                if (tmpAbs <= absDelta2)
                    tBias *= -1.f;
            }
            else
            {
                if (tmpAbs > absDelta2)
                    tBias *= -1.f;
            }
            x1d4_pos = x188_spline.ValidateLength(x1d4_pos + tBias);
        }
        else
        {
            x1d4_pos = x188_spline.ValidateLength(
                zeus::clamp(-1.f, (newPos - x1d4_pos) / x1e4_filterProportion, 1.f) * x1e0_filterMag * t + x1d4_pos);
        }
        ret = x188_spline.GetInterpolatedSplinePointByLength(x1d4_pos);
    }

    return ret;
}

void CPathCamera::ClampToClosedDoor(CStateManager& mgr)
{
    if (TCastToConstPtr<CScriptDoor> door =
        mgr.GetObjectById(mgr.GetCameraManager()->GetBallCamera()->GetTooCloseActorId()))
    {
        if (!door->IsOpen() && CBallCamera::IsBallNearDoor(GetTranslation(), mgr))
        {
            x1d4_pos = (x1d4_pos > x1d8_time) ?
                x1d8_time - x1dc_lengthExtent : x1d8_time + x1dc_lengthExtent;
            SetTranslation(x188_spline.GetInterpolatedSplinePointByLength(x1d4_pos).origin);
        }
    }
}

}
