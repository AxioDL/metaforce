#include "Runtime/Graphics/Shaders/CTextSupportShader.hpp"

#include "hsh/hsh.h"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/GuiSys/CTextRenderBuffer.hpp"

#include "CTextSupportShader.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

constexpr hsh::sampler ClampEdgeSamp(hsh::Linear, hsh::Linear, hsh::Linear, hsh::ClampToEdge, hsh::ClampToEdge,
                                     hsh::ClampToEdge);

template <CGuiWidget::EGuiModelDrawFlags Flags>
struct DrawFlagsAttachmentExt {
  using type = BlendAttachment<>;
};
template <>
struct DrawFlagsAttachmentExt<CGuiWidget::EGuiModelDrawFlags::Additive> {
  using type = AdditiveAttachment<>;
};
template <>
struct DrawFlagsAttachmentExt<CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw> {
  using type = color_attachment<hsh::One, hsh::One, hsh::Add, hsh::One, hsh::One, hsh::Add,
                                hsh::ColorComponentFlags(hsh::CC_Red | hsh::CC_Green | hsh::CC_Blue)>;
};
template <CGuiWidget::EGuiModelDrawFlags Flags>
using DrawFlagsAttachment = typename DrawFlagsAttachmentExt<Flags>::type;

template <CGuiWidget::EGuiModelDrawFlags Flags>
struct CTextSupportShaderCharacterPipeline
: pipeline<topology<hsh::TriangleStrip>, DrawFlagsAttachment<Flags>, depth_write<false>, depth_compare<hsh::LEqual>> {
  CTextSupportShaderCharacterPipeline(hsh::vertex_buffer<CTextSupportShader::CharacterInstance> vbo,
                                      hsh::uniform_buffer<CTextSupportShader::Uniform> ubo, hsh::texture2d_array tex) {
    this->position = ubo->m_mvp * hsh::float4(vbo->m_pos[this->vertex_id], 1.f);
    hsh::float4 fontColor = ubo->m_uniformColor * vbo->m_fontColor;
    hsh::float4 outlineColor = ubo->m_uniformColor * vbo->m_outlineColor;
    hsh::float4 texel = tex.sample<float>(vbo->m_uv[this->vertex_id]);
    this->color_out[0] = (fontColor * texel.x + outlineColor * texel.y) * vbo->m_mulColor;
  }
};
template struct CTextSupportShaderCharacterPipeline<CGuiWidget::EGuiModelDrawFlags::Alpha>;
template struct CTextSupportShaderCharacterPipeline<CGuiWidget::EGuiModelDrawFlags::Additive>;
template struct CTextSupportShaderCharacterPipeline<CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw>;

template <CGuiWidget::EGuiModelDrawFlags Flags>
struct CTextSupportShaderImagePipeline
: pipeline<topology<hsh::TriangleStrip>, DrawFlagsAttachment<Flags>, depth_write<false>, depth_compare<hsh::LEqual>> {
  CTextSupportShaderImagePipeline(hsh::vertex_buffer<CTextSupportShader::ImageInstance> vbo,
                                  hsh::uniform_buffer<CTextSupportShader::Uniform> ubo, hsh::texture2d tex) {
    this->position = ubo->m_mvp * hsh::float4(vbo->m_pos[this->vertex_id], 1.f);
    // FIXME hsh bug: sampler appears to be completely ignored
    hsh::float4 texel = tex.sample<float>(vbo->m_uv[this->vertex_id], ClampEdgeSamp);
    this->color_out[0] = ubo->m_uniformColor * vbo->m_color * texel;
  }
};
template struct CTextSupportShaderImagePipeline<CGuiWidget::EGuiModelDrawFlags::Alpha>;
template struct CTextSupportShaderImagePipeline<CGuiWidget::EGuiModelDrawFlags::Additive>;
template struct CTextSupportShaderImagePipeline<CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw>;

void CTextSupportShader::CharacterInstance::SetMetrics(const CGlyph& glyph, const zeus::CVector2i& offset) {
  float layer = glyph.GetLayer();

  m_pos[0] = {float(offset.x), 0.f, float(offset.y)};
  m_uv[0] = {glyph.GetStartU(), 1.f - glyph.GetStartV(), layer};

  m_pos[1] = {float(offset.x + glyph.GetCellWidth()), 0.f, float(offset.y)};
  m_uv[1] = {glyph.GetEndU(), 1.f - glyph.GetStartV(), layer};

  m_pos[2] = {float(offset.x), 0.f, float(offset.y + glyph.GetCellHeight())};
  m_uv[2] = {glyph.GetStartU(), 1.f - glyph.GetEndV(), layer};

  m_pos[3] = {float(offset.x + glyph.GetCellWidth()), 0.f, float(offset.y + glyph.GetCellHeight())};
  m_uv[3] = {glyph.GetEndU(), 1.f - glyph.GetEndV(), layer};
}

void CTextSupportShader::ImageInstance::SetMetrics(const CFontImageDef& imgDef, const zeus::CVector2i& offset) {
  zeus::CVector2f imgSize;
  if (!imgDef.x4_texs.empty()) {
    const CTexture& tex = *imgDef.x4_texs[0].GetObj();
    imgSize = {tex.GetWidth() * imgDef.x14_cropFactor.x(), tex.GetHeight() * imgDef.x14_cropFactor.y()};
  }
  zeus::CVector2f cropPad = imgDef.x14_cropFactor * 0.5f;

  m_pos[0] = {float(offset.x), 0.f, float(offset.y)};
  m_uv[0] = {0.5f - cropPad.x(), 0.5f + cropPad.y()};

  m_pos[1] = {offset.x + imgSize.x(), 0.f, float(offset.y)};
  m_uv[1] = {0.5f + cropPad.x(), 0.5f + cropPad.y()};

  m_pos[2] = {float(offset.x), 0.f, offset.y + imgSize.y()};
  m_uv[2] = {0.5f - cropPad.x(), 0.5f - cropPad.y()};

  m_pos[3] = {offset.x + imgSize.x(), 0.f, offset.y + imgSize.y()};
  m_uv[3] = {0.5f + cropPad.x(), 0.5f - cropPad.y()};
}

static CGuiWidget::EGuiModelDrawFlags ResolveFlags(CGuiWidget::EGuiModelDrawFlags flags) {
  switch (flags) {
  case CGuiWidget::EGuiModelDrawFlags::Shadeless:
  case CGuiWidget::EGuiModelDrawFlags::Opaque:
  case CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw:
    return CGuiWidget::EGuiModelDrawFlags::Alpha;
  default:
    return flags;
  }
}

void CTextSupportShader::BuildCharacterShaderBinding(CTextRenderBuffer& buf, BooFontCharacters& chs,
                                                     CGuiWidget::EGuiModelDrawFlags flags) {
  chs.m_instBuf = hsh::create_dynamic_vertex_buffer<CTextSupportShader::CharacterInstance>(chs.m_charCount);
  hsh::texture2d_array tex = chs.m_font->GetTexture();
  if (flags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw) {
    chs.m_dataBindingOverdraw.hsh_char_overdraw_bind(
        CTextSupportShaderCharacterPipeline<flags>(chs.m_instBuf.get(), buf.m_uniBuf2.get(), tex));
  }
  chs.m_dataBinding.hsh_char_bind(
      CTextSupportShaderCharacterPipeline<ResolveFlags(flags)>(chs.m_instBuf.get(), buf.m_uniBuf.get(), tex));
}

void CTextSupportShader::BuildImageShaderBinding(CTextRenderBuffer& buf, BooImage& img,
                                                 CGuiWidget::EGuiModelDrawFlags flags) {
  img.m_instBuf = hsh::create_dynamic_vertex_buffer<CTextSupportShader::ImageInstance>(1);
  if (flags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw) {
    for (TToken<CTexture>& token : img.m_imageDef.x4_texs) {
      hsh::texture2d tex = token->GetBooTexture();
      img.m_dataBindingOverdraw.emplace_back().hsh_img_overdraw_bind(
          CTextSupportShaderImagePipeline<flags>(img.m_instBuf.get(), buf.m_uniBuf2.get(), tex));
    }
  }
  flags = ResolveFlags(flags);
  for (TToken<CTexture>& token : img.m_imageDef.x4_texs) {
    hsh::texture2d tex = token->GetBooTexture();
    img.m_dataBinding.emplace_back().hsh_img_bind(
        CTextSupportShaderImagePipeline<flags>(img.m_instBuf.get(), buf.m_uniBuf.get(), tex));
  }
}

} // namespace urde
