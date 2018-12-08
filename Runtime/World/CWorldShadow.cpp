#include "CWorldShadow.hpp"
#include "CWorld.hpp"
#include "CStateManager.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde {

CWorldShadow::CWorldShadow(u32 w, u32 h, bool rgba8) : m_shader(w, h) {}

void CWorldShadow::EnableModelProjectedShadow(const zeus::CTransform& pos, s32 lightIdx, float f1) {
  zeus::CTransform texTransform = zeus::lookAt(zeus::CVector3f::skZero, x74_lightPos - x68_objPos);
  zeus::CTransform posXf = pos;
  posXf.origin = zeus::CVector3f::skZero;
  texTransform = posXf.inverse() * texTransform;
  texTransform = (texTransform * zeus::CTransform::Scale(float(M_SQRT2) * x64_objHalfExtent * f1)).inverse();
  texTransform = zeus::CTransform::Translate(0.5f, 0.f, 0.5f) * texTransform;
  CBooModel::EnableShadowMaps(m_shader.GetTexture().get(), texTransform);

#if CWORLDSHADOW_FEEDBACK
  if (!m_feedback)
    m_feedback.emplace(EFilterType::Blend, m_shader.GetTexture().get());

  zeus::CRectangle rect(0.4f, 0.4f, 0.2f, 0.2f);
  m_feedback->draw(zeus::CColor::skWhite, 1.f, rect);
#endif
}

void CWorldShadow::DisableModelProjectedShadow() { CBooModel::DisableShadowMaps(); }

void CWorldShadow::BuildLightShadowTexture(const CStateManager& mgr, TAreaId aid, s32 lightIdx,
                                           const zeus::CAABox& aabb, bool motionBlur, bool lighten) {
  if (x80_aid != aid || x84_lightIdx != lightIdx) {
    x88_blurReset = true;
    x80_aid = aid;
    x84_lightIdx = lightIdx;
  }

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
        x4_view = zeus::lookAt(light.GetPosition(), centerPoint, zeus::CVector3f::skDown);
        x68_objPos = centerPoint;
        x74_lightPos = light.GetPosition();
        CGraphics::SetViewPointMatrix(x4_view);
        zeus::CFrustum frustum;
        frustum.updatePlanes(x4_view, zeus::SProjPersp(zeus::degToRad(fov), 1.f, 0.1f, distance + x64_objHalfExtent));
        g_Renderer->SetClippingPlanes(frustum);
        g_Renderer->SetPerspective(fov, m_shader.GetWidth(), m_shader.GetHeight(), 0.1f, 1000.f);
        SViewport backupVp = g_Viewport;
        zeus::CVector2f backupDepthRange = CGraphics::g_CachedDepthRange;
        m_shader.bindRenderTarget();
        g_Renderer->SetViewport(0, 0, m_shader.GetWidth(), m_shader.GetHeight());
        CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_FAR);

        x34_model = zeus::lookAt(centerPoint - zeus::CVector3f(0.f, 0.f, 0.1f), light.GetPosition());
        CGraphics::SetModelMatrix(x34_model);

        float extent = float(M_SQRT2) * x64_objHalfExtent;
        /* Depth test and write */
        /* Color white 100% alpha */
        m_shader.drawBase(extent);

        CGraphics::SetModelMatrix(zeus::CTransform::Identity());
        CBooModel::SetDrawingOccluders(true);
        g_Renderer->PrepareDynamicLights({});
        g_Renderer->UpdateAreaUniforms(aid, true);
        g_Renderer->DrawUnsortedGeometry(aid, 0, 0, true);
        CBooModel::SetDrawingOccluders(false);

        if (lighten) {
          CGraphics::SetModelMatrix(x34_model);
          /* No depth test or write */
          /* Color white 25% alpha */
          m_shader.lightenShadow();
        }

        if (motionBlur && !x88_blurReset) {
          /* No depth test or write */
          /* Color white 85% alpha */
          /* Draw in shadow texture */
          m_shader.blendPreviousShadow();
        }

        x88_blurReset = false;

        m_shader.resolveTexture();
        CBooRenderer::BindMainDrawTarget();

        g_Renderer->SetViewport(backupVp.x0_left, backupVp.x4_top, backupVp.x8_width, backupVp.xc_height);
        CGraphics::SetDepthRange(backupDepthRange[0], backupDepthRange[1]);
      }
    }
  }
}

void CWorldShadow::ResetBlur() { x88_blurReset = true; }

} // namespace urde
