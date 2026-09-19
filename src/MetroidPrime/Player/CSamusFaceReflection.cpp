#include "MetroidPrime/Player/CSamusFaceReflection.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

static const char* const skFaceAssetIdName = "ACS_SamusFace";
static const CTransform4f skFaceModelViewAdjust =
    CTransform4f::Scale(0.3f) * CTransform4f::Translate(CVector3f(0.f, 0.5f, 0.f));

CSamusFaceReflection::CSamusFaceReflection(const CStateManager& mgr)
: x0_modelData(
      CModelData(CAnimRes(gpResourceFactory->GetResourceIdByName(skFaceAssetIdName)->GetId(),
                          CAnimRes::kDefaultCharIdx, CVector3f(1.f, 1.f, 1.f), 0, true)))
, x4c_lights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4))
, x50_lookRot(CQuaternion::NoRotation())
, x60_lookDir(CVector3f::Forward())
, x6c_(0)
, x70_hidden(true) {
  CAnimPlaybackParms parms(0, -1, 1.f, true);
  x0_modelData.AnimationData()->SetAnimation(parms, false);
}

static inline float FaceLookBlend(float dt, float lookDot, float freeLookSpeed) {
  const float lookAng = acosf(CMath::Limit(lookDot, 1.f));
  const float f = lookAng > 0.f ? freeLookSpeed / lookAng : 0.f;
  return CMath::Clamp(0.f, dt * 18.f * f, 1.f);
}

void CSamusFaceReflection::Update(float dt, const CStateManager& mgr, CRandom16& rand) {
  if (const CFirstPersonCamera* const fpCam =
          TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
    CVector3f camTrans = fpCam->GetTranslation();
    x0_modelData.AdvanceAnimationIgnoreParticles(dt, rand, true);

    CActorLights& lights = *x4c_lights;
    lights.SetFindShadowLight(false);

    TAreaId areaId = mgr.GetPlayer()->GetCurrentAreaId();
    if (areaId == kInvalidAreaId)
      return;

    const CVector3f offset(0.125f, 0.125f, 0.125f);
    const CAABox aabb(camTrans - offset, camTrans + offset);

    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(areaId);
    lights.BuildFaceLightList(mgr, area, aabb);

    const CMatrix3f matrix(fpCam->GetTransform().BuildMatrix3f());
    const CUnitVector3f lookDir(matrix.GetColumn(kDY));

    const CQuaternion xfLook1 =
        CQuaternion::LookAt(CUnitVector3f(lookDir), CUnitVector3f(CVector3f::Forward()),
                            CRelAngle::FromRadians(M_2PIF));
    CQuaternion xfLook2 =
        CQuaternion::LookAt(CUnitVector3f(CVector3f::Forward()), xfLook1.Transform(x60_lookDir),
                            CRelAngle::FromRadians(M_2PIF));
    xfLook2 *= xfLook2;

    const CVector3f lookCenter = xfLook2.BuildTransform().GetColumn(kDY);
    const CVector3f lookRotCenter = CVector3f(x50_lookRot.BuildTransform().GetColumn(kDY));
    xfLook2 = CQuaternion::SlerpLocal(x50_lookRot, xfLook2, FaceLookBlend(dt, CVector3f::Dot(lookRotCenter, lookCenter),
                                     dt * gpTweakPlayer->GetFreeLookSpeed() * 0.5f));
    x50_lookRot = xfLook2;
    x60_lookDir = lookDir;
  }
}

void CSamusFaceReflection::PreDraw(const CStateManager& mgr) {
  if ((x6c_ == 2 || (x4c_lights->GetActiveLightCount() < 1 && (x6c_ == 0 || x6c_ == 3))) ||
      !TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
    x70_hidden = true;
  } else {
    x70_hidden = false;
    x0_modelData.AnimationData()->PreRender();
  }
}

void CSamusFaceReflection::Draw(const CStateManager& mgr) const {
  if (x70_hidden)
    return;

  if (const CFirstPersonCamera* fpCam =
          TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {

    const CVector3f camTranslation = fpCam->GetTranslation();
    const CVector3f camYcol = fpCam->GetTransform().GetColumn(kDY);
    const CVector3f camZcol = fpCam->GetTransform().GetColumn(kDZ);

    CQuaternion camRot = CQuaternion::FromMatrix(fpCam->GetTransform());

    float dist = CTweakGui::FaceReflectionDistanceDebugValueToActualValue(
        gpTweakGui->GetFaceReflectionDistance());
    float height = CTweakGui::FaceReflectionHeightDebugValueToActualValue(
        gpTweakGui->GetFaceReflectionHeight());
    float aspect = CTweakGui::FaceReflectionAspectDebugValueToActualValue(
        gpTweakGui->GetFaceReflectionAspect());
    float orthoWidth = CTweakGui::FaceReflectionOrthoWidthDebugValueToActualValue(
        gpTweakGui->GetFaceReflectionOrthoWidth());
    float orthoHeight = CTweakGui::FaceReflectionOrthoHeightDebugValueToActualValue(
        gpTweakGui->GetFaceReflectionOrthoHeight());

    CTransform4f modelXf = CTransform4f((camRot * x50_lookRot).BuildTransform(),
                                        camTranslation + (dist * camYcol) + (height * camZcol)) *
                           skFaceModelViewAdjust;

    CGraphics::SetViewPointMatrix(fpCam->GetTransform());
    CGraphics::SetOrtho(aspect * -orthoWidth, aspect * orthoWidth, orthoHeight, -orthoHeight, -10.f,
                        10.f);

    CActorLights* lights = x6c_ == 1 ? nullptr : x4c_lights.get();
    if (x6c_ == 3) {
      x0_modelData.Render(mgr, modelXf, lights, CModelFlags::Normal());
    } else {
      float transFactor = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Combat
                              ? mgr.GetPlayerState()->GetVisorTransitionFactor()
                              : 0.f;
      if (transFactor > 0.f) {
        x0_modelData.Render(mgr, modelXf, nullptr,
                            CModelFlags::Additive(CColor::Black()).DepthCompareUpdate(true, true));
        x0_modelData.Render(mgr, modelXf, lights,
                            CModelFlags::Additive(transFactor).DepthCompareUpdate(true, false));
      }
    }
  }
}
