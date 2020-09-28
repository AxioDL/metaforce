#include "Runtime/Graphics/Shaders/CFogVolumePlaneShader.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

#include "CFogVolumePlaneShader.cpp.hshhead"

namespace urde {
using namespace hsh::pipeline;

template <size_t I>
struct CFogVolumePlanePipelineConfig : pipeline<color_attachment<>> {};

using AlphaOnlyAttachment =
    color_attachment<hsh::Zero, hsh::Zero, hsh::Add, hsh::One, hsh::Zero, hsh::Add, hsh::CC_Alpha>;

template <>
struct CFogVolumePlanePipelineConfig<0>
: pipeline<AlphaOnlyAttachment, depth_compare<hsh::LEqual>, depth_write<true>, cull_mode<hsh::CullFront>> {};

template <>
struct CFogVolumePlanePipelineConfig<1>
: pipeline<AlphaOnlyAttachment, depth_compare<hsh::Always>, depth_write<false>, cull_mode<hsh::CullFront>> {};

template <>
struct CFogVolumePlanePipelineConfig<2>
: pipeline<AlphaOnlyAttachment, depth_compare<hsh::LEqual>, depth_write<true>, cull_mode<hsh::CullBack>> {};

template <>
struct CFogVolumePlanePipelineConfig<3>
: pipeline<AlphaOnlyAttachment, depth_compare<hsh::Greater>, depth_write<false>, cull_mode<hsh::CullBack>> {};

template <size_t I>
struct CFogVolumePlanePipeline : CFogVolumePlanePipelineConfig<I> {
  CFogVolumePlanePipeline(hsh::vertex_buffer<CFogVolumePlaneShader::Vert> vbo) {
    this->position = vbo->pos;
    this->color_out[0] = hsh::float4(1.f);
  }
};

template struct CFogVolumePlanePipeline<0>;
template struct CFogVolumePlanePipeline<1>;
template struct CFogVolumePlanePipeline<2>;
template struct CFogVolumePlanePipeline<3>;

void CFogVolumePlaneShader::CommitResources(size_t capacity) {
  m_vertCapacity = capacity;
  m_vbo = hsh::create_dynamic_vertex_buffer<Vert>(capacity);
  for (size_t i = 0; i < m_dataBinds.size(); ++i) {
    m_dataBinds[i].hsh_bind(CFogVolumePlanePipeline<i>(m_vbo.get()));
  }
}

void CFogVolumePlaneShader::addFan(const zeus::CVector3f* verts, size_t numVerts) {
  zeus::CMatrix4f proj = CGraphics::GetPerspectiveProjectionMatrix(true);
  zeus::CVector4f vert0 = proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[0]);
  if (!m_verts.empty()) {
    m_verts.push_back(m_verts.back());
    m_verts.emplace_back(vert0);
    if ((m_verts.size() & size_t(1)) != 0) {
      m_verts.emplace_back(vert0);
    }
  }
  TriFanToStrip<Vert> fanToStrip(m_verts);
  fanToStrip.EmplaceVert(vert0);
  for (size_t i = 1; i < numVerts; ++i) {
    fanToStrip.EmplaceVert(proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[i]));
  }
}

void CFogVolumePlaneShader::draw(size_t pass) {
  if (m_verts.empty()) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP("CFogVolumePlaneShader::draw", zeus::skMagenta);
  if (pass == 0) {
    if (m_vertCapacity < m_verts.size()) {
      CommitResources(m_verts.size());
    }
    m_vbo.load(m_verts);
  }
  m_dataBinds[pass].draw(0, m_verts.size());
}

} // namespace urde
