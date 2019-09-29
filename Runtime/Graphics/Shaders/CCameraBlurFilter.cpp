#include "Runtime/Graphics/Shaders/CCameraBlurFilter.hpp"

#include <algorithm>
#include <cmath>

#include "Runtime/Graphics/CGraphics.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CCameraBlurFilter::Initialize() { s_Pipeline = hecl::conv->convert(Shader_CCameraBlurFilter{}); }

void CCameraBlurFilter::Shutdown() { s_Pipeline.reset(); }

CCameraBlurFilter::CCameraBlurFilter() {
  CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
    m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 4);
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
    boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
    boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get()};
    m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr, 1, bufs, stages, nullptr, nullptr,
                                          1, texs, nullptr, nullptr);
    return true;
  } BooTrace);
}

void CCameraBlurFilter::draw(float amount, bool clearDepth) {
  if (amount <= 0.f)
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CCameraBlurFilter::draw", zeus::skMagenta);

  SClipScreenRect clipRect(g_Viewport);
  CGraphics::ResolveSpareTexture(clipRect, 0, clearDepth);
  float aspect = CGraphics::g_CroppedViewport.xc_width / float(CGraphics::g_CroppedViewport.x10_height);

  float xFac = CGraphics::g_CroppedViewport.xc_width / float(g_Viewport.x8_width);
  float yFac = CGraphics::g_CroppedViewport.x10_height / float(g_Viewport.xc_height);
  float xBias = CGraphics::g_CroppedViewport.x4_left / float(g_Viewport.x8_width);
  float yBias = CGraphics::g_CroppedViewport.x8_top / float(g_Viewport.xc_height);

  Vert verts[4] = {
      {{-1.0, -1.0}, {xBias, yBias}},
      {{-1.0, 1.0}, {xBias, yBias + yFac}},
      {{1.0, -1.0}, {xBias + xFac, yBias}},
      {{1.0, 1.0}, {xBias + xFac, yBias + yFac}},
  };
  m_vbo->load(verts, sizeof(verts));

  for (int i = 0; i < 6; ++i) {
    float tmp = i;
    tmp *= 2.f * M_PIF;
    tmp /= 6.f;

    float amtX = std::cos(tmp);
    amtX *= amount / 448.f / aspect;

    float amtY = std::sin(tmp);
    amtY *= amount / 448.f;

    m_uniform.m_uv[i][0] = amtX * xFac;
    m_uniform.m_uv[i][1] = amtY * yFac;
  }
  m_uniform.m_opacity = std::min(amount / 2.f, 1.f);
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 4);
}

} // namespace urde
