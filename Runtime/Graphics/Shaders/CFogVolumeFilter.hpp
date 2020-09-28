#pragma once

#include "hsh/hsh.h"

namespace zeus {
class CColor;
}

namespace urde {

class CFogVolumeFilter {
  struct Vert {
    hsh::float2 m_pos;
    hsh::float2 m_uv;
  };
  struct Uniform {
    hsh::float4 m_color;
  };

  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind1Way;
  hsh::binding m_dataBind2Way;

public:
  CFogVolumeFilter();
  void draw2WayPass(const zeus::CColor& color);
  void draw1WayPass(const zeus::CColor& color);
};

} // namespace urde
