#pragma once

#include "hsh/hsh.h"
#include "zeus/CVector4f.hpp"

namespace urde {

class CCameraBlurFilter {
  friend struct CCameraBlurFilterPipeline;

public:
  struct Vert {
    hsh::float2 m_pos;
    hsh::float2 m_uv;
  };
  struct Uniform {
    hsh::aligned_array<hsh::float2, 6> m_uv;
    float m_opacity = 1.f;
  };

private:
  hsh::dynamic_owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;

public:
  CCameraBlurFilter();
  void draw(float amount, bool clearDepth = false);
};

} // namespace urde
