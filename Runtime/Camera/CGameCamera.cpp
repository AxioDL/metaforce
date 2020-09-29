#include "Runtime/Camera/CGameCamera.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/World/CActorParameters.hpp"

namespace urde {

CGameCamera::CGameCamera(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                         const zeus::CTransform& xf, float fovy, float znear, float zfar, float aspect,
                         TUniqueId watchedId, bool disableInput, u32 controllerIdx)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_watchedObject(watchedId)
, x12c_origXf(xf)
, x15c_currentFov(fovy)
, x160_znear(znear)
, x164_zfar(zfar)
, x168_aspect(aspect)
, x16c_controllerIdx(controllerIdx)
, x170_25_disablesInput(disableInput)
, x180_perspInterpStartFov(fovy)
, x184_perspInterpEndFov(fovy) {

  xe7_29_drawEnabled = false;
}

void CGameCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::AddSplashInhabitant) {
    mgr.GetCameraManager()->SetInsideFluid(true, uid);
    return;
  } else if (msg == EScriptObjectMessage::RemoveSplashInhabitant) {
    mgr.GetCameraManager()->SetInsideFluid(false, kInvalidUniqueId);
    return;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CGameCamera::SetActive(bool active) {
  CActor::SetActive(active);
  xe7_29_drawEnabled = false;
}

zeus::CMatrix4f CGameCamera::GetPerspectiveMatrix() const {
  if (x170_24_perspDirty) {
    const_cast<CGameCamera*>(this)->xec_perspectiveMatrix =
        CGraphics::CalculatePerspectiveMatrix(x15c_currentFov, x168_aspect, x160_znear, x164_zfar, false);
    const_cast<CGameCamera*>(this)->x170_24_perspDirty = false;
  }

  return xec_perspectiveMatrix;
}

zeus::CVector3f CGameCamera::ConvertToScreenSpace(const zeus::CVector3f& v) const {
  zeus::CVector3f rVec = x34_transform.transposeRotate(v - x34_transform.origin);

  if (rVec.isZero())
    return {-1.f, -1.f, 1.f};

  rVec = zeus::CVector3f(rVec.x(), rVec.z(), -rVec.y());
  zeus::CMatrix4f mtx = GetPerspectiveMatrix();
  return mtx.multiplyOneOverW(rVec);
}

zeus::CTransform CGameCamera::ValidateCameraTransform(const zeus::CTransform& newXf,
                                                      const zeus::CTransform& oldXf) const {
  zeus::CTransform xfCpy(newXf);
  if (!zeus::close_enough(newXf.rightVector().magnitude(), 1.f) ||
      !zeus::close_enough(newXf.frontVector().magnitude(), 1.f) ||
      !zeus::close_enough(newXf.upVector().magnitude(), 1.f))
    xfCpy.orthonormalize();
  float f2 = zeus::clamp(-1.f, newXf.frontVector().dot(zeus::skUp), 1.f);
  if (std::fabs(f2) > 0.999f)
    xfCpy = oldXf;

  if (xfCpy.upVector().z() < -0.2f)
    xfCpy = zeus::CQuaternion::fromAxisAngle(xfCpy.frontVector(), M_PIF).toTransform() * xfCpy;

  if (!zeus::close_enough(xfCpy.rightVector().z(), 0.f) && !zeus::close_enough(xfCpy.upVector().z(), 0.f)) {
    if (xfCpy.frontVector().canBeNormalized())
      xfCpy = zeus::lookAt(zeus::skZero3f, xfCpy.frontVector());
    else
      xfCpy = oldXf;
  }

  xfCpy.origin = newXf.origin;
  return xfCpy;
}

void CGameCamera::UpdatePerspective(float dt) {
  if (x174_delayTime > 0.f) {
    x174_delayTime -= dt;
    return;
  }

  if (x178_perspInterpRemTime <= 0.f)
    return;

  x178_perspInterpRemTime -= dt;
  if (x178_perspInterpRemTime <= 0.f) {
    x15c_currentFov = x184_perspInterpEndFov;
    x170_24_perspDirty = true;
  } else {
    x15c_currentFov = zeus::clamp(0.f, (x178_perspInterpRemTime / x17c_perspInterpDur), 1.f) *
                          (x180_perspInterpStartFov - x184_perspInterpEndFov) +
                      x184_perspInterpEndFov;
    x170_24_perspDirty = true;
  }
}

void CGameCamera::SetFovInterpolation(float start, float fov, float time, float delayTime) {
  if (time < 0.f) {
    x15c_currentFov = fov;
    x170_24_perspDirty = true;
    x184_perspInterpEndFov = fov;
    x178_perspInterpRemTime = x174_delayTime = 0.f;
  } else {
    x174_delayTime = std::max(0.f, delayTime);
    x17c_perspInterpDur = time;
    x178_perspInterpRemTime = time;
    x180_perspInterpStartFov = start;
    x184_perspInterpEndFov = fov;
    x15c_currentFov = start;
    x170_24_perspDirty = true;
  }
}

void CGameCamera::SkipFovInterpolation() {
  if (x178_perspInterpRemTime > 0) {
    x15c_currentFov = x184_perspInterpEndFov;
    x170_24_perspDirty = true;
  }

  x178_perspInterpRemTime = x174_delayTime = 0.f;
}
} // namespace urde
