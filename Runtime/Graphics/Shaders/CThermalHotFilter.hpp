#pragma once

#include <array>

#include "hsh/hsh.h"
#include "zeus/CColor.hpp"

namespace metaforce {

class CThermalHotFilter {
public:
  struct Uniform {
    std::array<hsh::float4, 3> m_colorRegs;
  };
  struct Vert {
    hsh::float2 m_pos;
    hsh::float2 m_uv;
  };

private:
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform{};

public:
  CThermalHotFilter();
  void setColorA(const zeus::CColor& color) { m_uniform.m_colorRegs[0] = color; }
  void setColorB(const zeus::CColor& color) { m_uniform.m_colorRegs[1] = color; }
  void setColorC(const zeus::CColor& color) { m_uniform.m_colorRegs[2] = color; }
  void draw();
};

} // namespace metaforce
