#include "Runtime/Graphics/Shaders/CThermalHotFilter.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

//#include <hecl/Pipeline.hpp>
#include <zeus/CVector2f.hpp>

namespace metaforce {

//static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CThermalHotFilter::Initialize() {
//  s_Pipeline = hecl::conv->convert(Shader_CThermalHotFilter{});
}

void CThermalHotFilter::Shutdown() {
//  s_Pipeline.reset();
}

CThermalHotFilter::CThermalHotFilter() {
//  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
//    struct Vert {
//      zeus::CVector2f m_pos;
//      zeus::CVector2f m_uv;
//    };
//    const std::array<Vert, 4> verts{{
//        {{-1.0, -1.0}, {0.0, 0.0}},
//        {{-1.0, 1.0}, {0.0, 1.0}},
//        {{1.0, -1.0}, {1.0, 0.0}},
//        {{1.0, 1.0}, {1.0, 1.0}},
//    }};
//    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts.data(), 32, verts.size());
//    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
//
//    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
//    constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
//    const std::array<boo::ObjToken<boo::ITexture>, 2> texs{
//        CGraphics::g_SpareTexture.get(),
//        g_Renderer->GetThermoPalette(),
//    };
//    m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
//                                          stages.data(), nullptr, nullptr, texs.size(), texs.data(), nullptr, nullptr);
//    return true;
//  } BooTrace);
}

void CThermalHotFilter::draw() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CThermalHotFilter::draw", zeus::skMagenta);

//  CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);

  // m_uniBuf->load(&m_uniform, sizeof(m_uniform));

//  CGraphics::SetShaderDataBinding(m_dataBind);
//  CGraphics::DrawArray(0, 4);
}

} // namespace metaforce
