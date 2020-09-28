#pragma once

#include <array>
#include <vector>

#include "zeus/CColor.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CVector3f.hpp"

namespace urde {
class CTexture;

class CRadarPaintShader {
public:
  struct Instance {
    std::array<zeus::CVector3f, 4> pos;
    std::array<zeus::CVector2f, 4> uv;
    zeus::CColor color;
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

} // namespace urde
