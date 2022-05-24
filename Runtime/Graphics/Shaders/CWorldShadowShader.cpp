#include "Runtime/Graphics/Shaders/CWorldShadowShader.hpp"

#include <array>

#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

//#include <hecl/Pipeline.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {

//static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;
//static boo::ObjToken<boo::IShaderPipeline> s_ZPipeline;

void CWorldShadowShader::Initialize() {
//  s_Pipeline = hecl::conv->convert(Shader_CWorldShadowShader{});
//  s_ZPipeline = hecl::conv->convert(Shader_CWorldShadowShaderZ{});
}

void CWorldShadowShader::Shutdown() {
//  s_Pipeline.reset();
//  s_ZPipeline.reset();
}

CWorldShadowShader::CWorldShadowShader(u32 w, u32 h) : m_w(w), m_h(h) {
//  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
//    m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 16, 4);
//    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
//
//    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
//    constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
//    m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
//                                          stages.data(), nullptr, nullptr, 0, nullptr, nullptr, nullptr);
//    m_zDataBind = ctx.newShaderDataBinding(s_ZPipeline, m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
//                                           stages.data(), nullptr, nullptr, 0, nullptr, nullptr, nullptr);
//
//    m_tex = ctx.newRenderTexture(m_w, m_h, boo::TextureClampMode::ClampToWhite, 1, 0);
//    return true;
//  } BooTrace);
}

void CWorldShadowShader::bindRenderTarget() {
//  CGraphics::g_BooMainCommandQueue->setRenderTarget(m_tex);
}

void CWorldShadowShader::drawBase(float extent) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CWorldShadowShader::drawBase", zeus::skMagenta);

  const std::array<zeus::CVector3f, 4> verts{{
      {-extent, 0.f, extent},
      {extent, 0.f, extent},
      {-extent, 0.f, -extent},
      {extent, 0.f, -extent},
  }};
//  m_vbo->load(verts.data(), sizeof(verts));

  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(/*true*/) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = zeus::skWhite;
//  m_uniBuf->load(&m_uniform, sizeof(m_uniform));
//
//  CGraphics::SetShaderDataBinding(m_zDataBind);
//  CGraphics::DrawArray(0, 4);
}

void CWorldShadowShader::lightenShadow() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CWorldShadowShader::lightenShadow", zeus::skMagenta);

  m_uniform.m_color = zeus::CColor(1.f, 0.25f);
//  m_uniBuf->load(&m_uniform, sizeof(m_uniform));
//
//  CGraphics::SetShaderDataBinding(m_dataBind);
//  CGraphics::DrawArray(0, 4);
}

void CWorldShadowShader::blendPreviousShadow() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CWorldShadowShader::blendPreviousShadow", zeus::skMagenta);

//  if (!m_prevQuad)
//    m_prevQuad.emplace(EFilterType::Blend, m_tex);
//  zeus::CRectangle rect(0.f, 1.f, 1.f, -1.f);
//  m_prevQuad->draw({1.f, 0.85f}, 1.f, rect);
}

void CWorldShadowShader::resolveTexture() {
//  boo::SWindowRect rect = {0, 0, int(m_w), int(m_h)};
//  CGraphics::g_BooMainCommandQueue->resolveBindTexture(m_tex, rect, false, 0, true, false, true);
}

} // namespace metaforce
