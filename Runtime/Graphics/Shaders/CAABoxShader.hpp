#pragma once

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"

namespace zeus {
class CAABox;
}

namespace urde {

class CAABoxShader {
  struct Vert {
    hsh::float3 m_pos;
  };
  struct Uniform {
    hsh::float4x4 m_xf;
    hsh::float4 m_color;
  };
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;

public:
  explicit CAABoxShader(bool zOnly = false);
  void setAABB(const zeus::CAABox& aabb);
  void draw(const zeus::CColor& color);
};

} // namespace urde
