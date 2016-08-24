#ifndef __URDE_TSHADER_HPP__
#define __URDE_TSHADER_HPP__

#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/Vulkan.hpp"

namespace urde
{

template <class FilterImp>
class TShader
{
public:
    struct IDataBindingFactory
    {
        virtual boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                boo::IShaderPipeline* pipeline,
                                                                boo::IVertexFormat* vtxFmt,
                                                                FilterImp& filter)=0;
    };

    static boo::IShaderPipeline* m_pipeline;
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
            case boo::IGraphicsDataFactory::Platform::OpenGL:
                m_bindFactory.reset(FilterImp::Initialize(static_cast<boo::GLDataFactory::Context&>(ctx),
                                                          m_pipeline));
                break;
#if _WIN32
            case boo::IGraphicsDataFactory::Platform::D3D11:
            case boo::IGraphicsDataFactory::Platform::D3D12:
                m_bindFactory.reset(FilterImp::Initialize(static_cast<boo::ID3DDataFactory::Context&>(ctx),
                                                          m_pipeline, m_vtxFmt));
                break;
#endif
#if BOO_HAS_METAL
            case boo::IGraphicsDataFactory::Platform::Metal:
                m_bindFactory.reset(FilterImp::Initialize(static_cast<boo::MetalDataFactory::Context&>(ctx),
                                                          m_pipeline, m_vtxFmt));
                break;
#endif
#if BOO_HAS_VULKAN
            case boo::IGraphicsDataFactory::Platform::Vulkan:
                m_bindFactory.reset(FilterImp::Initialize(static_cast<boo::VulkanDataFactory::Context&>(ctx),
                                                          m_pipeline, m_vtxFmt));
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

    static boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, FilterImp& filter)
    {
        return m_bindFactory->BuildShaderDataBinding(ctx, m_pipeline, m_vtxFmt, filter);
    }
};

#define URDE_DECL_SPECIALIZE_SHADER(cls) \
template <> boo::IShaderPipeline* \
TShader<cls>::m_pipeline; \
template <> boo::IVertexFormat* \
TShader<cls>::m_vtxFmt; \
\
template <> std::unique_ptr<TShader<cls>::IDataBindingFactory> \
TShader<cls>::m_bindFactory; \
template <> boo::GraphicsDataToken \
TShader<cls>::m_gfxToken; \

#define URDE_SPECIALIZE_SHADER(cls) \
template <> boo::IShaderPipeline* \
TShader<cls>::m_pipeline = nullptr; \
template <> boo::IVertexFormat* \
TShader<cls>::m_vtxFmt = nullptr; \
\
template <> std::unique_ptr<TShader<cls>::IDataBindingFactory> \
TShader<cls>::m_bindFactory = {}; \
template <> boo::GraphicsDataToken \
TShader<cls>::m_gfxToken = {}; \
\
template class TShader<cls>;

}

#endif // __URDE_TSHADER_HPP__
