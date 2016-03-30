#ifndef __PSHAG_CLINERENDERERSHADERS_HPP__
#define __PSHAG_CLINERENDERERSHADERS_HPP__

#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/Vulkan.hpp"

namespace urde
{
class CLineRenderer;

class CLineRendererShaders
{
public:
    struct IDataBindingFactory
    {
        virtual void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                            CLineRenderer& renderer,
                                            boo::IShaderPipeline* pipeline,
                                            boo::ITexture* texture)=0;
    };

private:
    static boo::IShaderPipeline* m_texAlpha;
    static boo::IShaderPipeline* m_texAdditive;

    static boo::IShaderPipeline* m_noTexAlpha;
    static boo::IShaderPipeline* m_noTexAdditive;

    static boo::IVertexFormat* m_texVtxFmt;
    static boo::IVertexFormat* m_noTexVtxFmt;

    static std::unique_ptr<IDataBindingFactory> m_bindFactory;
    static boo::GraphicsDataToken m_gfxToken;

public:
    static IDataBindingFactory* Initialize(boo::GLDataFactory::Context& ctx);
#if _WIN32
    static IDataBindingFactory* Initialize(boo::ID3DDataFactory::Context& ctx);
#endif
#if BOO_HAS_METAL
    static IDataBindingFactory* Initialize(boo::MetalDataFactory::Context& ctx);
#endif
#if BOO_HAS_VULKAN
    static IDataBindingFactory* Initialize(boo::VulkanDataFactory::Context& ctx);
#endif

    static void Initialize();
    static void Shutdown();
    static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CLineRenderer& renderer,
                                       boo::ITexture* texture, bool additive);
};

}

#endif // __PSHAG_CLINERENDERERSHADERS_HPP__
