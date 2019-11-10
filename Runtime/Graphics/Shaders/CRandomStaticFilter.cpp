#include "Runtime/Graphics/Shaders/CRandomStaticFilter.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_CookieCutterPipeline;

void CRandomStaticFilter::Initialize() {
  s_AlphaPipeline = hecl::conv->convert(Shader_CRandomStaticFilterAlpha{});
  s_AddPipeline = hecl::conv->convert(Shader_CRandomStaticFilterAdd{});
  s_MultPipeline = hecl::conv->convert(Shader_CRandomStaticFilterMult{});
  s_CookieCutterPipeline = hecl::conv->convert(Shader_CRandomStaticFilterCookieCutter{});
}

void CRandomStaticFilter::Shutdown() {
  s_AlphaPipeline.reset();
  s_AddPipeline.reset();
  s_MultPipeline.reset();
  s_CookieCutterPipeline.reset();
}

static boo::ObjToken<boo::IShaderPipeline> SelectPipeline(EFilterType type) {
  switch (type) {
  case EFilterType::Blend:
    return s_AlphaPipeline;
  case EFilterType::Add:
    return s_AddPipeline;
  case EFilterType::Multiply:
    return s_MultPipeline;
  default:
    return {};
  }
}

CRandomStaticFilter::CRandomStaticFilter(EFilterType type, bool cookieCutter) : m_cookieCutter(cookieCutter) {
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    struct Vert {
      zeus::CVector2f m_pos;
      zeus::CVector2f m_uv;
    };
    const std::array<Vert, 4> verts{{
        {{-1.f, -1.f}, {0.f, 0.f}},
        {{-1.f, 1.f}, {0.f, 448.f}},
        {{1.f, -1.f}, {640.f, 0.f}},
        {{1.f, 1.f}, {640.f, 448.f}},
    }};
    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts.data(), sizeof(Vert), verts.size());
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);

    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
    constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
    const std::array<boo::ObjToken<boo::ITexture>, 1> texs{g_Renderer->GetRandomStaticEntropyTex()};
    m_dataBind = ctx.newShaderDataBinding(m_cookieCutter ? s_CookieCutterPipeline : SelectPipeline(type), m_vbo.get(),
                                          nullptr, nullptr, bufs.size(), bufs.data(), stages.data(), nullptr, nullptr,
                                          texs.size(), texs.data(), nullptr, nullptr);
    return true;
  } BooTrace);
}

void CRandomStaticFilter::draw(const zeus::CColor& color, float t) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CRandomStaticFilter::draw", zeus::skMagenta);

  m_uniform.color = color;
  m_uniform.randOff = ROUND_UP_32(int64_t(rand()) * 32767 / RAND_MAX);
  m_uniform.discardThres = 1.f - t;

  m_uniBuf->load(&m_uniform, sizeof(Uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 4);
}

} // namespace urde
