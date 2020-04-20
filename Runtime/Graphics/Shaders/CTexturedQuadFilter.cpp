#include "Runtime/Graphics/Shaders/CTexturedQuadFilter.hpp"

#include <array>

#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaGEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaGEqualZWritePipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaLEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddGEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddGEqualZWritePipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddLEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_SubtractPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_SubtractGEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_SubtractGEqualZWritePipeline;
static boo::ObjToken<boo::IShaderPipeline> s_SubtractLEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultGEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultGEqualZWritePipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultLEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_InvDstMultPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_InvDstMultGEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_InvDstMultLEqualPipeline;

static boo::ObjToken<boo::IShaderPipeline> s_AAlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AAddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_ASubtractPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AMultPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AInvDstMultPipeline;

void CTexturedQuadFilter::Initialize() {
  s_AlphaPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlpha{});
  s_AlphaGEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlphaGEqual{});
  s_AlphaGEqualZWritePipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlphaGEqualZWrite{});
  s_AlphaLEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlphaLEqual{});
  s_AddPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAdd{});
  s_AddGEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAddGEqual{});
  s_AddGEqualZWritePipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAddGEqualZWrite{});
  s_AddLEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAddLEqual{});
  s_SubtractPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterSubtract{});
  s_SubtractGEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterSubtractGEqual{});
  s_SubtractGEqualZWritePipeline = hecl::conv->convert(Shader_CTexturedQuadFilterSubtractGEqualZWrite{});
  s_SubtractLEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterSubtractLEqual{});
  s_MultPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterMult{});
  s_MultGEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterMultGEqual{});
  s_MultGEqualZWritePipeline = hecl::conv->convert(Shader_CTexturedQuadFilterMultGEqualZWrite{});
  s_MultLEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterMultLEqual{});
  s_InvDstMultPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterInvDstMult{});
  s_InvDstMultGEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterInvDstMultGEqual{});
  s_InvDstMultLEqualPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterInvDstMultLEqual{});
}

void CTexturedQuadFilter::Shutdown() {
  s_AlphaPipeline.reset();
  s_AlphaGEqualPipeline.reset();
  s_AlphaGEqualZWritePipeline.reset();
  s_AlphaLEqualPipeline.reset();
  s_AddPipeline.reset();
  s_AddGEqualPipeline.reset();
  s_AddGEqualZWritePipeline.reset();
  s_AddLEqualPipeline.reset();
  s_SubtractPipeline.reset();
  s_SubtractGEqualPipeline.reset();
  s_SubtractGEqualZWritePipeline.reset();
  s_SubtractLEqualPipeline.reset();
  s_MultPipeline.reset();
  s_MultGEqualPipeline.reset();
  s_MultGEqualZWritePipeline.reset();
  s_MultLEqualPipeline.reset();
  s_InvDstMultPipeline.reset();
  s_InvDstMultGEqualPipeline.reset();
  s_InvDstMultLEqualPipeline.reset();
}

void CTexturedQuadFilterAlpha::Initialize() {
  s_AAlphaPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlphaTexAlpha{});
  s_AAddPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlphaTexAdd{});
  s_ASubtractPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlphaTexSubtract{});
  s_AMultPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlphaTexMult{});
  s_AInvDstMultPipeline = hecl::conv->convert(Shader_CTexturedQuadFilterAlphaTexInvDstMult{});
}

void CTexturedQuadFilterAlpha::Shutdown() {
  s_AAlphaPipeline.reset();
  s_AAddPipeline.reset();
  s_ASubtractPipeline.reset();
  s_AMultPipeline.reset();
  s_AInvDstMultPipeline.reset();
}

static boo::ObjToken<boo::IShaderPipeline> SelectPipeline(EFilterType type, CTexturedQuadFilter::ZTest zTest) {
  switch (zTest) {
  case CTexturedQuadFilter::ZTest::GEqual:
    switch (type) {
    case EFilterType::Blend:
      return s_AlphaGEqualPipeline;
    case EFilterType::Add:
      return s_AddGEqualPipeline;
    case EFilterType::Subtract:
      return s_SubtractGEqualPipeline;
    case EFilterType::Multiply:
      return s_MultGEqualPipeline;
    default:
      break;
    }
    break;
  case CTexturedQuadFilter::ZTest::GEqualZWrite:
    switch (type) {
    case EFilterType::Blend:
      return s_AlphaGEqualZWritePipeline;
    case EFilterType::Add:
      return s_AddGEqualZWritePipeline;
    case EFilterType::Subtract:
      return s_SubtractGEqualZWritePipeline;
    case EFilterType::Multiply:
      return s_MultGEqualZWritePipeline;
    default:
      break;
    }
    break;
  case CTexturedQuadFilter::ZTest::LEqual:
    switch (type) {
    case EFilterType::Blend:
      return s_AlphaLEqualPipeline;
    case EFilterType::Add:
      return s_AddLEqualPipeline;
    case EFilterType::Subtract:
      return s_SubtractLEqualPipeline;
    case EFilterType::Multiply:
      return s_MultLEqualPipeline;
    case EFilterType::InvDstMultiply:
      return s_InvDstMultLEqualPipeline;
    default:
      break;
    }
    break;
  default:
    break;
  }

  switch (type) {
  case EFilterType::Blend:
    return s_AlphaPipeline;
  case EFilterType::Add:
    return s_AddPipeline;
  case EFilterType::Subtract:
    return s_SubtractPipeline;
  case EFilterType::Multiply:
    return s_MultPipeline;
  case EFilterType::InvDstMultiply:
    return s_InvDstMultPipeline;
  default:
    return {};
  }
}

static boo::ObjToken<boo::IShaderPipeline> SelectAlphaPipeline(EFilterType type) {
  switch (type) {
  case EFilterType::Blend:
    return s_AAlphaPipeline;
  case EFilterType::Add:
    return s_AAddPipeline;
  case EFilterType::Subtract:
    return s_ASubtractPipeline;
  case EFilterType::Multiply:
    return s_AMultPipeline;
  case EFilterType::InvDstMultiply:
    return s_AInvDstMultPipeline;
  default:
    return {};
  }
}

CTexturedQuadFilter::CTexturedQuadFilter(const boo::ObjToken<boo::ITexture>& tex) : m_booTex(tex) {
  m_flipRect = CGraphics::g_BooFactory->platform() == boo::IGraphicsDataFactory::Platform::Vulkan;
}

CTexturedQuadFilter::CTexturedQuadFilter(EFilterType type, const boo::ObjToken<boo::ITexture>& tex, ZTest ztest)
: m_booTex(tex), m_zTest(ztest) {
  m_flipRect = CGraphics::g_BooFactory->platform() == boo::IGraphicsDataFactory::Platform::Vulkan;
  tex->setClampMode(boo::TextureClampMode::ClampToEdge);
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 16);
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);

    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
    const std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
    const std::array<boo::ObjToken<boo::ITexture>, 1> texs{m_booTex.get()};
    m_dataBind =
        ctx.newShaderDataBinding(SelectPipeline(type, m_zTest), m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
                                 stages.data(), nullptr, nullptr, texs.size(), texs.data(), nullptr, nullptr);
    return true;
  } BooTrace);
}

CTexturedQuadFilter::CTexturedQuadFilter(EFilterType type, TLockedToken<CTexture> tex, ZTest ztest)
: CTexturedQuadFilter(type, (tex ? tex->GetBooTexture() : nullptr), ztest) {
  m_flipRect = CGraphics::g_BooFactory->platform() == boo::IGraphicsDataFactory::Platform::Vulkan;
  m_tex = tex;
}

void CTexturedQuadFilter::draw(const zeus::CColor& color, float uvScale, const zeus::CRectangle& rect, float z) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CTexturedQuadFilter::draw", zeus::skMagenta);

  const std::array<Vert, 4> verts{{
      {{0.f, 0.f, z}, {0.f, 0.f}},
      {{0.f, 1.f, z}, {0.f, uvScale}},
      {{1.f, 0.f, z}, {uvScale, 0.f}},
      {{1.f, 1.f, z}, {uvScale, uvScale}},
  }};
  m_vbo->load(verts.data(), sizeof(verts));

  if (!m_flipRect) {
    m_uniform.m_matrix[0][0] = rect.size.x() * 2.f;
    m_uniform.m_matrix[1][1] = rect.size.y() * 2.f;
    m_uniform.m_matrix[3][0] = rect.position.x() * 2.f - 1.f;
    m_uniform.m_matrix[3][1] = rect.position.y() * 2.f - 1.f;
  } else {
    m_uniform.m_matrix[0][0] = rect.size.x() * 2.f;
    m_uniform.m_matrix[1][1] = rect.size.y() * -2.f;
    m_uniform.m_matrix[3][0] = rect.position.x() * 2.f - 1.f;
    m_uniform.m_matrix[3][1] = rect.position.y() * -2.f + 1.f;
  }
  m_uniform.m_color = color;
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 4);
}

void CTexturedQuadFilter::drawCropped(const zeus::CColor& color, float uvScale) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CTexturedQuadFilter::drawCropped", zeus::skMagenta);

  const float xFac = CGraphics::g_CroppedViewport.xc_width / float(g_Viewport.x8_width);
  const float yFac = CGraphics::g_CroppedViewport.x10_height / float(g_Viewport.xc_height);
  const float xBias = CGraphics::g_CroppedViewport.x4_left / float(g_Viewport.x8_width);
  const float yBias = CGraphics::g_CroppedViewport.x8_top / float(g_Viewport.xc_height);

  const std::array<Vert, 4> verts{{
      {{-1.f, -1.f, 0.f}, {xBias * uvScale, yBias * uvScale}},
      {{-1.f, 1.f, 0.f}, {xBias * uvScale, (yBias + yFac) * uvScale}},
      {{1.f, -1.f, 0.f}, {(xBias + xFac) * uvScale, yBias * uvScale}},
      {{1.f, 1.f, 0.f}, {(xBias + xFac) * uvScale, (yBias + yFac) * uvScale}},
  }};
  m_vbo->load(verts.data(), sizeof(verts));

  m_uniform.m_color = color;
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 4);
}

void CTexturedQuadFilter::drawVerts(const zeus::CColor& color, std::array<Vert, 4> verts, float lod) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CTexturedQuadFilter::drawVerts", zeus::skMagenta);

  m_vbo->load(verts.data(), sizeof(Vert) * verts.size());

  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = color;
  m_uniform.m_lod = lod;
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, verts.size());
}

void CTexturedQuadFilter::DrawFilter(EFilterShape shape, const zeus::CColor& color, float t) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CTexturedQuadFilter::DrawFilter", zeus::skMagenta);

  m_uniform.m_matrix = zeus::CMatrix4f();
  m_uniform.m_lod = 0.f;
  m_uniform.m_color = color;
  m_uniBuf->load(&m_uniform, sizeof(m_uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);

  if (shape == EFilterShape::FullscreenQuarters) {
    const std::array<Vert, 16> QuadVerts{{
        // ll
        {{-1.f, -1.f, 0.f}, {t, t}},
        {{-1.f, 0.f, 0.f}, {t, 0.f}},
        {{0.f, -1.f, 0.f}, {0.f, t}},
        {{0.f, 0.f, 0.f}, {0.f, 0.f}},
        // ul
        {{-1.f, 1.f, 0.f}, {t, t}},
        {{-1.f, 0.f, 0.f}, {t, 0.f}},
        {{0.f, 1.f, 0.f}, {0.f, t}},
        {{0.f, 0.f, 0.f}, {0.f, 0.f}},
        // lr
        {{1.f, -1.f, 0.f}, {t, t}},
        {{1.f, 0.f, 0.f}, {t, 0.f}},
        {{0.f, -1.f, 0.f}, {0.f, t}},
        {{0.f, 0.f, 0.f}, {0.f, 0.f}},
        // ur
        {{1.f, 1.f, 0.f}, {t, t}},
        {{1.f, 0.f, 0.f}, {t, 0.f}},
        {{0.f, 1.f, 0.f}, {0.f, t}},
        {{0.f, 0.f, 0.f}, {0.f, 0.f}},
    }};
    m_vbo->load(QuadVerts.data(), sizeof(QuadVerts));
    CGraphics::DrawArray(0, 4);
    CGraphics::DrawArray(4, 4);
    CGraphics::DrawArray(8, 4);
    CGraphics::DrawArray(12, 4);
  } else {
    const std::array<Vert, 4> FullscreenVerts{{
        {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
        {{-1.f, 1.f, 0.f}, {0.f, t}},
        {{1.f, -1.f, 0.f}, {t, 0.f}},
        {{1.f, 1.f, 0.f}, {t, t}},
    }};
    m_vbo->load(FullscreenVerts.data(), sizeof(FullscreenVerts));
    CGraphics::DrawArray(0, FullscreenVerts.size());
  }
}

constexpr zeus::CRectangle CTexturedQuadFilter::DefaultRect{0.f, 0.f, 1.f, 1.f};

CTexturedQuadFilterAlpha::CTexturedQuadFilterAlpha(EFilterType type, const boo::ObjToken<boo::ITexture>& tex)
: CTexturedQuadFilter(tex) {
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 4);
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);

    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
    constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
    const std::array<boo::ObjToken<boo::ITexture>, 1> texs{m_booTex.get()};
    m_dataBind =
        ctx.newShaderDataBinding(SelectAlphaPipeline(type), m_vbo.get(), nullptr, nullptr, bufs.size(), bufs.data(),
                                 stages.data(), nullptr, nullptr, texs.size(), texs.data(), nullptr, nullptr);
    return true;
  } BooTrace);
}

CTexturedQuadFilterAlpha::CTexturedQuadFilterAlpha(EFilterType type, TLockedToken<CTexture> tex)
: CTexturedQuadFilterAlpha(type, (tex ? tex->GetBooTexture() : nullptr)) {
  m_tex = tex;
}

} // namespace urde
