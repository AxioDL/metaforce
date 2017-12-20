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
class CFontImageDef;

class CTextSupportShader
{
    friend struct CTextSupportShaderGLDataBindingFactory;
    friend struct CTextSupportShaderVulkanDataBindingFactory;
    friend struct CTextSupportShaderMetalDataBindingFactory;
    friend struct CTextSupportShaderD3DDataBindingFactory;
    friend class CTextRenderBuffer;

    static boo::ObjToken<boo::IVertexFormat> s_TextVtxFmt;
    static boo::ObjToken<boo::IShaderPipeline> s_TextAlphaPipeline;
    static boo::ObjToken<boo::IShaderPipeline> s_TextAddPipeline;
    static boo::ObjToken<boo::IShaderPipeline> s_TextAddOverdrawPipeline;

    static boo::ObjToken<boo::IVertexFormat> s_ImageVtxFmt;
    static boo::ObjToken<boo::IShaderPipeline> s_ImageAlphaPipeline;
    static boo::ObjToken<boo::IShaderPipeline> s_ImageAddPipeline;
    static boo::ObjToken<boo::IShaderPipeline> s_ImageAddOverdrawPipeline;

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
        void SetMetrics(const CFontImageDef& imgDef, const zeus::CVector2i& offset);
    };

    static hecl::VertexBufferPool<CharacterInstance> s_CharInsts;
    static hecl::VertexBufferPool<ImageInstance> s_ImgInsts;
    static hecl::UniformBufferPool<Uniform> s_Uniforms;

public:
    using _CLS = CTextSupportShader;
#include "TMultiBlendShaderDecl.hpp"

    static boo::ObjToken<boo::IShaderPipeline> SelectTextPipeline(CGuiWidget::EGuiModelDrawFlags df)
    {
        switch (df)
        {
        case CGuiWidget::EGuiModelDrawFlags::Shadeless:
        case CGuiWidget::EGuiModelDrawFlags::Opaque:
        case CGuiWidget::EGuiModelDrawFlags::Alpha:
        case CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw:
            return s_TextAlphaPipeline;
        case CGuiWidget::EGuiModelDrawFlags::Additive:
            return s_TextAddPipeline;
        default:
            return {};
        }
    }

    static boo::ObjToken<boo::IShaderPipeline> SelectImagePipeline(CGuiWidget::EGuiModelDrawFlags df)
    {
        switch (df)
        {
        case CGuiWidget::EGuiModelDrawFlags::Shadeless:
        case CGuiWidget::EGuiModelDrawFlags::Opaque:
        case CGuiWidget::EGuiModelDrawFlags::Alpha:
        case CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw:
            return s_ImageAlphaPipeline;
        case CGuiWidget::EGuiModelDrawFlags::Additive:
            return s_ImageAddPipeline;
        default:
            return {};
        }
    }

    static boo::ObjToken<boo::IShaderPipeline> GetTextAdditiveOverdrawPipeline()
    {
        return s_TextAddOverdrawPipeline;
    }

    static boo::ObjToken<boo::IShaderPipeline> GetImageAdditiveOverdrawPipeline()
    {
        return s_ImageAddOverdrawPipeline;
    }

    static void UpdateBuffers()
    {
        s_CharInsts.updateBuffers();
        s_ImgInsts.updateBuffers();
        s_Uniforms.updateBuffers();
    }

    static void Shutdown();
};

}

#endif // __URDE_CTEXTSUPPORTSHADER_HPP__
