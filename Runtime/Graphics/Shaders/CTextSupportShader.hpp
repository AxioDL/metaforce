#ifndef __URDE_CTEXTSUPPORTSHADER_HPP__
#define __URDE_CTEXTSUPPORTSHADER_HPP__

#include "TMultiBlendShader.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "hecl/VertexBufferPool.hpp"
#include "hecl/UniformBufferPool.hpp"

namespace urde
{
class CGlyph;
class CTextRenderBuffer;

class CTextSupportShader
{
    friend struct CTextSupportShaderGLDataBindingFactory;
    friend struct CTextSupportShaderVulkanDataBindingFactory;
    friend struct CTextSupportShaderMetalDataBindingFactory;
    friend struct CTextSupportShaderD3DDataBindingFactory;
    friend class CTextRenderBuffer;

    static boo::IVertexFormat* s_TextVtxFmt;
    static boo::IShaderPipeline* s_TextAlphaPipeline;
    static boo::IShaderPipeline* s_TextAddPipeline;

    static boo::IVertexFormat* s_ImageVtxFmt;
    static boo::IShaderPipeline* s_ImageAlphaPipeline;
    static boo::IShaderPipeline* s_ImageAddPipeline;

    struct Uniform
    {
        zeus::CMatrix4f m_mvp;
        zeus::CColor m_uniformColor;
    };

    struct CharacterInstance
    {
        zeus::CVector3f m_pos[4];
        zeus::CVector3f m_uv[4];
        zeus::CColor m_fontColor;
        zeus::CColor m_outlineColor;
        zeus::CColor m_mulColor;
        void SetMetrics(const CGlyph& glyph, const zeus::CVector2i& offset);
    };

    struct ImageInstance
    {
        zeus::CVector3f m_pos[4];
        zeus::CVector2f m_uv[4];
        zeus::CColor m_color;
        void SetMetrics(const zeus::CVector2f& imgSize, const zeus::CVector2i& offset);
    };

    static hecl::VertexBufferPool<CharacterInstance> s_CharInsts;
    static hecl::VertexBufferPool<ImageInstance> s_ImgInsts;
    static hecl::UniformBufferPool<Uniform> s_Uniforms;

public:
    using _CLS = CTextSupportShader;
#include "TMultiBlendShaderDecl.hpp"

    static boo::IShaderPipeline* SelectTextPipeline(CGuiWidget::EGuiModelDrawFlags df)
    {
        switch (df)
        {
        case CGuiWidget::EGuiModelDrawFlags::Shadeless:
        case CGuiWidget::EGuiModelDrawFlags::Opaque:
        case CGuiWidget::EGuiModelDrawFlags::Alpha:
            return s_TextAlphaPipeline;
        case CGuiWidget::EGuiModelDrawFlags::Additive:
        case CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw:
            return s_TextAddPipeline;
        default:
            return nullptr;
        }
    }

    static boo::IShaderPipeline* SelectImagePipeline(CGuiWidget::EGuiModelDrawFlags df)
    {
        switch (df)
        {
        case CGuiWidget::EGuiModelDrawFlags::Shadeless:
        case CGuiWidget::EGuiModelDrawFlags::Opaque:
        case CGuiWidget::EGuiModelDrawFlags::Alpha:
            return s_ImageAlphaPipeline;
        case CGuiWidget::EGuiModelDrawFlags::Additive:
        case CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw:
            return s_ImageAddPipeline;
        default:
            return nullptr;
        }
    }

    static void UpdateBuffers()
    {
        s_CharInsts.updateBuffers();
        s_ImgInsts.updateBuffers();
        s_Uniforms.updateBuffers();
    }
};

}

#endif // __URDE_CTEXTSUPPORTSHADER_HPP__
