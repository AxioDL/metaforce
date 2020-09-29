#pragma once

#include "hsh/hsh.h"

namespace zeus {
class CColor;
class CAABox;
} // namespace zeus

namespace urde {
class CAABoxShader {
public:
  struct Vert {
    hsh::float3 m_pos;
  };
  struct Uniform {
    hsh::float4x4 m_xf;
    hsh::float4 m_color;
  };

private:
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform{};

public:
  CAABoxShader(const zeus::CAABox& aabb, bool zOnly = false);
  void draw(const zeus::CColor& color);
};

} // namespace urde
