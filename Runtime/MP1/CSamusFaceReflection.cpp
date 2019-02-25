#include "CSamusFaceReflection.hpp"
#include "GameGlobalObjects.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "Camera/CFirstPersonCamera.hpp"
#include "World/CPlayer.hpp"
#include "World/CWorld.hpp"

namespace urde::MP1 {

static const zeus::CTransform PreXf = zeus::CTransform::Scale(0.3f) * zeus::CTransform::Translate(0.f, 0.5f, 0.f);

CSamusFaceReflection::CSamusFaceReflection(CStateManager& stateMgr)
: x0_modelData(CAnimRes(g_ResFactory->GetResourceIdByName("ACS_SamusFace")->id, 0, zeus::skOne3f, 0, true))
, x4c_lights(std::make_unique<CActorLights>(8, zeus::skZero3f, 4, 4, false, false, false, 0.1f)) {
  x60_lookDir = zeus::skForward;
  CAnimPlaybackParms parms(0, -1, 1.f, true);
  x0_modelData.AnimationData()->SetAnimation(parms, false);
}

void CSamusFaceReflection::PreDraw(const CStateManager& mgr) {
  if (x6c_ != 2 && (x4c_lights->GetActiveLightCount() >= 1 || (x6c_ != 0 && x6c_ != 3))) {
    if (!TCastToConstPtr<CFirstPersonCamera>(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
      x70_hidden = true;
    } else {
      x70_hidden = false;
      x0_modelData.AnimationData()->PreRender();
    }
  }
}

void CSamusFaceReflection::Draw(const CStateManager& mgr) const {
  if (x70_hidden)
    return;

  if (TCastToConstPtr<CFirstPersonCamera> fpCam = (mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
    zeus::CQuaternion camRot(fpCam->GetTransform().basis);
    float dist = ITweakGui::FaceReflectionDistanceDebugValueToActualValue(g_tweakGui->GetFaceReflectionDistance());
    float height = ITweakGui::FaceReflectionHeightDebugValueToActualValue(g_tweakGui->GetFaceReflectionHeight());
    float aspect = ITweakGui::FaceReflectionAspectDebugValueToActualValue(g_tweakGui->GetFaceReflectionAspect());
    float orthoWidth =
        ITweakGui::FaceReflectionOrthoWidthDebugValueToActualValue(g_tweakGui->GetFaceReflectionOrthoWidth());
    float orthoHeight =
        ITweakGui::FaceReflectionOrthoHeightDebugValueToActualValue(g_tweakGui->GetFaceReflectionOrthoHeight());

    zeus::CTransform modelXf =
        zeus::CTransform(camRot * x50_lookRot, fpCam->GetTransform().basis[1] * dist + fpCam->GetTransform().origin +
                                                   fpCam->GetTransform().basis[2] * height) *
        PreXf;

    CGraphics::SetViewPointMatrix(fpCam->GetTransform());
    CGraphics::SetOrtho(aspect * -orthoWidth, aspect * orthoWidth, orthoHeight, -orthoHeight, -10.f, 10.f);

    CActorLights* lights = x6c_ == 1 ? nullptr : x4c_lights.get();
    if (x6c_ == 3) {
      x0_modelData.Render(mgr, modelXf, lights, CModelFlags(0, 0, 3, zeus::skWhite));
    } else {
      float transFactor;
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Combat)
        transFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
      else
        transFactor = 0.f;
      if (transFactor > 0.f) {
        x0_modelData.Render(mgr, modelXf, nullptr, CModelFlags(7, 0, 3, zeus::skBlack));
        x0_modelData.Render(mgr, modelXf, lights, CModelFlags(7, 0, 1, zeus::CColor(1.f, transFactor)));
      }
    }
  }
}

void CSamusFaceReflection::Update(float dt, const CStateManager& mgr, CRandom16& rand) {
  if (TCastToConstPtr<CFirstPersonCamera> fpCam = (mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
    x0_modelData.AdvanceAnimationIgnoreParticles(dt, rand, true);
    x4c_lights->SetFindShadowLight(false);
    TAreaId areaId = mgr.GetPlayer().GetAreaIdAlways();
    if (areaId == kInvalidAreaId)
      return;

    zeus::CAABox aabb(fpCam->GetTranslation() - 0.125f, fpCam->GetTranslation() + 0.125f);
    const CGameArea* area = mgr.GetWorld()->GetAreaAlways(areaId);
    x4c_lights->BuildFaceLightList(mgr, *area, aabb);

    zeus::CUnitVector3f lookDir(fpCam->GetTransform().basis[1]);
    zeus::CUnitVector3f xfLook =
        zeus::CQuaternion::lookAt(lookDir, zeus::skForward, 2.f * M_PIF).transform(x60_lookDir);
    zeus::CQuaternion xfLook2 = zeus::CQuaternion::lookAt(zeus::skForward, xfLook, 2.f * M_PIF);
    xfLook2 *= xfLook2;
    zeus::CMatrix3f newXf(xfLook2);
    zeus::CMatrix3f prevXf(x50_lookRot);
    float lookDot = prevXf[1].dot(newXf[1]);
    if (std::fabs(lookDot) > 1.f)
      lookDot = lookDot > 0.f ? 1.f : -1.f;
    float lookAng = std::acos(lookDot);
    x50_lookRot = zeus::CQuaternion::slerp(
        x50_lookRot, xfLook2,
        zeus::clamp(0.f, 18.f * dt * ((lookAng > 0.f) ? 0.5f * dt * g_tweakPlayer->GetFreeLookSpeed() / lookAng : 0.f),
                    1.f));
    x60_lookDir = lookDir;
  }
}

} // namespace urde::MP1
