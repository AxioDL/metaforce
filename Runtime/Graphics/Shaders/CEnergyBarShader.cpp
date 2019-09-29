#include "Runtime/Graphics/Shaders/CEnergyBarShader.hpp"

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CEnergyBarShader::Initialize() { s_Pipeline = hecl::conv->convert(Shader_CEnergyBarShader{}); }

void CEnergyBarShader::Shutdown() { s_Pipeline.reset(); }

void CEnergyBarShader::updateModelMatrix() {
  m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
}

void CEnergyBarShader::draw(const zeus::CColor& color0, const std::vector<Vertex>& verts0, const zeus::CColor& color1,
                            const std::vector<Vertex>& verts1, const zeus::CColor& color2,
                            const std::vector<Vertex>& verts2, const CTexture* tex) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CEnergyBarShader::draw", zeus::skMagenta);

  size_t totalVerts = verts0.size() + verts1.size() + verts2.size();
  if (!totalVerts)
    return;

  if (totalVerts > m_maxVerts) {
    m_maxVerts = totalVerts;
    m_tex = tex;
    CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
      m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(Vertex), m_maxVerts);
      boo::ObjToken<boo::IGraphicsBuffer> bufs[1];
      boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
      boo::ObjToken<boo::ITexture> texs[] = {m_tex->GetBooTexture()};
      for (int i = 0; i < 3; ++i) {
        m_uniBuf[i] = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        bufs[0] = m_uniBuf[i].get();
        m_dataBind[i] = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr, 1, bufs, stages, nullptr,
                                                 nullptr, 1, texs, nullptr, nullptr);
      }
      return true;
    } BooTrace);
  }

  size_t vertIter = 0;
  Vertex* verts = reinterpret_cast<Vertex*>(m_vbo->map(sizeof(Vertex) * totalVerts));
  if (verts0.size()) {
    memmove(verts, verts0.data(), sizeof(Vertex) * verts0.size());
    vertIter += verts0.size();
  }
  if (verts1.size()) {
    memmove(verts + vertIter, verts1.data(), sizeof(Vertex) * verts1.size());
    vertIter += verts1.size();
  }
  if (verts2.size()) {
    memmove(verts + vertIter, verts2.data(), sizeof(Vertex) * verts2.size());
  }
  m_vbo->unmap();

  vertIter = 0;
  if (verts0.size()) {
    m_uniform.m_color = color0;
    m_uniBuf[0]->load(&m_uniform, sizeof(Uniform));
    CGraphics::SetShaderDataBinding(m_dataBind[0]);
    CGraphics::DrawArray(0, verts0.size());
    vertIter += verts0.size();
  }
  if (verts1.size()) {
    m_uniform.m_color = color1;
    m_uniBuf[1]->load(&m_uniform, sizeof(Uniform));
    CGraphics::SetShaderDataBinding(m_dataBind[1]);
    CGraphics::DrawArray(vertIter, verts1.size());
    vertIter += verts1.size();
  }
  if (verts2.size()) {
    m_uniform.m_color = color2;
    m_uniBuf[2]->load(&m_uniform, sizeof(Uniform));
    CGraphics::SetShaderDataBinding(m_dataBind[2]);
    CGraphics::DrawArray(vertIter, verts2.size());
  }
}

} // namespace urde
