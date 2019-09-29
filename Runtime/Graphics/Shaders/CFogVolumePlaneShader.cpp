#include "Runtime/Graphics/Shaders/CFogVolumePlaneShader.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {

static boo::ObjToken<boo::IShaderPipeline> s_Pipelines[4];

void CFogVolumePlaneShader::Initialize() {
  s_Pipelines[0] = hecl::conv->convert(Shader_CFogVolumePlaneShader0{});
  s_Pipelines[1] = hecl::conv->convert(Shader_CFogVolumePlaneShader1{});
  s_Pipelines[2] = hecl::conv->convert(Shader_CFogVolumePlaneShader2{});
  s_Pipelines[3] = hecl::conv->convert(Shader_CFogVolumePlaneShader3{});
}

void CFogVolumePlaneShader::Shutdown() {
  s_Pipelines[0].reset();
  s_Pipelines[1].reset();
  s_Pipelines[2].reset();
  s_Pipelines[3].reset();
}

void CFogVolumePlaneShader::CommitResources(size_t capacity) {
  m_vertCapacity = capacity;
  CGraphics::CommitResources([this, capacity](boo::IGraphicsDataFactory::Context& ctx) {
    m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(zeus::CVector4f), capacity);
    for (int i = 0; i < 4; ++i)
      m_dataBinds[i] = ctx.newShaderDataBinding(s_Pipelines[i], m_vbo.get(), nullptr, nullptr, 0, nullptr, nullptr,
                                                nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    return true;
  } BooTrace);
}

void CFogVolumePlaneShader::addFan(const zeus::CVector3f* verts, int numVerts) {
  zeus::CMatrix4f proj = CGraphics::GetPerspectiveProjectionMatrix(true);
  zeus::CVector4f vert0 = proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[0]);
  if (m_verts.size()) {
    m_verts.push_back(m_verts.back());
    m_verts.push_back(vert0);
    if (m_verts.size() & 1)
      m_verts.push_back(vert0);
  }
  TriFanToStrip<zeus::CVector4f> fanToStrip(m_verts);
  fanToStrip.AddVert(vert0);
  for (int i = 1; i < numVerts; ++i)
    fanToStrip.AddVert(proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[i]));
}

void CFogVolumePlaneShader::draw(int pass) {
  if (m_verts.empty())
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CFogVolumePlaneShader::draw", zeus::skMagenta);
  if (pass == 0) {
    if (m_vertCapacity < m_verts.size())
      CommitResources(m_verts.size());
    m_vbo->load(m_verts.data(), m_verts.size() * sizeof(zeus::CVector4f));
  }
  CGraphics::SetShaderDataBinding(m_dataBinds[pass]);
  CGraphics::DrawArray(0, m_verts.size());
}

} // namespace urde
