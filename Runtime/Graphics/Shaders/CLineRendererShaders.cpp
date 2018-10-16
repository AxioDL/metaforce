#include "CLineRendererShaders.hpp"
#include "Graphics/CLineRenderer.hpp"
#include "hecl/Pipeline.hpp"

namespace urde
{

boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_texAlpha;
boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_texAdditive;

boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_noTexAlpha;
boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_noTexAdditive;

boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_texAlphaZ;
boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_texAdditiveZ;

boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_noTexAlphaZ;
boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_noTexAdditiveZ;

boo::ObjToken<boo::IShaderPipeline> CLineRendererShaders::m_noTexAlphaZGEqual;

void CLineRendererShaders::Initialize()
{
    CGraphics::CommitResources(
    [](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_texAlpha = hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAlpha{});
        m_texAdditive = hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAdditive{});
        m_noTexAlpha = hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlpha{});
        m_noTexAdditive = hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAdditive{});
        m_texAlphaZ = hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAlphaZ{});
        m_texAdditiveZ = hecl::conv->convert(ctx, Shader_CLineRendererShaderTexAdditiveZ{});
        m_noTexAlphaZ = hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlphaZ{});
        m_noTexAdditiveZ = hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAdditiveZ{});
        m_noTexAlphaZGEqual = hecl::conv->convert(ctx, Shader_CLineRendererShaderNoTexAlphaZGEqual{});
        return true;
    } BooTrace);
}

void CLineRendererShaders::Shutdown()
{
    m_texAlpha.reset();
    m_texAdditive.reset();
    m_noTexAlpha.reset();
    m_noTexAdditive.reset();
    m_texAlphaZ.reset();
    m_texAdditiveZ.reset();
    m_noTexAlphaZ.reset();
    m_noTexAdditiveZ.reset();
    m_noTexAlphaZGEqual.reset();
}

void CLineRendererShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                  CLineRenderer& renderer,
                                                  const boo::ObjToken<boo::ITexture>& texture,
                                                  bool additive, bool zTest, bool zGEqual)
{
    boo::ObjToken<boo::IShaderPipeline> pipeline;

    if (zGEqual)
    {
        pipeline = m_noTexAlphaZGEqual;
    }
    else if (zTest)
    {
        if (texture)
        {
            if (additive)
                pipeline = m_texAdditiveZ;
            else
                pipeline = m_texAlphaZ;
        }
        else
        {
            if (additive)
                pipeline = m_noTexAdditiveZ;
            else
                pipeline = m_noTexAlphaZ;
        }
    }
    else
    {
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
    }

    int texCount = 0;
    boo::ObjToken<boo::ITexture> textures[1];

    std::pair<boo::ObjToken<boo::IGraphicsBufferD>,
        hecl::VertexBufferPool<CLineRenderer::SDrawVertTex>::IndexTp> vbufInfo;
    std::pair<boo::ObjToken<boo::IGraphicsBufferD>,
        hecl::UniformBufferPool<CLineRenderer::SDrawUniform>::IndexTp> ubufInfo =
        renderer.m_uniformBuf.getBufferInfo();
    if (texture)
    {
        vbufInfo = renderer.m_vertBufTex.getBufferInfo();
        textures[0] = texture;
        texCount = 1;
    }
    else
    {
        vbufInfo = renderer.m_vertBufNoTex.getBufferInfo();
    }

    boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {ubufInfo.first.get()};
    boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
    size_t ubufOffs[] = {size_t(ubufInfo.second)};
    size_t ubufSizes[] = {sizeof(CLineRenderer::SDrawUniform)};

    renderer.m_shaderBind = ctx.newShaderDataBinding(pipeline, vbufInfo.first.get(),
                                                     nullptr, nullptr, 1, uniforms, stages,
                                                     ubufOffs, ubufSizes, texCount, textures,
                                                     nullptr, nullptr, vbufInfo.second);
}

}
