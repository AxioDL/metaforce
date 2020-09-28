#include "Runtime/Graphics/Shaders/CFogVolumeFilter.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <hecl/Pipeline.hpp>
#include <zeus/CColor.hpp>

namespace urde {

CFogVolumeFilter::CFogVolumeFilter() {
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    constexpr std::array<Vert, 4> verts{{
        {{-1.0, -1.0}, {0.0, 0.0}},
        {{-1.0, 1.0}, {0.0, 1.0}},
        {{1.0, -1.0}, {1.0, 0.0}},
        {{1.0, 1.0}, {1.0, 1.0}},
    }};
    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts.data(), sizeof(Vert), verts.size());
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CColor), 1);
    const std::array<boo::ObjToken<boo::ITexture>, 3> texs{
        CGraphics::g_SpareTexture.get(),
        CGraphics::g_SpareTexture.get(),
        g_Renderer->GetFogRampTex().get(),
    };
    constexpr std::array bindIdxs{0, 1, 0};
    constexpr std::array bindDepth{true, true, false};
    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> ubufs{m_uniBuf.get()};

    m_dataBind1Way =
        ctx.newShaderDataBinding(s_1WayPipeline, m_vbo.get(), nullptr, nullptr, ubufs.size(), ubufs.data(), nullptr,
                                 nullptr, nullptr, texs.size(), texs.data(), bindIdxs.data(), bindDepth.data());
    m_dataBind2Way =
        ctx.newShaderDataBinding(s_2WayPipeline, m_vbo.get(), nullptr, nullptr, ubufs.size(), ubufs.data(), nullptr,
                                 nullptr, nullptr, texs.size(), texs.data(), bindIdxs.data(), bindDepth.data());
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
