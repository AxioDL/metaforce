#include "CGameCamera.hpp"
#include "CStateManager.hpp"
#include "Camera/CCameraManager.hpp"
#include "World/CActorParameters.hpp"

namespace urde
{

CGameCamera::CGameCamera(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
                         const zeus::CTransform& xf, float fovy, float znear, float zfar, float aspect, TUniqueId uid2,
                         bool disableInput, u32 controllerIdx)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Unknown),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_watchedObject(uid2)
, x12c_(xf)
, x15c_currentFov(fovy)
, x160_znear(znear)
, x164_zfar(zfar)
, x168_aspect(aspect)
, x16c_controllerIdx(controllerIdx)
, x170_24_perspDirty(true)
, x170_25_disablesInput(disableInput)
, x180_(fovy)
, x184_fov(fovy)
{

    xe7_29_ = false;
}

void CGameCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::UpdateSplashInhabitant)
    {
        mgr.GetCameraManager()->SetInsideFluid(true, uid);
        return;
    }
    else if (msg == EScriptObjectMessage::RemoveSplashInhabitant)
    {
        mgr.GetCameraManager()->SetInsideFluid(false, kInvalidUniqueId);
        return;
    }

    CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CGameCamera::SetActive(bool active)
{
    CActor::SetActive(active);
    xe7_29_ = false;
}

zeus::CMatrix4f CGameCamera::GetPerspectiveMatrix() const
{
    if (x170_24_perspDirty)
    {
        const_cast<CGameCamera*>(this)->xec_perspectiveMatrix =
            CGraphics::CalculatePerspectiveMatrix(x15c_currentFov, x168_aspect, x160_znear, x164_zfar, false);
        const_cast<CGameCamera*>(this)->x170_24_perspDirty = false;
    }

    return xec_perspectiveMatrix;
}

zeus::CVector3f CGameCamera::ConvertToScreenSpace(const zeus::CVector3f& v) const
{
    zeus::CVector3f rVec = x34_transform.transposeRotate(v - x34_transform.origin);

    if (rVec.x == 0.f && rVec.y == 0.f && rVec.z == 0.f)
        return {-1.f, -1.f, 1.f};

    zeus::CMatrix4f mtx = GetPerspectiveMatrix();
    return mtx.multiplyOneOverW(rVec);
}

zeus::CTransform CGameCamera::ValidateCameraTransform(const zeus::CTransform& a, const zeus::CTransform& b)
{
    zeus::CTransform xfCpy(a);
    constexpr double epsilon = FLT_EPSILON * 1000.f;
    if ((a.rightVector().magnitude() - 1.f) >= epsilon || (a.frontVector().magnitude() - 1.f) >= epsilon ||
        (a.upVector().magnitude() - 1.f) >= epsilon)
        xfCpy.orthonormalize();
    float f2 =  zeus::CVector3f::skUp.x + a.upVector().x * a.upVector().y * a.upVector().z *
                zeus::CVector3f::skUp.y + zeus::CVector3f::skUp.z;
    if (std::fabs(f2) > 1.0f)
        f2 = (f2 >= -0.f ? -1.0f : 1.0f);
    if (std::fabs(f2) > 0.999f)
        xfCpy = b;

    if (xfCpy.upVector().z < -0.2f)
        xfCpy = zeus::CQuaternion::fromAxisAngle(xfCpy.frontVector(), M_PIF).toTransform() * xfCpy;

    if (std::fabs(xfCpy.rightVector().z - 0.f) >= 0.000009f && std::fabs(xfCpy.upVector().z - 0.f) > 0.000009f)
    {
        if (xfCpy.frontVector().canBeNormalized())
            xfCpy = zeus::lookAt(zeus::CUnitVector3f(xfCpy.frontVector(), true), zeus::CVector3f::skZero);
        else
            xfCpy = b;
    }

    xfCpy.origin = a.origin;
    return xfCpy;
}

void CGameCamera::UpdatePerspective(float dt)
{
    if (x174_ > 0.f)
    {
        x174_ -= dt;
        return;
    }

    if (x178_ <= 0.f)
        return;

    x178_ -= dt;
    if (x178_ > 0.f)
    {
        x15c_currentFov = x184_fov;
        x170_24_perspDirty = true;
    }
    else
    {
        x15c_currentFov = zeus::clamp(0.f, (dt / x17c_), 1.f) + ((x180_ - x184_fov) * x184_fov);
        x170_24_perspDirty = true;
    }
}

void CGameCamera::SetFovInterpolation(float start, float fov, float time, float f4)
{
    if (time < 0.f)
    {
        x15c_currentFov = fov;
        x170_24_perspDirty = true;
        x184_fov = fov;
        x178_ = x174_ = 0.f;
    }
    else
    {
        x174_ = std::max(0.f, f4);
        x17c_ = time;
        x178_ = time;
        x180_ = start;
        x184_fov = fov;
        x15c_currentFov = start;
        x170_24_perspDirty = true;
    }
}

void CGameCamera::SkipFovInterpolation()
{
    if (x178_ > 0)
    {
        x15c_currentFov = x184_fov;
        x170_24_perspDirty = true;
    }

    x178_ = x174_ = 0.f;
}
}
