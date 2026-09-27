#include "MetroidPrime/CVisorFlare.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Collision/CRayCastResult.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/math.hpp"

#include "dolphin/gx/GXTev.h"

#include "math.h"

CVisorFlare::CFlareDef::CFlareDef(const TToken< CTexture >& tex, float pos, float scale, uint color)
: mTex(tex), mPos(pos), mScale(scale), mColor(color) {
  mTex.Lock();
}

rstl::optional_object< CVisorFlare::CFlareDef > CVisorFlare::LoadFlareDef(CInputStream& in) {
  uint propCount = in.ReadLong();
  if (propCount != 4)
    return rstl::optional_object_null();

  CAssetId txtrId = in.Get< CAssetId >();
  float pos = in.ReadFloat();
  float scale = in.ReadFloat();
  CColor color = in.Get< CColor >();
  if (txtrId == kInvalidAssetId)
    return rstl::optional_object_null();

  TToken< CTexture > tex = gpSimplePool->GetObj(SObjectTag('TXTR', txtrId));

  return CFlareDef(tex, pos, scale, color.GetColor_u32());
}

CVisorFlare::CVisorFlare(EBlendMode blendMode, bool distanceScaled, float fadeTime,
                         float angularFalloff, float rotationScale, uint thermalVisorMode,
                         uint combatVisorMode, const rstl::vector< CFlareDef >& flares)
: mBlendMode(blendMode)
, mFlareDefs(flares)
, mDistanceScaled(distanceScaled)
, mFadeTime(rstl::max_val(1.0E-4f, fadeTime))
, mAngularFalloff(angularFalloff)
, mRotationScale(rotationScale)
, mIntensity(0.f)
, mOcclusionTime(0.f)
, mThermalVisorMode(thermalVisorMode)
, mCombatVisorMode(combatVisorMode) {}

void CVisorFlare::Update(float dt, const CVector3f& pos, const CActor* act, CStateManager& mgr) {
  CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();

  if ((visor == CPlayerState::kPV_Combat ||
       (mThermalVisorMode != 1 && visor == CPlayerState::kPV_Thermal)) &&
      mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {

    CVector3f camPos = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTranslation();
    CVector3f camDiff = pos - camPos;
    float mag = camDiff.Magnitude();
    camDiff /= mag;

    bool blocked;
    {
      CMaterialFilter nearMaterialList = CMaterialFilter::MakeInclude(CMaterialList(kMT_Occluder));
      TEntityList nearVec;
      mgr.BuildNearList(nearVec, camPos, camDiff, mag, nearMaterialList, act);

      CMaterialFilter rayMaterialList = CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Solid), CMaterialList(kMT_SeeThrough));
      TUniqueId id(kInvalidUniqueId);
      CRayCastResult result =
          mgr.RayWorldIntersection(id, camPos, camDiff, mag, rayMaterialList, nearVec);

      blocked = result.IsValid();
    }

    if (blocked) {
      mOcclusionTime += dt;
    } else {
      mOcclusionTime -= dt;
    }
    mOcclusionTime = rstl::min_val(mFadeTime, rstl::max_val(0.f, mOcclusionTime));

    const CGameCamera& curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    CVector3f cameraForward = curCam.GetTransform().GetColumn(kDY);
    CVector3f dir = pos - curCam.GetTranslation();
    mIntensity = 1.f - mOcclusionTime / mFadeTime;

    float dot = CVector3f::Dot(dir.AsNormalized(), cameraForward);
    mIntensity *= rstl::max_val(0.f, 1.f - (mAngularFalloff * 4.f * (1.f - dot)));

    if (mThermalVisorMode == 2) {
      mgr.AddThermalColdScale2(mIntensity);
    }
  }
}

void CVisorFlare::Render(const CVector3f& inPos, const CStateManager& mgr) const {
  if (close_enough(mOcclusionTime, mFadeTime) ||
      mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
    return;
  }

  switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
  case CPlayerState::kPV_Combat:
    if (mCombatVisorMode != 0)
      return;
    break;
  case CPlayerState::kPV_Thermal:
    if (mThermalVisorMode != 0)
      return;
    break;
  default:
    return;
  }

  CGraphics::DisableAllLights();
  gpRender->SetDepthReadWrite(false, false);
  const CGameCamera& cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CVector3f camPos = cam.GetTranslation();
  CVector3f inPosCopy = inPos;

  CTransform4f viewMatrix = CGraphics::GetViewMatrix();
  CVector3f invPos = viewMatrix.GetInverse() * inPosCopy;
  invPos = CVector3f(-invPos.GetX(), invPos.GetY(), -invPos.GetZ());
  const CVector3f invPos2 = viewMatrix * invPos;
  CVector3f camFront = cam.GetTransform().GetForward();
  CVector3f toFlare = inPosCopy - camPos;
  if (!close_enough(mIntensity, 0.f)) {
    float angle = 0.f;
    if (!close_enough(mRotationScale, 0.f)) {
      CVector3f camDist(CVector3f(toFlare.GetX(), toFlare.GetY(), 0.f).AsNormalized());
      CVector3f camDir(CVector3f(camFront.GetX(), camFront.GetY(), 0.f).AsNormalized());
      float acos = CMath::ArcCosineR(CVector3f::Dot(camDist, camDir));
      if (camDist.GetX() * camDir.GetY() - camDir.GetX() * camDist.GetY() < 0.f) {
        acos = -acos;
      }
      angle = mRotationScale * acos;
    }
    const bool drawDirect = mgr.GetThermalDrawFlag() == kTD_Hot;
    const float radians = angle;
    SetupRenderState(mgr);
    for (int i = 0; i < mFlareDefs.size(); ++i) {
      const CFlareDef* item = &mFlareDefs[i];
      CVector3f origin = CVector3f::Lerp(inPosCopy, invPos2, item->GetPosition());
      CTransform4f modelMatrix = CTransform4f::LookAt(origin, camPos);
      gpRender->SetModelMatrix(modelMatrix);
      float scale = 0.5f * mIntensity * item->GetScale();
      if (mDistanceScaled) {
        CVector3f dist = origin - camPos;
        if (dist.CanBeNormalized()) {
          scale *= dist.Magnitude();
        }
      }
      if (item->GetTexture().IsLoaded()) {
        TLockedToken< CTexture > tok(item->GetTexture());
        tok->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        float f1;
        float f2;
        if (close_enough(radians, 0.f)) {
          f1 = 0.f;
          f2 = scale;
        } else {
          f1 = scale * sine(CRelAngle::FromRadians(angle));
          f2 = scale * cosine(CRelAngle::FromRadians(angle));
        }
        uint color = item->GetColor().GetColor_u32();
        if (drawDirect) {
          DrawDirect(CColor(color), f1, f2);
        } else {
          DrawStreamed(CColor(color), f1, f2);
        }
      }
    }
    ResetTevSwapMode(mgr);
  }
}

void CVisorFlare::DrawDirect(const CColor& color, float f1, float f2) const {
  CColor kcolor = color;
  kcolor.SetAlpha(kcolor.GetRed() * mIntensity);
  CGX::SetTevKColor(GX_KCOLOR0, kcolor.GetGXColor());
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(f1 - f2, 0.f, f2 + f1);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(f1 + f2, 0.f, f2 - f1);
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(-(f1 + f2), 0.f, -(f2 - f1));
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(-f1 + f2, 0.f, -f2 - f1);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();
}

void CVisorFlare::DrawStreamed(const CColor& color, float f1, float f2) const {
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(color.WithAlphaModulatedBy(mIntensity));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(f1 - f2, 0.f, f2 + f1);
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(f1 + f2, 0.f, f2 - f1);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(-(f1 + f2), 0.f, -(f2 - f1));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(-f1 + f2, 0.f, -f2 - f1);
  CGraphics::StreamEnd();
}

void CVisorFlare::SetupRenderState(const CStateManager& mgr) const {
  if (mgr.GetThermalDrawFlag() == kTD_Hot) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0_A);
    CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    CGX::SetNumTexGens(1);
    CGX::SetNumChans(0);
    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
    switch (mBlendMode) {
    case kBM_Additive:
      CGX::SetNumTevStages(1);
      break;
    case kBM_Blend:
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
      CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXA, GX_CC_CPREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetNumTevStages(2);
      break;
    }
    static const GXVtxDescList vtxDescList[3] = {
        {GX_VA_POS, GX_DIRECT},
        {GX_VA_TEX0, GX_DIRECT},
        {GX_VA_NULL, GX_NONE},
    };
    CGX::SetVtxDescv(vtxDescList);
  } else {
    switch (mBlendMode) {
    case kBM_Additive:
      gpRender->SetBlendMode_AdditiveAlpha();
      break;
    case kBM_Blend:
      gpRender->SetBlendMode_AlphaBlended();
      break;
    }
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }
}

void CVisorFlare::ResetTevSwapMode(const CStateManager& mgr) const {
  if (mgr.GetThermalDrawFlag() == kTD_Hot) {
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
}
