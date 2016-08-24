#include "CLineRendererShaders.hpp"

namespace urde
{

boo::IShaderPipeline* CLineRendererShaders::m_texAlpha = nullptr;
boo::IShaderPipeline* CLineRendererShaders::m_texAdditive = nullptr;

boo::IShaderPipeline* CLineRendererShaders::m_noTexAlpha = nullptr;
boo::IShaderPipeline* CLineRendererShaders::m_noTexAdditive = nullptr;

boo::IVertexFormat* CLineRendererShaders::m_texVtxFmt = nullptr;
boo::IVertexFormat* CLineRendererShaders::m_noTexVtxFmt = nullptr;

std::unique_ptr<CLineRendererShaders::IDataBindingFactory> CLineRendererShaders::m_bindFactory;
boo::GraphicsDataToken CLineRendererShaders::m_gfxToken;

void CLineRendererShaders::Initialize()
{
    if (!CGraphics::g_BooFactory)
        return;

    m_gfxToken = CGraphics::CommitResources(
    [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        switch (ctx.platform())
        {
        case boo::IGraphicsDataFactory::Platform::OpenGL:
            m_bindFactory.reset(Initialize(static_cast<boo::GLDataFactory::Context&>(ctx)));
            break;
#if _WIN32
        case boo::IGraphicsDataFactory::Platform::D3D11:
        case boo::IGraphicsDataFactory::Platform::D3D12:
            m_bindFactory.reset(Initialize(static_cast<boo::ID3DDataFactory::Context&>(ctx)));
            break;
#endif
#if BOO_HAS_METAL
        case boo::IGraphicsDataFactory::Platform::Metal:
            m_bindFactory.reset(Initialize(static_cast<boo::MetalDataFactory::Context&>(ctx)));
            break;
#endif
#if BOO_HAS_VULKAN
        case boo::IGraphicsDataFactory::Platform::Vulkan:
            m_bindFactory.reset(Initialize(static_cast<boo::VulkanDataFactory::Context&>(ctx)));
            break;
#endif
        default: break;
        }
        return true;
    });
}

void CLineRendererShaders::Shutdown()
{
    m_gfxToken.doDestroy();
}

void CLineRendererShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                  CLineRenderer& renderer,
                                                  boo::ITexture* texture,
                                                  bool additive)
{
    boo::IShaderPipeline* pipeline = nullptr;
    if (texture)
    {
        if (additive)
            pipeline = m_texAdditive;
        else
            pipeline = m_texAlpha;
    }
    else
    {
        if (additive)
            pipeline = m_noTexAdditive;
        else
            pipeline = m_noTexAlpha;
    }

    m_bindFactory->BuildShaderDataBinding(ctx, renderer, pipeline, texture);
}

}
