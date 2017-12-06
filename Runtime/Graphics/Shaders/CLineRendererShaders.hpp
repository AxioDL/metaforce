#ifndef __URDE_CLINERENDERERSHADERS_HPP__
#define __URDE_CLINERENDERERSHADERS_HPP__

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
                                            const boo::ObjToken<boo::IShaderPipeline>& pipeline,
                                            const boo::ObjToken<boo::ITexture>& texture)=0;
    };

private:
    static boo::ObjToken<boo::IShaderPipeline> m_texAlpha;
    static boo::ObjToken<boo::IShaderPipeline> m_texAdditive;

    static boo::ObjToken<boo::IShaderPipeline> m_noTexAlpha;
    static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditive;

    static boo::ObjToken<boo::IVertexFormat> m_texVtxFmt;
    static boo::ObjToken<boo::IVertexFormat> m_noTexVtxFmt;

    static std::unique_ptr<IDataBindingFactory> m_bindFactory;

public:
#if BOO_HAS_GL
    static IDataBindingFactory* Initialize(boo::GLDataFactory::Context& ctx);
#endif
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
                                       const boo::ObjToken<boo::ITexture>& texture, bool additive);
};

}

#endif // __URDE_CLINERENDERERSHADERS_HPP__
