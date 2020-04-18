#include "Runtime/Graphics/Shaders/CColoredQuadFilter.hpp"

#include <array>

#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;

void CColoredQuadFilter::Initialize() {
  s_AlphaPipeline = hecl::conv->convert(Shader_CColoredQuadFilter{});
  s_AddPipeline = hecl::conv->convert(Shader_CColoredQuadFilterAdd{});
  s_MultPipeline = hecl::conv->convert(Shader_CColoredQuadFilterMul{});
}

void CColoredQuadFilter::Shutdown() {
  s_AlphaPipeline.reset();
  s_AddPipeline.reset();
  s_MultPipeline.reset();
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
    return s_AlphaPipeline;
  }
}

CColoredQuadFilter::CColoredQuadFilter(EFilterType type) {
  CGraphics::CommitResources([this, type](boo::IGraphicsDataFactory::Context& ctx) {
    struct Vert {
      zeus::CVector2f m_pos;
    };

    const std::array<Vert, 4> verts{{
        {{0.0, 0.0}},
        {{0.0, 1.0}},
        {{1.0, 0.0}},
        {{1.0, 1.0}},
    }};

    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts.data(), 16, verts.size());
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);

    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
    constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
    m_dataBind = ctx.newShaderDataBinding(SelectPipeline(type), m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
                                          stages.data(), nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    return true;
  } BooTrace);
}

void CColoredQuadFilter::draw(const zeus::CColor& color, const zeus::CRectangle& rect) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CColoredQuadFilter::draw", zeus::skMagenta);

  m_uniform.m_matrix[0][0] = rect.size.x() * 2.f;
  m_uniform.m_matrix[1][1] = rect.size.y() * 2.f;
  m_uniform.m_matrix[3][0] = rect.position.x() * 2.f - 1.f;
  m_uniform.m_matrix[3][1] = rect.position.y() * 2.f - 1.f;
  m_uniform.m_color = color;
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 4);
}

void CWideScreenFilter::draw(const zeus::CColor& color, float t) {
  if (g_Viewport.aspect < 1.7777f) {
    float targetHeight = g_Viewport.x8_width / 1.7777f;
    float delta = (g_Viewport.xc_height - targetHeight) * t / 2.f;
    delta /= float(g_Viewport.xc_height);
    zeus::CRectangle rect(0.f, 0.f, 1.f, delta);
    m_bottom.draw(color, rect);
    rect.position.y() = 1.f - delta;
    m_top.draw(color, rect);
  }
}

void CWideScreenFilter::DrawFilter(EFilterShape shape, const zeus::CColor& color, float t) {}

float CWideScreenFilter::SetViewportToMatch(float t) {
  if (g_Viewport.aspect < 1.7777f) {
    float targetHeight = g_Viewport.x8_width / 1.7777f;
    float delta = (g_Viewport.xc_height - targetHeight) * t / 2.f;
    boo::SWindowRect rect = {};
    rect.size[0] = g_Viewport.x8_width;
    rect.size[1] = g_Viewport.xc_height - delta * 2.f;
    rect.location[1] = delta;
    CGraphics::g_CroppedViewport = rect;
    CGraphics::g_BooMainCommandQueue->setViewport(rect);
    return 1.7777f;
  } else {
    SetViewportToFull();
    return g_Viewport.aspect;
  }
}

void CWideScreenFilter::SetViewportToFull() {
  boo::SWindowRect rect = {};
  rect.size[0] = g_Viewport.x8_width;
  rect.size[1] = g_Viewport.xc_height;
  CGraphics::g_CroppedViewport = rect;
  CGraphics::g_BooMainCommandQueue->setViewport(rect);
}

constexpr zeus::CRectangle CColoredQuadFilter::DefaultRect{0.f, 0.f, 1.f, 1.f};

} // namespace urde
