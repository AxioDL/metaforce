#include "Runtime/Graphics/Shaders/CRadarPaintShader.hpp"

#include <cstring>

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CRadarPaintShader::Initialize() { s_Pipeline = hecl::conv->convert(Shader_CRadarPaintShader{}); }

void CRadarPaintShader::Shutdown() { s_Pipeline.reset(); }

void CRadarPaintShader::draw(const std::vector<Instance>& instances, const CTexture* tex) {
  if (!instances.size())
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CRadarPaintShader::draw", zeus::skMagenta);

  if (instances.size() > m_maxInsts) {
    m_maxInsts = instances.size();
    m_tex = tex;
    CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
      m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(Instance), m_maxInsts);
      m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CMatrix4f), 1);
      boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
      boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
      boo::ObjToken<boo::ITexture> texs[] = {m_tex->GetBooTexture()};
      m_dataBind = ctx.newShaderDataBinding(s_Pipeline, nullptr, m_vbo.get(), nullptr, 1, bufs, stages, nullptr,
                                            nullptr, 1, texs, nullptr, nullptr);
      return true;
    } BooTrace);
  }

  zeus::CMatrix4f uniMtx = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniBuf->load(&uniMtx, sizeof(zeus::CMatrix4f));

  size_t mapSz = sizeof(Instance) * instances.size();
  Instance* insts = reinterpret_cast<Instance*>(m_vbo->map(mapSz));
  memmove(insts, instances.data(), mapSz);
  m_vbo->unmap();

  CGraphics::SetShaderDataBinding(m_dataBind);
  CGraphics::DrawInstances(0, 4, instances.size());
}

} // namespace urde
