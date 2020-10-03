#include "Runtime/Graphics/Shaders/CPhazonSuitFilter.hpp"

#include <array>

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include "CPhazonSuitFilter.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <bool NoIndTex>
struct CPhazonSuitFilterPipeline : pipeline<topology<hsh::TriangleStrip>, AdditiveAttachment<>, depth_write<false>> {
  CPhazonSuitFilterPipeline(hsh::vertex_buffer<CPhazonSuitFilter::Vert> vbo,
                            hsh::uniform_buffer<CPhazonSuitFilter::Uniform> ubo, hsh::render_texture2d screenTex,
                            hsh::texture2d indTex, hsh::render_texture2d maskTex, hsh::render_texture2d maskTexBlur) {
    this->position = hsh::float4(vbo->pos, 1.f);
    hsh::float2 maskUv = vbo->maskUv;
    maskUv.y = 1.f - maskUv.y;
    hsh::float2 screenUv = vbo->screenUv;
    screenUv.y = 1.f - screenUv.y;
    float maskBlurAlpha = hsh::saturate((maskTexBlur.sample<float>(maskUv).w - maskTex.sample<float>(maskUv).w) * 2.f);
    if constexpr (NoIndTex) {
      this->color_out[0] =
          hsh::float4((ubo->color * screenTex.sample<float>(screenUv) * maskBlurAlpha).xyz(), ubo->color.w);
    } else {
      hsh::float2 indUv =
          (indTex.sample<float>(vbo->indUv).xy() - hsh::float2(0.5f)) * ubo->indScaleOff.xy() + ubo->indScaleOff.zw();
      this->color_out[0] =
          hsh::float4((ubo->color * screenTex.sample<float>(indUv + screenUv) * maskBlurAlpha).xyz(), ubo->color.w);
    }
  }
};
template struct CPhazonSuitFilterPipeline<true>;
template struct CPhazonSuitFilterPipeline<false>;

struct CPhazonSuitFilterBlurPipeline
: pipeline<color_attachment<hsh::One, hsh::Zero, hsh::Add, hsh::One, hsh::Zero, hsh::Add, hsh::CC_Alpha>,
           depth_write<false>> {
  CPhazonSuitFilterBlurPipeline(hsh::vertex_buffer<CPhazonSuitFilter::BlurVert> vbo,
                                hsh::uniform_buffer<CPhazonSuitFilter::BlurUniform> ubo,
                                hsh::render_texture2d maskTex) {
    this->position = hsh::float4(vbo->pos, 1.f);

    // this will be our alpha sum
    float sum = 0.f;

    // apply blurring, using a 23-tap filter with predefined gaussian weights
    hsh::float2 blurDir = ubo->blur.xy();
    hsh::float2 uv = vbo->uv;
    uv.y = 1.f - uv.y;
    sum += maskTex.sample<float>(uv + hsh::float2(-11.f) * blurDir).w * 0.007249f;
    sum += maskTex.sample<float>(uv + hsh::float2(-10.f) * blurDir).w * 0.011032f;
    sum += maskTex.sample<float>(uv + hsh::float2(-9.f) * blurDir).w * 0.016133f;
    sum += maskTex.sample<float>(uv + hsh::float2(-8.f) * blurDir).w * 0.022665f;
    sum += maskTex.sample<float>(uv + hsh::float2(-7.f) * blurDir).w * 0.030595f;
    sum += maskTex.sample<float>(uv + hsh::float2(-6.f) * blurDir).w * 0.039680f;
    sum += maskTex.sample<float>(uv + hsh::float2(-5.f) * blurDir).w * 0.049444f;
    sum += maskTex.sample<float>(uv + hsh::float2(-4.f) * blurDir).w * 0.059195f;
    sum += maskTex.sample<float>(uv + hsh::float2(-3.f) * blurDir).w * 0.068091f;
    sum += maskTex.sample<float>(uv + hsh::float2(-2.f) * blurDir).w * 0.075252f;
    sum += maskTex.sample<float>(uv + hsh::float2(-1.f) * blurDir).w * 0.079905f;
    sum += maskTex.sample<float>(uv).w * 0.081519f;
    sum += maskTex.sample<float>(uv + hsh::float2(1.f) * blurDir).w * 0.079905f;
    sum += maskTex.sample<float>(uv + hsh::float2(2.f) * blurDir).w * 0.075252f;
    sum += maskTex.sample<float>(uv + hsh::float2(3.f) * blurDir).w * 0.068091f;
    sum += maskTex.sample<float>(uv + hsh::float2(4.f) * blurDir).w * 0.059195f;
    sum += maskTex.sample<float>(uv + hsh::float2(5.f) * blurDir).w * 0.049444f;
    sum += maskTex.sample<float>(uv + hsh::float2(6.f) * blurDir).w * 0.039680f;
    sum += maskTex.sample<float>(uv + hsh::float2(7.f) * blurDir).w * 0.030595f;
    sum += maskTex.sample<float>(uv + hsh::float2(8.f) * blurDir).w * 0.022665f;
    sum += maskTex.sample<float>(uv + hsh::float2(9.f) * blurDir).w * 0.016133f;
    sum += maskTex.sample<float>(uv + hsh::float2(10.f) * blurDir).w * 0.011032f;
    sum += maskTex.sample<float>(uv + hsh::float2(11.f) * blurDir).w * 0.007249f;

    this->color_out[0] = hsh::float4(1.f, 1.f, 1.f, sum);
  }
};

void CPhazonSuitFilter::drawBlurPasses(float radius, const CTexture* indTex) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CPhazonSuitFilter::drawBlurPasses", zeus::skMagenta);
  if (!m_dataBind || indTex != m_indTex) {
    m_indTex = indTex;

    m_uniBufBlurX = hsh::create_dynamic_uniform_buffer<BlurUniform>();
    m_uniBufBlurY = hsh::create_dynamic_uniform_buffer<BlurUniform>();
    m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();

    constexpr std::array<BlurVert, 4> blurVerts{{
        {{-1.f, 1.f, 0.f}, {0.f, 1.f}},
        {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
        {{1.f, 1.f, 0.f}, {1.f, 1.f}},
        {{1.f, -1.f, 0.f}, {1.f, 0.f}},
    }};
    m_blurVbo = hsh::create_vertex_buffer(blurVerts);

    m_vbo = hsh::create_vertex_buffer(std::array<Vert, 4>{{
        {{-1.f, 1.f, 0.f}, {0.01f, 0.99f}, {0.f, 4.f}, {0.f, 1.f}},
        {{-1.f, -1.f, 0.f}, {0.01f, 0.01f}, {0.f, 0.f}, {0.f, 0.f}},
        {{1.f, 1.f, 0.f}, {0.99f, 0.99f}, {g_Viewport.aspect * 4.f, 4.f}, {1.f, 1.f}},
        {{1.f, -1.f, 0.f}, {0.99f, 0.01f}, {g_Viewport.aspect * 4.f, 0.f}, {1.f, 0.f}},
    }});

    m_dataBindBlurX.hsh_blurx_bind(
        CPhazonSuitFilterBlurPipeline(m_blurVbo.get(), m_uniBufBlurX.get(), CGraphics::g_SpareTexture.get_color(1)));
    m_dataBindBlurY.hsh_blury_bind(
        CPhazonSuitFilterBlurPipeline(m_blurVbo.get(), m_uniBufBlurY.get(), CGraphics::g_SpareTexture.get_color(2)));

    bool hasIndTex = m_indTex != nullptr;
    if (hasIndTex) {
      hsh::texture2d tex = m_indTex->GetBooTexture();
      m_dataBind.hsh_ind_bind(CPhazonSuitFilterPipeline<hasIndTex>(
          m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_color(0), tex,
          CGraphics::g_SpareTexture.get_color(1), CGraphics::g_SpareTexture.get_color(2)));
    } else {
      m_dataBind.hsh_noind_bind(CPhazonSuitFilterPipeline<hasIndTex>(
          m_vbo.get(), m_uniBuf.get(), CGraphics::g_SpareTexture.get_color(0), hsh::texture2d{},
          CGraphics::g_SpareTexture.get_color(1), CGraphics::g_SpareTexture.get_color(2)));
    }
  }

  SClipScreenRect rect;
  rect.x4_left = g_Viewport.x0_left;
  rect.x8_top = g_Viewport.x4_top;
  rect.xc_width = g_Viewport.x8_width;
  rect.x10_height = g_Viewport.xc_height;

  constexpr float blurScale = 1.0f / 128.0f;

  /* X Pass */
  m_uniBufBlurX.load(
      {zeus::CVector4f{g_Viewport.xc_height / float(g_Viewport.x8_width) * radius * blurScale, 0.f, 0.f, 0.f}});
  m_dataBindBlurX.draw(0, 4);
  CGraphics::ResolveSpareTexture(rect, 2);

  /* Y Pass */
  m_uniBufBlurY.load({zeus::CVector4f{0.f, radius * blurScale, 0.f, 0.f}});
  m_dataBindBlurY.draw(0, 4);
  CGraphics::ResolveSpareTexture(rect, 2);
}

void CPhazonSuitFilter::draw(const zeus::CColor& color, float indScale, float indOffX, float indOffY) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CPhazonSuitFilter::draw", zeus::skMagenta);
  m_uniBuf.load({color, zeus::CVector4f(indScale, indScale, indOffX, indOffY)});
  m_dataBind.draw(0, 4);
}

} // namespace urde
