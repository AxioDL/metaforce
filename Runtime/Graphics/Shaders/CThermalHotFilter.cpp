#include "CThermalHotFilter.hpp"
#include "Graphics/CGraphics.hpp"
#include "hecl/Pipeline.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CThermalHotFilter::Initialize() { s_Pipeline = hecl::conv->convert(Shader_CThermalHotFilter{}); }

void CThermalHotFilter::Shutdown() { s_Pipeline.reset(); }

CThermalHotFilter::CThermalHotFilter() {
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    struct Vert {
      zeus::CVector2f m_pos;
      zeus::CVector2f m_uv;
    } verts[4] = {
        {{-1.0, -1.0}, {0.0, 0.0}},
        {{-1.0, 1.0}, {0.0, 1.0}},
        {{1.0, -1.0}, {1.0, 0.0}},
        {{1.0, 1.0}, {1.0, 1.0}},
    };
    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, 32, 4);
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
    boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
    boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), g_Renderer->GetThermoPalette()};
    m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr, 1, bufs, stages, nullptr, nullptr,
                                          2, texs, nullptr, nullptr);
    return true;
  } BooTrace);
}

void CThermalHotFilter::draw() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CThermalHotFilter::draw", zeus::skMagenta);

  CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);

  // m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 4);
}

} // namespace urde
