#pragma once

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde::MP1
{

class CNESShader
{
public:
    static void Initialize();
    static void Shutdown();

    static boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                         boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                         boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                         boo::ObjToken<boo::ITextureD> tex);

    static boo::ObjToken<boo::IShaderPipeline> g_Pipeline;
};

}

