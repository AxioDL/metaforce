#pragma once

#include <array>
#include <vector>

#include "hsh/hsh.h"

namespace metaforce {
class CTexture;

class CRadarPaintShader {
public:
  struct Instance {
    std::array<hsh::float3, 4> pos;
    std::array<hsh::float2, 4> uv;
    hsh::float4 color;
  };
  struct Uniform {
    hsh::float4x4 xf;
  };

private:
  hsh::dynamic_owner<hsh::vertex_buffer<Instance>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  const CTexture* m_tex = nullptr;
  size_t m_maxInsts = 0;

public:
  void draw(const std::vector<Instance>& instances, const CTexture* tex);
};

} // namespace metaforce
