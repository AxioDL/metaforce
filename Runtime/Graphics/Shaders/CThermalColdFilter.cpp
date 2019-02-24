#include "CThermalColdFilter.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "hecl/Pipeline.hpp"
#include "GameGlobalObjects.hpp"

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CThermalColdFilter::Initialize() { s_Pipeline = hecl::conv->convert(Shader_CThermalColdFilter{}); }

void CThermalColdFilter::Shutdown() { s_Pipeline.reset(); }

CThermalColdFilter::CThermalColdFilter() {
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    struct Vert {
      zeus::CVector2f m_pos;
      zeus::CVector2f m_uv;
      zeus::CVector2f m_uvNoise;
    } verts[4] = {
        {{-1.f, -1.f}, {0.f, 0.f}, {0.f, 0.f}},
        {{-1.f, 1.f}, {0.f, 1.f}, {0.f, 448.f}},
        {{1.f, -1.f}, {1.f, 0.f}, {640.f, 0.f}},
        {{1.f, 1.f}, {1.f, 1.f}, {640.f, 448.f}},
    };
    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, 48, 4);
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
    boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
    boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(),
                                           g_Renderer->GetRandomStaticEntropyTex()};
    m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr, 1, bufs, stages, nullptr, nullptr,
                                          2, texs, nullptr, nullptr);
    return true;
  } BooTrace);

  setNoiseOffset(0);
  setScale(0.f);
}

void CThermalColdFilter::draw() {
  CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));
  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 4);
}

} // namespace urde
