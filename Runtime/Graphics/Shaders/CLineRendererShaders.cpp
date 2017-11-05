#include "CLineRendererShaders.hpp"

namespace urde
{

boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_texAlpha;
boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_texAdditive;

boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_noTexAlpha;
boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_noTexAdditive;

boo::ObjToken<boo::IVertexFormat> CLineRendererShaders::m_texVtxFmt;
boo::ObjToken<boo::IVertexFormat> CLineRendererShaders::m_noTexVtxFmt;

std::unique_ptr<CLineRendererShaders::IDataBindingFactory> CLineRendererShaders::m_bindFactory;

void CLineRendererShaders::Initialize()
{
    if (!CGraphics::g_BooFactory)
        return;

    CGraphics::CommitResources(
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
    m_texAlpha.reset();
    m_texAdditive.reset();
    m_noTexAlpha.reset();
    m_noTexAdditive.reset();
    m_texVtxFmt.reset();
    m_noTexVtxFmt.reset();
}

void CLineRendererShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                  CLineRenderer& renderer,
                                                  const boo::ObjToken<boo::ITexture>& texture,
                                                  bool additive)
{
    boo::ObjToken<boo::IShaderPipeline> pipeline;
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
