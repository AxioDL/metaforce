#include "CGameCamera.hpp"
#include "CStateManager.hpp"
#include "Camera/CCameraManager.hpp"
#include "World/CActorParameters.hpp"

namespace urde
{

CGameCamera::CGameCamera(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
                         const zeus::CTransform& xf, float fovy, float znear, float zfar, float aspect, TUniqueId uid2,
                         bool b1, u32 w1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Zero),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_watchedObject(uid2)
, x12c_(xf)
, x15c_fov(fovy)
, x160_znear(znear)
, x164_zfar(zfar)
, x168_aspect(aspect)
, x16c_(w1)
, x170_24_perspDirty(true)
, x170_25_disablesInput(b1)
, x180_(fovy)
, x184_(fovy)
{

    xe7_29_ = false;
}

void CGameCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::InternalMessage15)
    {
        mgr.GetCameraManager()->SetInsideFluid(true, uid);
        return;
    }
    else if (msg == EScriptObjectMessage::InternalMessage17)
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
            CGraphics::CalculatePerspectiveMatrix(x15c_fov, x168_aspect, x160_znear, x164_zfar, false);
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
    float f2 =  zeus::kUpVec.x + a.upVector().x * a.upVector().y * a.upVector().z * zeus::kUpVec.y + zeus::kUpVec.z;
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

float CGameCamera::GetNearClipDistance() const { return x160_znear; }

float CGameCamera::GetFarClipDistance() const { return x164_zfar; }

float CGameCamera::GetAspectRatio() const { return x168_aspect; }

TUniqueId CGameCamera::GetWatchedObject() const { return xe8_watchedObject; }

float CGameCamera::GetFov() const { return x15c_fov; }

void CGameCamera::SetFov(float fov) { x15c_fov = fov; }
}
