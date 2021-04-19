#pragma once

#include <array>
#include <vector>

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CVector3f.hpp"

#include "hsh/hsh.h"

namespace metaforce {
class CTexture;

class CEnergyBarShader {
public:
  struct Vertex {
    hsh::float3 pos;
    hsh::float2 uv;
  };
  struct Uniform {
    hsh::float4x4 m_matrix;
    hsh::float4 m_color;
  };

private:
  hsh::dynamic_owner<hsh::vertex_buffer<Vertex>> m_vbo;
  std::array<hsh::dynamic_owner<hsh::uniform_buffer<Uniform>>, 3> m_uniBuf;
  std::array<hsh::binding, 3> m_dataBind;
  Uniform m_uniform;
  size_t m_maxVerts = 0;

public:
  void updateModelMatrix();
  void draw(const zeus::CColor& color0, const std::vector<Vertex>& verts0, const zeus::CColor& color1,
            const std::vector<Vertex>& verts1, const zeus::CColor& color2, const std::vector<Vertex>& verts2,
            hsh::texture2d tex);
};

} // namespace metaforce
