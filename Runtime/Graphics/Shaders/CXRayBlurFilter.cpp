#include "Runtime/Graphics/Shaders/CXRayBlurFilter.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CTexture.hpp"

//#include <hecl/Pipeline.hpp>
#include <zeus/CVector2f.hpp>

namespace metaforce {

//static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CXRayBlurFilter::Initialize() {
//  s_Pipeline = hecl::conv->convert(Shader_CXRayBlurFilter{});
}

void CXRayBlurFilter::Shutdown() {
//  s_Pipeline.reset();
}

CXRayBlurFilter::CXRayBlurFilter(TLockedToken<CTexture>& tex) : m_paletteTex(tex) { // , m_booTex(tex->GetPaletteTexture())
//  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
//    struct Vert {
//      zeus::CVector2f m_pos;
//      zeus::CVector2f m_uv;
//    };
//    const std::array<Vert, 4> verts{{
//        {{-1.f, -1.f}, {0.f, 0.f}},
//        {{-1.f, 1.f}, {0.f, 1.f}},
//        {{1.f, -1.f}, {1.f, 0.f}},
//        {{1.f, 1.f}, {1.f, 1.f}},
//    }};
//    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts.data(), 32, verts.size());
//    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
//
//    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
//    constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
//    const std::array<boo::ObjToken<boo::ITexture>, 2> texs{
//        CGraphics::g_SpareTexture.get(),
//        m_booTex,
//    };
//    m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
//                                          stages.data(), nullptr, nullptr, texs.size(), texs.data(), nullptr, nullptr);
//    return true;
//  } BooTrace);
}

void CXRayBlurFilter::draw(float amount) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CXRayBlurFilter::draw", zeus::skMagenta);

//  CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);

  const float blurL = amount * g_tweakGui->GetXrayBlurScaleLinear() * 0.25f;
  const float blurQ = amount * g_tweakGui->GetXrayBlurScaleQuadratic() * 0.25f;

  for (size_t i = 0; i < m_uniform.m_uv.size(); ++i) {
    const float iflt = float(i) / 2.f;
    const float uvScale = (1.f - (blurL * iflt + blurQ * iflt * iflt));
    const float uvOffset = uvScale * -0.5f + 0.5f;
    m_uniform.m_uv[i][0][0] = uvScale;
    m_uniform.m_uv[i][1][1] = uvScale;
    m_uniform.m_uv[i][3][0] = uvOffset;
    m_uniform.m_uv[i][3][1] = uvOffset;
  }
//  m_uniBuf->load(&m_uniform, sizeof(m_uniform));
//
//  CGraphics::SetShaderDataBinding(m_dataBind);
//  CGraphics::DrawArray(0, 4);
}

} // namespace metaforce
