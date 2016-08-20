#ifndef __URDE_TMULTIBLENDSHADER_HPP__
#define __URDE_TMULTIBLENDSHADER_HPP__

#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/Vulkan.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{

template <class FilterImp>
class TMultiBlendShader
{
public:
    struct IDataBindingFactory
    {
        virtual boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                boo::IShaderPipeline* pipeline,
                                                                boo::IVertexFormat* vtxFmt,
                                                                FilterImp& filter)=0;
    };

    static boo::IShaderPipeline* m_alphaBlendPipeline;
    static boo::IShaderPipeline* m_additiveAlphaPipeline;
    static boo::IShaderPipeline* m_colorMultiplyPipeline;
    static boo::IVertexFormat* m_vtxFmt; /* No OpenGL */

    static std::unique_ptr<IDataBindingFactory> m_bindFactory;
    static boo::GraphicsDataToken m_gfxToken;

    static void Initialize()
    {
        if (!CGraphics::g_BooFactory)
            return;

        m_gfxToken = CGraphics::CommitResources(
        [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            switch (ctx.platform())
            {
            case boo::IGraphicsDataFactory::Platform::OGL:
                m_bindFactory.reset(FilterImp::Initialize(static_cast<boo::GLDataFactory::Context&>(ctx),
                                                          m_alphaBlendPipeline, m_additiveAlphaPipeline, m_colorMultiplyPipeline));
                break;
#if _WIN32
            case boo::IGraphicsDataFactory::Platform::D3D11:
            case boo::IGraphicsDataFactory::Platform::D3D12:
                m_bindFactory.reset(FilterImp::Initialize(static_cast<boo::ID3DDataFactory::Context&>(ctx),
                                                          m_alphaBlendPipeline, m_additiveAlphaPipeline,
                                                          m_colorMultiplyPipeline, m_vtxFmt));
                break;
#endif
#if BOO_HAS_METAL
            case boo::IGraphicsDataFactory::Platform::Metal:
                m_bindFactory.reset(FilterImp::Initialize(static_cast<boo::MetalDataFactory::Context&>(ctx),
                                                          m_alphaBlendPipeline, m_additiveAlphaPipeline,
                                                          m_colorMultiplyPipeline, m_vtxFmt));
                break;
#endif
#if BOO_HAS_VULKAN
            case boo::IGraphicsDataFactory::Platform::Vulkan:
                m_bindFactory.reset(FilterImp::Initialize(static_cast<boo::VulkanDataFactory::Context&>(ctx),
                                                          m_alphaBlendPipeline, m_additiveAlphaPipeline,
                                                          m_colorMultiplyPipeline, m_vtxFmt));
                break;
#endif
            default: break;
            }
            return true;
        });
    }

    static void Shutdown()
    {
        m_gfxToken.doDestroy();
    }

    static boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                           CCameraFilterPass::EFilterType type,
                                                           FilterImp& filter)
    {
        if (type == CCameraFilterPass::EFilterType::Add)
            return m_bindFactory->BuildShaderDataBinding(ctx, m_additiveAlphaPipeline, m_vtxFmt, filter);
        else if (type == CCameraFilterPass::EFilterType::Multiply)
            return m_bindFactory->BuildShaderDataBinding(ctx, m_colorMultiplyPipeline, m_vtxFmt, filter);
        else
            return m_bindFactory->BuildShaderDataBinding(ctx, m_alphaBlendPipeline, m_vtxFmt, filter);
    }
};

#define URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(cls) \
template <> boo::IShaderPipeline* \
TMultiBlendShader<cls>::m_alphaBlendPipeline; \
template <> boo::IShaderPipeline* \
TMultiBlendShader<cls>::m_additiveAlphaPipeline; \
template <> boo::IShaderPipeline* \
TMultiBlendShader<cls>::m_colorMultiplyPipeline; \
template <> boo::IVertexFormat* \
TMultiBlendShader<cls>::m_vtxFmt; \
\
template <> std::unique_ptr<TMultiBlendShader<cls>::IDataBindingFactory> \
TMultiBlendShader<cls>::m_bindFactory; \
template <> boo::GraphicsDataToken \
TMultiBlendShader<cls>::m_gfxToken; \

#define URDE_SPECIALIZE_MULTI_BLEND_SHADER(cls) \
template <> boo::IShaderPipeline* \
TMultiBlendShader<cls>::m_alphaBlendPipeline = nullptr; \
template <> boo::IShaderPipeline* \
TMultiBlendShader<cls>::m_additiveAlphaPipeline = nullptr; \
template <> boo::IShaderPipeline* \
TMultiBlendShader<cls>::m_colorMultiplyPipeline = nullptr; \
template <> boo::IVertexFormat* \
TMultiBlendShader<cls>::m_vtxFmt = nullptr; \
\
template <> std::unique_ptr<TMultiBlendShader<cls>::IDataBindingFactory> \
TMultiBlendShader<cls>::m_bindFactory = {}; \
template <> boo::GraphicsDataToken \
TMultiBlendShader<cls>::m_gfxToken = {}; \
\
template class TMultiBlendShader<cls>;

}

#endif // __URDE_TMULTIBLENDSHADER_HPP__
