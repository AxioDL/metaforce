#ifndef URDE_CNESSHADER_HPP
#define URDE_CNESSHADER_HPP

#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Vulkan.hpp"

namespace urde::MP1
{

class CNESShader
{
public:
#if BOO_HAS_GL
    static void Initialize(boo::GLDataFactory::Context& ctx);
    static boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::GLDataFactory::Context& ctx,
                                                                         boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                         boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                         boo::ObjToken<boo::ITextureD> tex);
#endif
#if _WIN32
    static void Initialize(boo::ID3DDataFactory::Context& ctx);
    static boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::ID3DDataFactory::Context& ctx,
                                                                         boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                         boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                         boo::ObjToken<boo::ITextureD> tex);
#endif
#if BOO_HAS_METAL
    static void Initialize(boo::MetalDataFactory::Context& ctx);
    static boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::MetalDataFactory::Context& ctx,
                                                                         boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                         boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                         boo::ObjToken<boo::ITextureD> tex);
#endif
#if BOO_HAS_VULKAN
    static void Initialize(boo::VulkanDataFactory::Context& ctx);
    static boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::VulkanDataFactory::Context& ctx,
                                                                         boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                         boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                         boo::ObjToken<boo::ITextureD> tex);
#endif
    static void Initialize();
    static void Shutdown();

    static boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                         boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                         boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                         boo::ObjToken<boo::ITextureD> tex);

    static boo::ObjToken<boo::IShaderPipeline> g_Pipeline;
    static boo::ObjToken<boo::IVertexFormat> g_VtxFmt;
};

}

#endif // URDE_CNESSHADER_HPP
