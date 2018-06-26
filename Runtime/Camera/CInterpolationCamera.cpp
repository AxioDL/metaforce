#include "CInterpolationCamera.hpp"
#include "CCameraManager.hpp"
#include "CStateManager.hpp"
#include "Camera/CBallCamera.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptSpindleCamera.hpp"
#include "TCastTo.hpp"

namespace urde
{

CInterpolationCamera::CInterpolationCamera(TUniqueId uid, const zeus::CTransform& xf)
: CGameCamera(uid, false, "Interpolation Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, kInvalidEditorId),
              xf, CCameraManager::ThirdPersonFOV(), CCameraManager::NearPlane(),
              CCameraManager::FarPlane(), CCameraManager::Aspect(), kInvalidUniqueId, false, 0)
{
}

void CInterpolationCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CInterpolationCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    CGameCamera::AcceptScriptMsg(msg, sender, mgr);
}

void CInterpolationCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr)
{
    // Empty
}

void CInterpolationCamera::Render(const CStateManager& mgr) const
{
    // Empty
}

void CInterpolationCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
    // Empty
}

void CInterpolationCamera::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    x15c_currentFov = mgr.GetCameraManager()->GetBallCamera()->GetFov();
    x170_24_perspDirty = true;
    if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphing)
        DeactivateInterpCamera(mgr);

    x18c_time += dt;
    if (x18c_time > x190_maxTime)
        x18c_time = x190_maxTime;

    zeus::CTransform xf = GetTransform();

    if (TCastToConstPtr<CGameCamera> cam = mgr.GetObjectById(x188_targetId))
    {
        zeus::CVector3f targetOrigin = cam->GetTranslation();
        zeus::CVector3f ballLookPos = mgr.GetCameraManager()->GetBallCamera()->GetLookPos();
        if (mgr.GetCameraManager()->GetBallCamera()->GetBehaviour() == CBallCamera::EBallCameraBehaviour::SpindleCamera)
        {
            if (TCastToConstPtr<CScriptSpindleCamera> spindle = mgr.GetObjectById(mgr.GetCameraManager()->GetSpindleCameraId()))
            {
                float mag = (mgr.GetPlayer().GetTranslation() - spindle->GetTranslation()).magnitude();
                ballLookPos = spindle->GetTranslation() + (mag * spindle->GetTransform().frontVector());
            }
        }
        bool deactivate = false;

        if (x1d8_24_)
            deactivate = sub802654d8(xf, targetOrigin, ballLookPos, x190_maxTime, x18c_time);
        else
            deactivate = sub802658c0(xf, targetOrigin, ballLookPos, x1d0_, x1d4_, x190_maxTime, x18c_time);

        SetTransform(xf);
        if (deactivate)
            DeactivateInterpCamera(mgr);
    }
    else
        DeactivateInterpCamera(mgr);
}

void CInterpolationCamera::SetInterpolation(const zeus::CTransform& xf, const zeus::CVector3f& lookPos,
                                            float f1, float f2, float f3, TUniqueId camId, bool b1, CStateManager& mgr)
{
    SetActive(true);
    SetTransform(xf);
    x1c4_lookPos = lookPos;
    x188_targetId = camId;
    x1d8_24_ = b1;
    x190_maxTime = f1;
    x1d0_ = f2;
    x1d4_ = f3;

    if (TCastToConstPtr<CGameCamera> cam = (mgr.GetObjectById(camId)))
    {
        x15c_currentFov = cam->GetFov();
        x170_24_perspDirty = true;
    }
}

void CInterpolationCamera::DeactivateInterpCamera(CStateManager& mgr)
{
    SetActive(false);
    if (!mgr.GetCameraManager()->ShouldBypassInterpolation())
        mgr.GetCameraManager()->SetCurrentCameraId(x188_targetId, mgr);
}

bool CInterpolationCamera::sub802654d8(zeus::CTransform& xf, const zeus::CVector3f& targetOrigin, const zeus::CVector3f& lookPos, float maxTime, float curTime)
{
    return false;
}

bool CInterpolationCamera::sub802658c0(zeus::CTransform &, const zeus::CVector3f &, const zeus::CVector3f &, float, float, float, float)
{
    return false;
}

}
