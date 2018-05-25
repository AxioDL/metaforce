#ifndef __URDE_TSHADER_HPP__
#define __URDE_TSHADER_HPP__

#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/Vulkan.hpp"

namespace urde
{

template <class ShaderImp>
class TShader
{
public:
    struct IDataBindingFactory
    {
        virtual boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                              ShaderImp& filter)=0;
        virtual ~IDataBindingFactory() = default;
    };

    static std::unique_ptr<IDataBindingFactory> m_bindFactory;

    static void Initialize()
    {
        if (!CGraphics::g_BooFactory)
            return;

        CGraphics::CommitResources(
        [&](boo::IGraphicsDataFactory::Context& ctx)
        {
            switch (ctx.platform())
            {
#if BOO_HAS_GL
            case boo::IGraphicsDataFactory::Platform::OpenGL:
                m_bindFactory.reset(ShaderImp::Initialize(static_cast<boo::GLDataFactory::Context&>(ctx)));
                break;
#endif
#if _WIN32
            case boo::IGraphicsDataFactory::Platform::D3D11:
                m_bindFactory.reset(ShaderImp::Initialize(static_cast<boo::D3DDataFactory::Context&>(ctx)));
                break;
#endif
#if BOO_HAS_METAL
            case boo::IGraphicsDataFactory::Platform::Metal:
                m_bindFactory.reset(ShaderImp::Initialize(static_cast<boo::MetalDataFactory::Context&>(ctx)));
                break;
#endif
#if BOO_HAS_VULKAN
            case boo::IGraphicsDataFactory::Platform::Vulkan:
                m_bindFactory.reset(ShaderImp::Initialize(static_cast<boo::VulkanDataFactory::Context&>(ctx)));
                break;
#endif
            default: break;
            }
            return true;
        } BooTrace);
    }

    static void Shutdown()
    {
        switch (CGraphics::g_BooFactory->platform())
        {
#if BOO_HAS_GL
        case boo::IGraphicsDataFactory::Platform::OpenGL:
            ShaderImp::template Shutdown<boo::GLDataFactory>();
            break;
#endif
#if _WIN32
        case boo::IGraphicsDataFactory::Platform::D3D11:
            ShaderImp::template Shutdown<boo::D3DDataFactory>();
            break;
#endif
#if BOO_HAS_METAL
        case boo::IGraphicsDataFactory::Platform::Metal:
            ShaderImp::template Shutdown<boo::MetalDataFactory>();
            break;
#endif
#if BOO_HAS_VULKAN
        case boo::IGraphicsDataFactory::Platform::Vulkan:
            ShaderImp::template Shutdown<boo::VulkanDataFactory>();
            break;
#endif
        default: break;
        }
    }

    static boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, ShaderImp& filter)
    {
        return m_bindFactory->BuildShaderDataBinding(ctx, filter);
    }
};

#define URDE_DECL_SPECIALIZE_SHADER(cls) \
template <> std::unique_ptr<TShader<cls>::IDataBindingFactory> \
TShader<cls>::m_bindFactory;

#define URDE_SPECIALIZE_SHADER(cls) \
template <> std::unique_ptr<TShader<cls>::IDataBindingFactory> \
TShader<cls>::m_bindFactory = {}; \
\
template class TShader<cls>;

}

#endif // __URDE_TSHADER_HPP__
