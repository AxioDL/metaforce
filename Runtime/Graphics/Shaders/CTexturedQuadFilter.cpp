#include "Runtime/Graphics/Shaders/CTexturedQuadFilter.hpp"

#include <array>

#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include "CTexturedQuadFilter.cpp.hshhead"

namespace urde {

template <EFilterType Type, bool TexAlpha>
struct CTexturedQuadFilterPipeline : FilterPipeline<Type> {
  CTexturedQuadFilterPipeline(hsh::vertex_buffer<CTexturedQuadFilter::Vert> vbo,
                              hsh::uniform_buffer<CTexturedQuadFilter::Uniform> uniBuf, hsh::texture2d tex) {
    this->position = hsh::float4(vbo->m_pos, 1.f);
    if constexpr (TexAlpha) {
      this->color_out[0] = uniBuf->m_color * tex.sample_bias<float>(vbo->m_uv, uniBuf->m_lod, {});
    } else {
      this->color_out[0] =
          uniBuf->m_color * hsh::float4(tex.sample_bias<float>(vbo->m_uv, uniBuf->m_lod, {}).xyz(), 1.f);
    }
  }
};

template <typename Tex>
CTexturedQuadFilter::CTexturedQuadFilter(EFilterType type, Tex tex, ZTest ztest)
: m_zTest(ztest) {
  m_vbo = hsh::create_dynamic_vertex_buffer<Vert>(16);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind(CTexturedQuadFilterPipeline<type, false>(m_vbo.get(), m_uniBuf.get(), tex));
}

CTexturedQuadFilter::CTexturedQuadFilter(EFilterType type, TLockedToken<CTexture> tex, ZTest ztest)
: CTexturedQuadFilter(type, (tex ? tex->GetBooTexture() : hsh::texture2d{}), ztest) {
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
  m_vbo.load(verts);

  m_uniform.m_matrix[0][0] = rect.size.x() * 2.f;
  m_uniform.m_matrix[1][1] = rect.size.y() * 2.f;
  m_uniform.m_matrix[3][0] = rect.position.x() * 2.f - 1.f;
  m_uniform.m_matrix[3][1] = rect.position.y() * 2.f - 1.f;
  m_uniform.m_color = color;
  m_uniBuf.load(m_uniform);

  m_dataBind.draw(0, 4);
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
  m_vbo.load(verts);

  m_uniform.m_color = color;
  m_uniBuf.load(m_uniform);

  m_dataBind.draw(0, 4);
}

void CTexturedQuadFilter::drawVerts(const zeus::CColor& color, const std::array<Vert, 4>& verts, float lod) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CTexturedQuadFilter::drawVerts", zeus::skMagenta);

  m_vbo.load(verts);

  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = color;
  m_uniform.m_lod = lod;
  m_uniBuf.load(m_uniform);

  m_dataBind.draw(0, 4);
}

void CTexturedQuadFilter::DrawFilter(EFilterShape shape, const zeus::CColor& color, float t) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CTexturedQuadFilter::DrawFilter", zeus::skMagenta);

  m_uniform.m_matrix = zeus::CMatrix4f();
  m_uniform.m_lod = 0.f;
  m_uniform.m_color = color;
  m_uniBuf.load(m_uniform);

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
    m_vbo.load(QuadVerts);
    m_dataBind.draw(0, 4);
    m_dataBind.draw(4, 4);
    m_dataBind.draw(8, 4);
    m_dataBind.draw(12, 4);
  } else {
    const std::array<Vert, 4> FullscreenVerts{{
        {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
        {{-1.f, 1.f, 0.f}, {0.f, t}},
        {{1.f, -1.f, 0.f}, {t, 0.f}},
        {{1.f, 1.f, 0.f}, {t, t}},
    }};
    m_vbo.load(FullscreenVerts);
    m_dataBind.draw(0, FullscreenVerts.size());
  }
}

template <typename Tex>
CTexturedQuadFilterAlpha::CTexturedQuadFilterAlpha(EFilterType type, Tex tex) {
  m_vbo = hsh::create_dynamic_vertex_buffer<Vert>(16);
  m_uniBuf = hsh::create_dynamic_uniform_buffer<Uniform>();
  m_dataBind.hsh_bind_alpha(CTexturedQuadFilterPipeline<type, true>(m_vbo.get(), m_uniBuf.get(), tex));
}

CTexturedQuadFilterAlpha::CTexturedQuadFilterAlpha(EFilterType type, TLockedToken<CTexture> tex)
: CTexturedQuadFilterAlpha(type, (tex ? tex->GetBooTexture() : hsh::texture2d{})) {
  m_tex = tex;
}

} // namespace urde
