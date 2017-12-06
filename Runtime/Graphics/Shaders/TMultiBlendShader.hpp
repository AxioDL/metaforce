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

template <class ShaderImp>
class TMultiBlendShader
{
public:
    struct IDataBindingFactory
    {
        virtual boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                              EFilterType type, ShaderImp& filter)=0;
    };

    static std::unique_ptr<IDataBindingFactory> m_bindFactory;

    static void Initialize()
    {
        if (!CGraphics::g_BooFactory)
            return;

        CGraphics::CommitResources(
        [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
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
            case boo::IGraphicsDataFactory::Platform::D3D12:
                m_bindFactory.reset(ShaderImp::Initialize(static_cast<boo::ID3DDataFactory::Context&>(ctx)));
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
        });
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
        case boo::IGraphicsDataFactory::Platform::D3D12:
            ShaderImp::template Shutdown<boo::ID3DDataFactory>();
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
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           EFilterType type, ShaderImp& filter)
    {
        return m_bindFactory->BuildShaderDataBinding(ctx, type, filter);
    }
};

#define URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(cls) \
template <> std::unique_ptr<TMultiBlendShader<cls>::IDataBindingFactory> \
TMultiBlendShader<cls>::m_bindFactory; \

#define URDE_SPECIALIZE_MULTI_BLEND_SHADER(cls) \
template <> std::unique_ptr<TMultiBlendShader<cls>::IDataBindingFactory> \
TMultiBlendShader<cls>::m_bindFactory = {}; \
\
template class TMultiBlendShader<cls>;

}

#endif // __URDE_TMULTIBLENDSHADER_HPP__
