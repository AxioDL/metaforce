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

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

static const char* const skFaceAssetIdName = "ACS_SamusFace";
static const CTransform4f skFaceModelViewAdjust =
    CTransform4f::Scale(0.3f) * CTransform4f::Translate(CVector3f(0.f, 0.5f, 0.f));

CSamusFaceReflection::CSamusFaceReflection(const CStateManager& mgr)
: mModelData(
      CModelData(CAnimRes(gpResourceFactory->GetResourceIdByName(skFaceAssetIdName)->GetId(),
                          CAnimRes::kDefaultCharIdx, CVector3f(1.f, 1.f, 1.f), 0, true)))
, mLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4))
, mLookRot(CQuaternion::NoRotation())
, mLookDir(CVector3f::Forward())
, x6c_(0)
, mHidden(true) {
  CAnimPlaybackParms parms(0, -1, 1.f, true);
  mModelData.AnimationData()->SetAnimation(parms, false);
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
    mModelData.AdvanceAnimationIgnoreParticles(dt, rand, true);

    CActorLights& lights = *mLights;
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
        CQuaternion::LookAt(CUnitVector3f(CVector3f::Forward()), xfLook1.Transform(mLookDir),
                            CRelAngle::FromRadians(M_2PIF));
    xfLook2 *= xfLook2;

    const CVector3f lookCenter = xfLook2.BuildTransform().GetColumn(kDY);
    const CVector3f lookRotCenter = CVector3f(mLookRot.BuildTransform().GetColumn(kDY));
    xfLook2 = CQuaternion::SlerpLocal(mLookRot, xfLook2, FaceLookBlend(dt, CVector3f::Dot(lookRotCenter, lookCenter),
                                     dt * gpTweakPlayer->GetFreeLookSpeed() * 0.5f));
    mLookRot = xfLook2;
    mLookDir = lookDir;
  }
}

void CSamusFaceReflection::PreDraw(const CStateManager& mgr) {
  if ((x6c_ == 2 || (mLights->GetActiveLightCount() < 1 && (x6c_ == 0 || x6c_ == 3))) ||
      !TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
    mHidden = true;
  } else {
    mHidden = false;
    mModelData.AnimationData()->PreRender();
  }
}

void CSamusFaceReflection::Draw(const CStateManager& mgr) const {
  if (mHidden)
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

    CTransform4f modelXf = CTransform4f((camRot * mLookRot).BuildTransform(),
                                        camTranslation + (dist * camYcol) + (height * camZcol)) *
                           skFaceModelViewAdjust;

    CGraphics::SetViewPointMatrix(fpCam->GetTransform());
    CGraphics::SetOrtho(aspect * -orthoWidth, aspect * orthoWidth, orthoHeight, -orthoHeight, -10.f,
                        10.f);
#if defined(TARGET_PC)
    metaforce::AdjustUiProjection();
#endif

    CActorLights* lights = x6c_ == 1 ? nullptr : mLights.get();
    if (x6c_ == 3) {
      mModelData.Render(mgr, modelXf, lights, CModelFlags::Normal());
    } else {
      float transFactor = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Combat
                              ? mgr.GetPlayerState()->GetVisorTransitionFactor()
                              : 0.f;
      if (transFactor > 0.f) {
        mModelData.Render(mgr, modelXf, nullptr,
                            CModelFlags::Additive(CColor::Black()).DepthCompareUpdate(true, true));
        mModelData.Render(mgr, modelXf, lights,
                            CModelFlags::Additive(transFactor).DepthCompareUpdate(true, false));
      }
    }
  }
}
