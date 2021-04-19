#pragma once

#include "hsh/hsh.h"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"

namespace metaforce {

class CColoredStripShader {
public:
  enum class Mode { Alpha, Additive, FullAdditive, Subtractive };
  struct Vert {
    hsh::float3 m_pos;
    hsh::float4 m_color;
    hsh::float2 m_uv;
  };
  struct Uniform {
    hsh::float4x4 m_matrix;
    hsh::float4 m_color;
  };

private:
  hsh::dynamic_owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform{};

public:
  CColoredStripShader(size_t maxVerts, Mode mode, hsh::texture2d tex);
  void draw(const zeus::CColor& color, size_t numVerts, const Vert* verts);
};

} // namespace urde
