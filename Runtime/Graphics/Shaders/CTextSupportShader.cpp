#include "CTextSupportShader.hpp"
#include "GuiSys/CRasterFont.hpp"

namespace urde
{

boo::IVertexFormat* CTextSupportShader::s_TextVtxFmt = nullptr;
boo::IShaderPipeline* CTextSupportShader::s_TextAlphaPipeline = nullptr;
boo::IShaderPipeline* CTextSupportShader::s_TextAddPipeline = nullptr;

boo::IVertexFormat* CTextSupportShader::s_ImageVtxFmt = nullptr;
boo::IShaderPipeline* CTextSupportShader::s_ImageAlphaPipeline = nullptr;
boo::IShaderPipeline* CTextSupportShader::s_ImageAddPipeline = nullptr;

hecl::VertexBufferPool<CTextSupportShader::CharacterInstance> CTextSupportShader::s_CharInsts;
hecl::VertexBufferPool<CTextSupportShader::ImageInstance> CTextSupportShader::s_ImgInsts;
hecl::UniformBufferPool<CTextSupportShader::Uniform> CTextSupportShader::s_Uniforms;

void CTextSupportShader::CharacterInstance::SetMetrics(const CGlyph& glyph,
                                                       const zeus::CVector2i& offset)
{
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

void CTextSupportShader::ImageInstance::SetMetrics(const zeus::CVector2f& imgSize,
                                                   const zeus::CVector2i& offset)
{
    m_pos[0].assign(offset.x, 0.f, offset.y);
    m_uv[0].assign(0.f, 1.f);

    m_pos[1].assign(offset.x + imgSize.x, 0.f, offset.y);
    m_uv[1].assign(1.f, 1.f);

    m_pos[2].assign(offset.x, 0.f, offset.y + imgSize.y);
    m_uv[2].assign(0.f, 0.f);

    m_pos[3].assign(offset.x + imgSize.x, 0.f, offset.y + imgSize.y);
    m_uv[3].assign(1.f, 0.f);
}

void CTextSupportShader::Shutdown()
{
    s_CharInsts.doDestroy();
    s_ImgInsts.doDestroy();
    s_Uniforms.doDestroy();
}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CTextSupportShader)

}
