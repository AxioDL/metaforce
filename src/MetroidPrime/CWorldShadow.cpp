#include "MetroidPrime/CWorldShadow.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "WorldFormat/CPVSAreaSet.hpp"
#include "WorldFormat/CWorldLight.hpp"

#include "dolphin/gx/GXFrameBuffer.h"

#if defined(TARGET_PC)
#include "dolphin/gx/GXAurora.h"
#endif

CWorldShadow::CWorldShadow(uint w, uint h, bool rgba8)
: mTexture(rs_new CTexture(rgba8 ? kTF_RGBA8 : kTF_RGB565, w, h, 1))
, mView(CTransform4f::Identity())
, mModel(CTransform4f::Identity())
, mObjHalfExtent(1.f)
, mObjPos(0.0f, 1.f, 0.0f)
, mLightPos(CVector3f::Zero())
, mAid(kInvalidAreaId)
, mLightIdx(-1)
, mBlurReset(true) {}

CWorldShadow::~CWorldShadow() {
  if (mTexture.get())
    mTexture->ScheduleDeletion();
}

void CWorldShadow::BuildLightShadowTexture(const CStateManager& mgr, TAreaId aid, uint lightIdx,
                                           const CAABox& aabb, bool motionBlur, bool lighten) {
  if (mAid != aid || mLightIdx != lightIdx) {
    mBlurReset = true;
    mAid = aid;
    mLightIdx = lightIdx;
  }

  if (aid != kInvalidAreaId) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(aid);
    if (area.IsLoaded()) {
      const CWorldLight& light = area.GetLightsA()[lightIdx];
      CVector3f centerPoint = aabb.GetCenterPoint();
      const CPVSAreaSet* pvs = area.GetAreaVisSet();
      if (pvs && gkPVSEnabled == 1) {
        CPVSVisSet lightSet = pvs->GetLightSet(lightIdx + pvs->GetNum2ndLights());
        gpRender->EnablePVS(&lightSet, aid.Value());
      } else {
#ifdef __MWERKS__
        gpRender->EnablePVS(&CPVSVisSet(kVSS_OutOfBounds), aid.Value());
#else
        const CPVSVisSet emptySet(kVSS_OutOfBounds);
        gpRender->EnablePVS(&emptySet, aid.Value());
#endif
      }
      CVector3f lightToPoint = centerPoint - light.GetPosition();
      mObjHalfExtent = (aabb.GetMaxPoint() - centerPoint).Magnitude();
      float distance = lightToPoint.Magnitude();
      float fov = CMath::Rad2Deg(CCast::ToReal32(atan2(mObjHalfExtent, distance))) * 2.f;
      if (!(fov < 0.00001f)) {
        lightToPoint.Normalize();
        mView =
            CTransform4f::LookAt(light.GetPosition(), centerPoint, CVector3f(0.0f, 0.0f, -1.0f));
        mObjPos = centerPoint;
        mLightPos = light.GetPosition();
        CGraphics::SetViewPointMatrix(mView);
        CFrustumPlanes frustum(mView, fov * 0.01745329238474369f, 1.0f, 0.1f, true,
                               distance + mObjHalfExtent);
        gpRender->SetClippingPlanes(frustum);
        gpRender->SetPerspective(fov, mTexture->GetWidth(), mTexture->GetHeight(), 0.1f,
                                 1000.f);
        float backupDepthNear = CGraphics::GetDepthNear();
        float backupDepthFar = CGraphics::GetDepthFar();
        CGraphics::SetDepthRange(0.f, 1.0f);
        int backupVpLeft = CGraphics::GetViewport().mLeft;
        int backupVpTop = CGraphics::GetViewport().mTop;
        int backupVpWidth = CGraphics::GetViewport().mWidth;
        int backupVpHeight = CGraphics::GetViewport().mHeight;
#if defined(TARGET_PC)
        GXCreateFrameBuffer(mTexture->GetWidth() * 2, mTexture->GetHeight() * 2);
        gpRender->SetViewport(0, CGraphics::GetRenderMode().efbHeight - mTexture->GetHeight() * 2,
                              mTexture->GetWidth() * 2, mTexture->GetHeight() * 2);
#else
        gpRender->SetViewport(0, 0, mTexture->GetWidth() * 2, mTexture->GetHeight() * 2);
#endif

        float extent = 1.4142f * mObjHalfExtent;
        mModel =
            CTransform4f::LookAt(centerPoint - CVector3f(0.f, 0.f, 0.1f), light.GetPosition());
        gpRender->SetModelMatrix(mModel);

        gpRender->PrimColor(CColor::White());
        CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
        CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
        CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
        CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
        CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);

        gpRender->BeginTriangleStrip(4);
        gpRender->PrimVertex(CVector3f(-extent, 0.f, extent));
        gpRender->PrimVertex(CVector3f(extent, 0.f, extent));
        gpRender->PrimVertex(CVector3f(-extent, 0.f, -extent));
        gpRender->PrimVertex(CVector3f(extent, 0.f, -extent));
        gpRender->EndPrimitive();

        gpRender->SetModelMatrix(CTransform4f::Identity());
        CCubeModel::SetRenderModelBlack(true);
        CCubeModel::SetDrawingOccluders(true);
        gpRender->PrepareDynamicLights(rstl::vector< CLight >());
        gpRender->DrawUnsortedGeometry(aid.value, 0, 0);
        CCubeModel::SetRenderModelBlack(false);
        CCubeModel::SetDrawingOccluders(false);

        if (lighten) {
          gpRender->SetModelMatrix(mModel);
          CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
          CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
          CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
          CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
          CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
          CGraphics::StreamBegin(kP_TriangleStrip);
          CGraphics::StreamColor(1.f, 1.f, 1.f, 0.25f);
          CGraphics::StreamVertex(CVector3f(-extent, 0.f, extent));
          CGraphics::StreamVertex(CVector3f(extent, 0.f, extent));
          CGraphics::StreamVertex(CVector3f(-extent, 0.f, -extent));
          CGraphics::StreamVertex(CVector3f(extent, 0.f, -extent));
          CGraphics::StreamEnd();
          CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
        }

        if (motionBlur && mBlurReset != true) {
          CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
          CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
          CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
          CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
          CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
          CGraphics::Render2D(*mTexture, 0, mTexture->GetWidth() * 2,
                              mTexture->GetHeight() * 2, (-mTexture->GetWidth()) * 2,
                              CColor(1.f, 1.f, 1.f, 0.85f));
          CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
        }

        mBlurReset = false;

#if defined(TARGET_PC)
        GXSetTexCopySrc(0, 0, mTexture->GetWidth() * 2, mTexture->GetHeight() * 2);
#else
        GXSetTexCopySrc(0, 448 - mTexture->GetHeight() * 2, mTexture->GetWidth() * 2,
                        mTexture->GetHeight() * 2);
#endif
        GXSetTexCopyDst(mTexture->GetWidth(), mTexture->GetHeight(),
                        mTexture->GetTexelFormat() == kTF_RGB565 ? GX_TF_RGB565 : GX_TF_RGBA8,
                        true);
        static int unkInt = 0;
        mTexture->SetFlag1(true);
        void* dest = mTexture->GetBitMapData(0);
        GXCopyTex(dest, true);
        mTexture->UnLock();

#if defined(TARGET_PC)
        GXRestoreFrameBuffer();
        gpRender->SetViewport(backupVpLeft,
                              CGraphics::GetRenderMode().efbHeight - backupVpTop - backupVpHeight,
                              backupVpWidth, backupVpHeight);
#else
        gpRender->SetViewport(backupVpLeft, backupVpTop, backupVpWidth, backupVpHeight);
#endif
        CGraphics::SetDepthRange(backupDepthNear, backupDepthFar);
      }
    }
  }
}

void CWorldShadow::EnableModelProjectedShadow(const CTransform4f& pos, uint lightIdx,
                                              float f1) const {

  static float sqrt2 = sqrt(2.0);
  CTransform4f texTransform = CTransform4f::LookAt(CVector3f::Zero(), mLightPos - mObjPos,
                                                   CVector3f(0.0f, 0.0f, 1.0f));
  CTransform4f posXf = pos;
  posXf.SetTranslation(CVector3f::Zero());
  texTransform = posXf.GetInverse() * texTransform;
  texTransform *= CTransform4f::Scale(float(sqrt2) * mObjHalfExtent * f1);
  texTransform = texTransform.GetInverse();
  texTransform = CTransform4f::Translate(0.5f, 0.f, 0.5f) * texTransform;

  uchar lightMask = 1 << lightIdx;
  CCubeModel::EnableShadowMaps(mTexture.get(), texTransform, lightMask, lightMask);
}

void CWorldShadow::DisableModelProjectedShadow() const { CCubeModel::DisableShadowMaps(); }

void CWorldShadow::ResetBlur() { mBlurReset = true; }
