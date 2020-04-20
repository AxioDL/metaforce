#include "Runtime/Graphics/Shaders/CTextSupportShader.hpp"

#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

boo::ObjToken<boo::IShaderPipeline> CTextSupportShader::s_TextAlphaPipeline;
boo::ObjToken<boo::IShaderPipeline> CTextSupportShader::s_TextAddPipeline;
boo::ObjToken<boo::IShaderPipeline> CTextSupportShader::s_TextAddOverdrawPipeline;

boo::ObjToken<boo::IShaderPipeline> CTextSupportShader::s_ImageAlphaPipeline;
boo::ObjToken<boo::IShaderPipeline> CTextSupportShader::s_ImageAddPipeline;
boo::ObjToken<boo::IShaderPipeline> CTextSupportShader::s_ImageAddOverdrawPipeline;

hecl::VertexBufferPool<CTextSupportShader::CharacterInstance> CTextSupportShader::s_CharInsts;
hecl::VertexBufferPool<CTextSupportShader::ImageInstance> CTextSupportShader::s_ImgInsts;
hecl::UniformBufferPool<CTextSupportShader::Uniform> CTextSupportShader::s_Uniforms;

void CTextSupportShader::Initialize() {
  s_TextAlphaPipeline = hecl::conv->convert(Shader_CTextSupportShaderAlpha{});
  s_TextAddPipeline = hecl::conv->convert(Shader_CTextSupportShaderAdd{});
  s_TextAddOverdrawPipeline = hecl::conv->convert(Shader_CTextSupportShaderAddOverdraw{});
  s_ImageAlphaPipeline = hecl::conv->convert(Shader_CTextSupportShaderImageAlpha{});
  s_ImageAddPipeline = hecl::conv->convert(Shader_CTextSupportShaderImageAdd{});
  s_ImageAddOverdrawPipeline = hecl::conv->convert(Shader_CTextSupportShaderImageAddOverdraw{});
}

void CTextSupportShader::Shutdown() {
  s_TextAlphaPipeline.reset();
  s_TextAddPipeline.reset();
  s_TextAddOverdrawPipeline.reset();
  s_ImageAlphaPipeline.reset();
  s_ImageAddPipeline.reset();
  s_ImageAddOverdrawPipeline.reset();

  s_CharInsts.doDestroy();
  s_ImgInsts.doDestroy();
  s_Uniforms.doDestroy();
}

void CTextSupportShader::CharacterInstance::SetMetrics(const CGlyph& glyph, const zeus::CVector2i& offset) {
  float layer = glyph.GetLayer();

  m_pos[0].assign(offset.x, 0.f, offset.y);
  m_uv[0].assign(glyph.GetStartU(), 1.f - glyph.GetStartV(), layer);

  m_pos[1].assign(offset.x + glyph.GetCellWidth(), 0.f, offset.y);
  m_uv[1].assign(glyph.GetEndU(), 1.f - glyph.GetStartV(), layer);

  m_pos[2].assign(offset.x, 0.f, offset.y + glyph.GetCellHeight());
  m_uv[2].assign(glyph.GetStartU(), 1.f - glyph.GetEndV(), layer);

  m_pos[3].assign(offset.x + glyph.GetCellWidth(), 0.f, offset.y + glyph.GetCellHeight());
  m_uv[3].assign(glyph.GetEndU(), 1.f - glyph.GetEndV(), layer);
}

void CTextSupportShader::ImageInstance::SetMetrics(const CFontImageDef& imgDef, const zeus::CVector2i& offset) {
  zeus::CVector2f imgSize;
  if (imgDef.x4_texs.size()) {
    const CTexture& tex = *imgDef.x4_texs[0].GetObj();
    imgSize.assign(tex.GetWidth() * imgDef.x14_cropFactor.x(), tex.GetHeight() * imgDef.x14_cropFactor.y());
  }
  zeus::CVector2f cropPad = imgDef.x14_cropFactor * 0.5f;

  m_pos[0].assign(offset.x, 0.f, offset.y);
  m_uv[0].assign(0.5f - cropPad.x(), 0.5f + cropPad.y());

  m_pos[1].assign(offset.x + imgSize.x(), 0.f, offset.y);
  m_uv[1].assign(0.5f + cropPad.x(), 0.5f + cropPad.y());

  m_pos[2].assign(offset.x, 0.f, offset.y + imgSize.y());
  m_uv[2].assign(0.5f - cropPad.x(), 0.5f - cropPad.y());

  m_pos[3].assign(offset.x + imgSize.x(), 0.f, offset.y + imgSize.y());
  m_uv[3].assign(0.5f + cropPad.x(), 0.5f - cropPad.y());
}

} // namespace urde
