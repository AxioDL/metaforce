#include "Runtime/World/CWorldShadow.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/World/CWorld.hpp"

namespace metaforce {

CWorldShadow::CWorldShadow(u32 w, u32 h, bool rgba8)
: x0_texture(
      std::make_unique<CTexture>(rgba8 ? ETexelFormat::RGBA8 : ETexelFormat::RGB565, w, h, 1, "World Shadow"sv)) {}

void CWorldShadow::EnableModelProjectedShadow(const zeus::CTransform& pos, s32 lightIdx, float f1) {
  zeus::CTransform texTransform = zeus::lookAt(zeus::skZero3f, x74_lightPos - x68_objPos);
  zeus::CTransform posXf = pos;
  posXf.origin = zeus::skZero3f;
  texTransform = posXf.inverse() * texTransform;
  texTransform = (texTransform * zeus::CTransform::Scale(float(M_SQRT2) * x64_objHalfExtent * f1)).inverse();
  texTransform = zeus::CTransform::Translate(0.5f, 0.f, 0.5f) * texTransform;
  GX::LightMask lightMask;
  lightMask.set(lightIdx);
  // CCubeModel::EnableShadowMaps(*x0_texture, texTransform, lightMask, lightMask);
}

void CWorldShadow::DisableModelProjectedShadow() { CCubeModel::DisableShadowMaps(); }

void CWorldShadow::BuildLightShadowTexture(const CStateManager& mgr, TAreaId aid, s32 lightIdx,
                                           const zeus::CAABox& aabb, bool motionBlur, bool lighten) {
  if (x80_aid != aid || x84_lightIdx != lightIdx) {
    x88_blurReset = true;
    x80_aid = aid;
    x84_lightIdx = lightIdx;
  }

  return; // TODO
  if (aid != kInvalidAreaId) {
    const CGameArea* area = mgr.GetWorld()->GetAreaAlways(aid);
    if (area->IsPostConstructed()) {
      const CWorldLight& light = area->GetPostConstructed()->x60_lightsA[lightIdx];
      zeus::CVector3f centerPoint = aabb.center();
      if (const CPVSAreaSet* pvs = area->GetAreaVisSet()) {
        CPVSVisSet lightSet = pvs->Get1stLightSet(lightIdx);
        g_Renderer->EnablePVS(lightSet, aid);
      } else {
        CPVSVisSet visSet;
        visSet.Reset(EPVSVisSetState::OutOfBounds);
        g_Renderer->EnablePVS(visSet, aid);
      }
      zeus::CVector3f lightToPoint = centerPoint - light.GetPosition();
      x64_objHalfExtent = (aabb.max - centerPoint).magnitude();
      float distance = lightToPoint.magnitude();
      float fov = zeus::radToDeg(std::atan2(x64_objHalfExtent, distance)) * 2.f;
      if (fov >= 0.00001f) {
        lightToPoint.normalize();
        x4_view = zeus::lookAt(light.GetPosition(), centerPoint, zeus::skDown);
        x68_objPos = centerPoint;
        x74_lightPos = light.GetPosition();
        CGraphics::SetViewPointMatrix(x4_view);
        zeus::CFrustum frustum;
        frustum.updatePlanes(x4_view, zeus::SProjPersp(zeus::degToRad(fov), 1.f, 0.1f, distance + x64_objHalfExtent));
        g_Renderer->SetClippingPlanes(frustum);
        g_Renderer->SetPerspective(fov, x0_texture->GetWidth(), x0_texture->GetHeight(), 0.1f, 1000.f);
        SViewport backupVp = CGraphics::g_Viewport;
        zeus::CVector2f backupDepthRange = CGraphics::g_CachedDepthRange;
        g_Renderer->SetViewport(0, 0, x0_texture->GetWidth(), x0_texture->GetHeight());
        CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_FAR);

        x34_model = zeus::lookAt(centerPoint - zeus::CVector3f(0.f, 0.f, 0.1f), light.GetPosition());
        CGraphics::SetModelMatrix(x34_model);

        float extent = float(M_SQRT2) * x64_objHalfExtent;
        g_Renderer->PrimColor(zeus::skWhite);
        CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
        CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                                ERglLogicOp::Clear);
        CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::skPassThru);
        CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
        g_Renderer->BeginTriangleStrip(4);
        g_Renderer->PrimVertex({-extent, 0.f, extent});
        g_Renderer->PrimVertex({extent, 0.f, extent});
        g_Renderer->PrimVertex({-extent, 0.f, -extent});
        g_Renderer->PrimVertex({extent, 0.f, -extent});
        g_Renderer->EndPrimitive();

        CGraphics::SetModelMatrix(zeus::CTransform());
        CCubeModel::SetRenderModelBlack(true);
        CCubeModel::SetDrawingOccluders(true);
        g_Renderer->PrepareDynamicLights({});
        g_Renderer->DrawUnsortedGeometry(aid, 0, 0);
        CCubeModel::SetRenderModelBlack(false);
        CCubeModel::SetDrawingOccluders(false);

        if (lighten) {
          CGraphics::SetModelMatrix(x34_model);
          CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
          CGraphics::SetDepthWriteMode(false, ERglEnum::LEqual, false);
          CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                                  ERglLogicOp::Clear);
          CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::skPassThru);
          CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
          CGraphics::StreamBegin(GX::TRIANGLESTRIP);
          CGraphics::StreamColor(1.f, 1.f, 1.f, 0.25f);
          CGraphics::StreamVertex(-extent, 0.f, extent);
          CGraphics::StreamVertex(extent, 0.f, extent);
          CGraphics::StreamVertex(-extent, 0.f, -extent);
          CGraphics::StreamVertex(extent, 0.f, -extent);
          CGraphics::StreamEnd();
          CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
        }

        if (motionBlur && !x88_blurReset) {
          CGraphics::SetDepthWriteMode(false, ERglEnum::LEqual, false);
          CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                                  ERglLogicOp::Clear);
          CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
          CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5ebc);
          CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
          CGraphics::Render2D(*x0_texture, 0, x0_texture->GetWidth() * 2, x0_texture->GetHeight() * 2,
                              x0_texture->GetWidth() * -2, zeus::CColor{1.f, 0.85f});
          CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
        }

        x88_blurReset = false;

        // TODO
        // m_shader.resolveTexture();
        // CBooRenderer::BindMainDrawTarget();

        g_Renderer->SetViewport(backupVp.x0_left, backupVp.x4_top, backupVp.x8_width, backupVp.xc_height);
        CGraphics::SetDepthRange(backupDepthRange[0], backupDepthRange[1]);
      }
    }
  }
}

void CWorldShadow::ResetBlur() { x88_blurReset = true; }

} // namespace metaforce
