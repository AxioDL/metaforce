#include "CAABoxShader.hpp"
#include "hecl/Pipeline.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;
static boo::ObjToken<boo::IShaderPipeline> s_zOnlyPipeline;

void CAABoxShader::Initialize() {
  s_Pipeline = hecl::conv->convert(Shader_CAABoxShader{});
  s_zOnlyPipeline = hecl::conv->convert(Shader_CAABoxShaderZOnly{});
}

void CAABoxShader::Shutdown() {
  s_Pipeline.reset();
  s_zOnlyPipeline.reset();
}

CAABoxShader::CAABoxShader(bool zOnly) {
  CGraphics::CommitResources([this, zOnly](boo::IGraphicsDataFactory::Context& ctx) {
    m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(zeus::CVector3f), 34);
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
    boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
    m_dataBind = ctx.newShaderDataBinding(zOnly ? s_zOnlyPipeline : s_Pipeline, m_vbo.get(), nullptr, nullptr, 1, bufs,
                                          stages, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    return true;
  } BooTrace);
}

void CAABoxShader::setAABB(const zeus::CAABox& aabb) {
  zeus::CVector3f vboData[] = {
      {aabb.max.x(), aabb.max.y(), aabb.min.z()}, {aabb.max.x(), aabb.min.y(), aabb.min.z()},
      {aabb.max.x(), aabb.max.y(), aabb.max.z()}, {aabb.max.x(), aabb.min.y(), aabb.max.z()},
      {aabb.max.x(), aabb.min.y(), aabb.max.z()},

      {aabb.min.x(), aabb.max.y(), aabb.min.z()}, {aabb.min.x(), aabb.max.y(), aabb.min.z()},
      {aabb.max.x(), aabb.max.y(), aabb.min.z()}, {aabb.min.x(), aabb.max.y(), aabb.max.z()},
      {aabb.max.x(), aabb.max.y(), aabb.max.z()}, {aabb.max.x(), aabb.max.y(), aabb.max.z()},

      {aabb.min.x(), aabb.max.y(), aabb.min.z()}, {aabb.min.x(), aabb.max.y(), aabb.min.z()},
      {aabb.min.x(), aabb.min.y(), aabb.min.z()}, {aabb.min.x(), aabb.max.y(), aabb.max.z()},
      {aabb.min.x(), aabb.min.y(), aabb.max.z()}, {aabb.min.x(), aabb.min.y(), aabb.max.z()},

      {aabb.min.x(), aabb.min.y(), aabb.min.z()}, {aabb.min.x(), aabb.min.y(), aabb.min.z()},
      {aabb.max.x(), aabb.min.y(), aabb.min.z()}, {aabb.min.x(), aabb.min.y(), aabb.max.z()},
      {aabb.max.x(), aabb.min.y(), aabb.max.z()}, {aabb.max.x(), aabb.min.y(), aabb.max.z()},

      {aabb.min.x(), aabb.min.y(), aabb.max.z()}, {aabb.min.x(), aabb.min.y(), aabb.max.z()},
      {aabb.max.x(), aabb.min.y(), aabb.max.z()}, {aabb.min.x(), aabb.max.y(), aabb.max.z()},
      {aabb.max.x(), aabb.max.y(), aabb.max.z()}, {aabb.max.x(), aabb.max.y(), aabb.max.z()},

      {aabb.min.x(), aabb.min.y(), aabb.min.z()}, {aabb.min.x(), aabb.min.y(), aabb.min.z()},
      {aabb.max.x(), aabb.min.y(), aabb.min.z()}, {aabb.min.x(), aabb.max.y(), aabb.min.z()},
      {aabb.max.x(), aabb.max.y(), aabb.min.z()},
  };

  m_vbo->load(vboData, sizeof(zeus::CVector3f) * 34);
}

void CAABoxShader::draw(const zeus::CColor& color) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CAABoxShader::draw", zeus::skMagenta);

  m_uniform.m_xf = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniform.m_color = color;
  m_uniBuf->load(&m_uniform, sizeof(Uniform));

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawArray(0, 34);
}

} // namespace urde
