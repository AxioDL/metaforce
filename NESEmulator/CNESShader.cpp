#include "CNESShader.hpp"
#include "Graphics/CGraphics.hpp"
#include "hecl/Pipeline.hpp"

namespace urde::MP1 {

boo::ObjToken<boo::IShaderPipeline> CNESShader::g_Pipeline;

void CNESShader::Initialize() { g_Pipeline = hecl::conv->convert(Shader_CNESShader{}); }

boo::ObjToken<boo::IShaderDataBinding> CNESShader::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                          boo::ObjToken<boo::IGraphicsBufferS> vbo,
                                                                          boo::ObjToken<boo::IGraphicsBufferD> uniBuf,
                                                                          boo::ObjToken<boo::ITextureD> tex) {
  boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {uniBuf.get()};
  boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
  boo::ObjToken<boo::ITexture> texs[] = {tex.get()};
  return ctx.newShaderDataBinding(g_Pipeline, vbo.get(), nullptr, nullptr, 1, bufs, stages, nullptr, nullptr, 1, texs,
                                  nullptr, nullptr);
}

void CNESShader::Shutdown() { g_Pipeline.reset(); }

} // namespace urde::MP1
