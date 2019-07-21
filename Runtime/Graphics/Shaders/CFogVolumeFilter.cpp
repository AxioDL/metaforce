#include "CFogVolumeFilter.hpp"
#include "hecl/Pipeline.hpp"
#include "Graphics/CGraphics.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_1WayPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_2WayPipeline;

void CFogVolumeFilter::Initialize() {
  s_1WayPipeline = hecl::conv->convert(Shader_CFogVolumeFilter1Way{});
  s_2WayPipeline = hecl::conv->convert(Shader_CFogVolumeFilter2Way{});
}

void CFogVolumeFilter::Shutdown() {
  s_1WayPipeline.reset();
  s_2WayPipeline.reset();
}

CFogVolumeFilter::CFogVolumeFilter() {
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
    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, sizeof(Vert), 4);
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CColor), 1);
    boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), CGraphics::g_SpareTexture.get(),
                                           g_Renderer->GetFogRampTex().get()};
    int bindIdxs[] = {0, 1, 0};
    bool bindDepth[] = {true, true, false};
    boo::ObjToken<boo::IGraphicsBuffer> ubufs[] = {m_uniBuf.get()};

    m_dataBind1Way = ctx.newShaderDataBinding(s_1WayPipeline, m_vbo.get(), nullptr, nullptr, 1, ubufs, nullptr, nullptr,
                                              nullptr, 3, texs, bindIdxs, bindDepth);
    m_dataBind2Way = ctx.newShaderDataBinding(s_2WayPipeline, m_vbo.get(), nullptr, nullptr, 1, ubufs, nullptr, nullptr,
                                              nullptr, 3, texs, bindIdxs, bindDepth);
    return true;
  } BooTrace);
}

void CFogVolumeFilter::draw2WayPass(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CFogVolumeFilter::draw2WayPass", zeus::skMagenta);

  m_uniBuf->load(&color, sizeof(zeus::CColor));
  CGraphics::SetShaderDataBinding(m_dataBind2Way);
  CGraphics::DrawArray(0, 4);
}

void CFogVolumeFilter::draw1WayPass(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CFogVolumeFilter::draw1WayPass", zeus::skMagenta);

  m_uniBuf->load(&color, sizeof(zeus::CColor));
  CGraphics::SetShaderDataBinding(m_dataBind1Way);
  CGraphics::DrawArray(0, 4);
}

} // namespace urde
