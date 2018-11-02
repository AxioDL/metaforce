#include "CEnvFxShaders.hpp"
#include "hecl/Pipeline.hpp"
#include "World/CEnvFxManager.hpp"

namespace urde
{
boo::ObjToken<boo::IShaderPipeline> CEnvFxShaders::m_snowPipeline;
boo::ObjToken<boo::IShaderPipeline> CEnvFxShaders::m_underwaterPipeline;

void CEnvFxShaders::Initialize()
{
    m_snowPipeline = hecl::conv->convert(Shader_CEnvFxSnowShader{});
    m_underwaterPipeline = hecl::conv->convert(Shader_CEnvFxUnderwaterShader{});
}

void CEnvFxShaders::Shutdown()
{
    m_snowPipeline.reset();
    m_underwaterPipeline.reset();
}

void CEnvFxShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                           CEnvFxManager& fxMgr, CEnvFxManagerGrid& grid)
{
    auto uBufInfo = grid.m_uniformBuf.getBufferInfo();
    auto iBufInfo = grid.m_instBuf.getBufferInfo();
    boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {uBufInfo.first.get(),
                                                      fxMgr.m_fogUniformBuf.get()};
    size_t ubufOffsets[] = {uBufInfo.second, 0};
    size_t ubufSizes[] = {256, 256};
    boo::PipelineStage uniformStages[] = {boo::PipelineStage::Vertex, boo::PipelineStage::Fragment};
    boo::ObjToken<boo::ITexture> textures[] = {fxMgr.xb74_txtrSnowFlake->GetBooTexture(),
                                               fxMgr.x40_txtrEnvGradient->GetBooTexture()};
    grid.m_snowBinding = ctx.newShaderDataBinding(m_snowPipeline, nullptr,
                                                  iBufInfo.first.get(), nullptr, 2, uniforms,
                                                  uniformStages, ubufOffsets, ubufSizes,
                                                  2, textures, nullptr, nullptr, 0, iBufInfo.second);
    textures[0] = fxMgr.xc48_underwaterFlake->GetBooTexture();
    grid.m_underwaterBinding = ctx.newShaderDataBinding(m_underwaterPipeline, nullptr,
                                                        iBufInfo.first.get(), nullptr, 2, uniforms,
                                                        uniformStages, ubufOffsets, ubufSizes,
                                                        2, textures, nullptr, nullptr, 0, iBufInfo.second);
}

}
