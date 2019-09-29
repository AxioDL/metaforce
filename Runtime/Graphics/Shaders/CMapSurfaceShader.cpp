#include "Runtime/Graphics/Shaders/CMapSurfaceShader.hpp"

#include <array>

#include "Runtime/Graphics/CGraphics.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CMapSurfaceShader::Initialize() { s_Pipeline = hecl::conv->convert(Shader_CMapSurfaceShader{}); }

void CMapSurfaceShader::Shutdown() { s_Pipeline.reset(); }

CMapSurfaceShader::CMapSurfaceShader(boo::IGraphicsDataFactory::Context& ctx,
                                     const boo::ObjToken<boo::IGraphicsBufferS>& vbo,
                                     const boo::ObjToken<boo::IGraphicsBufferS>& ibo)
: m_vbo(vbo), m_ibo(ibo) {
  m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
  const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 1> bufs{m_uniBuf.get()};
  constexpr std::array<boo::PipelineStage, 1> stages{boo::PipelineStage::Vertex};
  m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, m_ibo.get(), bufs.size(), bufs.data(),
                                        stages.data(), nullptr, nullptr, 0, nullptr, nullptr, nullptr);
}

void CMapSurfaceShader::draw(const zeus::CColor& color, u32 start, u32 count) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CMapSurfaceShader::draw", zeus::skMagenta);
  Uniform uniform = {CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f(), color};
  m_uniBuf->load(&uniform, sizeof(Uniform));
  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArrayIndexed(start, count);
}

} // namespace urde
