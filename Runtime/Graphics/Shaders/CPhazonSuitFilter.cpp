#include "CPhazonSuitFilter.hpp"
#include "hecl/Pipeline.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CTexture.hpp"

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_IndPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;
static boo::ObjToken<boo::IShaderPipeline> s_BlurPipeline;

void CPhazonSuitFilter::Initialize() {
  s_IndPipeline = hecl::conv->convert(Shader_CPhazonSuitFilterInd{});
  s_Pipeline = hecl::conv->convert(Shader_CPhazonSuitFilterNoInd{});
  s_BlurPipeline = hecl::conv->convert(Shader_CPhazonSuitFilterBlur{});
}

void CPhazonSuitFilter::Shutdown() {
  s_IndPipeline.reset();
  s_Pipeline.reset();
  s_BlurPipeline.reset();
}

#define BLUR_SCALE (1.f / 128.f)

void CPhazonSuitFilter::drawBlurPasses(float radius, const CTexture* indTex) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CPhazonSuitFilter::drawBlurPasses", zeus::skMagenta);
  if (!m_dataBind || indTex != m_indTex) {
    m_indTex = indTex;
    CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
      m_uniBufBlurX = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CVector4f), 1);
      m_uniBufBlurY = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CVector4f), 1);
      m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CVector4f) * 2, 1);

      struct BlurVert {
        zeus::CVector3f pos;
        zeus::CVector2f uv;
      } blurVerts[4] = {{{-1.f, 1.f, 0.f}, {0.f, 1.f}},
                        {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
                        {{1.f, 1.f, 0.f}, {1.f, 1.f}},
                        {{1.f, -1.f, 0.f}, {1.f, 0.f}}};
      m_blurVbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, blurVerts, sizeof(BlurVert), 4);

      struct Vert {
        zeus::CVector3f pos;
        zeus::CVector2f screenUv;
        zeus::CVector2f indUv;
        zeus::CVector2f maskUv;
      } verts[4] = {{{-1.f, 1.f, 0.f}, {0.01f, 0.99f}, {0.f, 4.f}, {0.f, 1.f}},
                    {{-1.f, -1.f, 0.f}, {0.01f, 0.01f}, {0.f, 0.f}, {0.f, 0.f}},
                    {{1.f, 1.f, 0.f}, {0.99f, 0.99f}, {g_Viewport.aspect * 4.f, 4.f}, {1.f, 1.f}},
                    {{1.f, -1.f, 0.f}, {0.99f, 0.01f}, {g_Viewport.aspect * 4.f, 0.f}, {1.f, 0.f}}};
      m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, sizeof(Vert), 4);

      boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBufBlurX.get()};
      boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
      boo::ObjToken<boo::ITexture> texs[4];
      int texBindIdxs[4];

      texs[0] = CGraphics::g_SpareTexture.get();
      texBindIdxs[0] = 1;
      m_dataBindBlurX = ctx.newShaderDataBinding(s_BlurPipeline, m_blurVbo.get(), nullptr, nullptr, 1, bufs, stages,
                                                 nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

      bufs[0] = m_uniBufBlurY.get();
      texs[0] = CGraphics::g_SpareTexture.get();
      texBindIdxs[0] = 2;
      m_dataBindBlurY = ctx.newShaderDataBinding(s_BlurPipeline, m_blurVbo.get(), nullptr, nullptr, 1, bufs, stages,
                                                 nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

      bufs[0] = m_uniBuf.get();
      size_t texCount;
      if (m_indTex) {
        texs[0] = CGraphics::g_SpareTexture.get();
        texBindIdxs[0] = 0;
        texs[1] = m_indTex->GetBooTexture();
        texBindIdxs[1] = 0;
        texs[2] = CGraphics::g_SpareTexture.get();
        texBindIdxs[2] = 1;
        texs[3] = CGraphics::g_SpareTexture.get();
        texBindIdxs[3] = 2;
        texCount = 4;
      } else {
        texs[0] = CGraphics::g_SpareTexture.get();
        texBindIdxs[0] = 0;
        texs[1] = CGraphics::g_SpareTexture.get();
        texBindIdxs[1] = 1;
        texs[2] = CGraphics::g_SpareTexture.get();
        texBindIdxs[2] = 2;
        texCount = 3;
      }

      m_dataBind = ctx.newShaderDataBinding(m_indTex ? s_IndPipeline : s_Pipeline, m_vbo.get(), nullptr, nullptr, 1,
                                            bufs, stages, nullptr, nullptr, texCount, texs, texBindIdxs, nullptr);
      return true;
    } BooTrace);
  }

  SClipScreenRect rect;
  rect.x4_left = g_Viewport.x0_left;
  rect.x8_top = g_Viewport.x4_top;
  rect.xc_width = g_Viewport.x8_width;
  rect.x10_height = g_Viewport.xc_height;

  /* X Pass */
  zeus::CVector4f blurDir =
      zeus::CVector4f{g_Viewport.xc_height / float(g_Viewport.x8_width) * radius * BLUR_SCALE, 0.f, 0.f, 0.f};
  m_uniBufBlurX->load(&blurDir, sizeof(zeus::CVector4f));

  CGraphics::SetShaderDataBinding(m_dataBindBlurX);
  CGraphics::DrawArray(0, 4);
  CGraphics::ResolveSpareTexture(rect, 2);

  /* Y Pass */
  blurDir = zeus::CVector4f{0.f, radius * BLUR_SCALE, 0.f, 0.f};
  m_uniBufBlurY->load(&blurDir, sizeof(zeus::CVector4f));

  CGraphics::SetShaderDataBinding(m_dataBindBlurY);
  CGraphics::DrawArray(0, 4);
  CGraphics::ResolveSpareTexture(rect, 2);
}

void CPhazonSuitFilter::draw(const zeus::CColor& color, float indScale, float indOffX, float indOffY) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CPhazonSuitFilter::draw", zeus::skMagenta);
  struct Uniform {
    zeus::CColor color;
    zeus::CVector4f indScaleOff;
  } uniform = {color, zeus::CVector4f(indScale, indScale, indOffX, indOffY)};

  m_uniBuf->load(&uniform, sizeof(Uniform));
  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 4);
}

} // namespace urde
