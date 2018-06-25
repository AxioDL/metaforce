#include "CPathCamera.hpp"
#include "CCameraManager.hpp"
#include "CStateManager.hpp"
#include "CBallCamera.hpp"
#include "World/CScriptCameraHint.hpp"
#include "World/CPlayer.hpp"
#include "GameGlobalObjects.hpp"
#include "TCastTo.hpp"

namespace urde
{

CPathCamera::CPathCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                         const zeus::CTransform& xf, bool active, float f1, float f2,
                         float f3, float f4, float f5, u32 flags, EInitialSplinePosition initPos)
: CGameCamera(uid, active, name, info, xf,
              CCameraManager::ThirdPersonFOV(),
              CCameraManager::NearPlane(),
              CCameraManager::FarPlane(),
              CCameraManager::Aspect(), kInvalidUniqueId, 0, 0)
, x188_spline(flags & 1), x1dc_(f1), x1e0_(f2), x1e4_(f3), x1e8_initPos(initPos)
, x1ec_flags(flags), x1f0_(f4), x1f4_(f5)
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
        sub8012DD3C(mgr);

    zeus::CVector3f tmp = ballLook - GetTranslation();
    tmp.z = 0.f;
    if (tmp.canBeNormalized())
        SetTransform(zeus::lookAt(GetTranslation(), ballLook));

    if (x1ec_flags & 4)
        SetTransform(xf);
}

void CPathCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
}

void CPathCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
    CPlayer& player = mgr.GetPlayer();
    float f23 = x188_spline.FindClosestLengthAlongSpline(0.f, player.GetTranslation() +  g_tweakPlayer->GetPlayerBallHalfExtent());

    float f25 = std::max(0.f, f23 - x1dc_);

    zeus::CTransform xf = x188_spline.GetInterpolatedSplinePointByLength(f25);

    float f1 = x188_spline.GetLength();

}

zeus::CTransform CPathCamera::MoveAlongSpline(float, CStateManager& mgr)
{
    return {};
}

void CPathCamera::sub8012DD3C(CStateManager& )
{
}

}
