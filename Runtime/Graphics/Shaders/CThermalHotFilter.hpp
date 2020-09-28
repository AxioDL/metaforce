#pragma once

#include <array>

#include "zeus/CColor.hpp"

namespace urde {

class CThermalHotFilter {
  struct Uniform {
    std::array<zeus::CColor, 3> m_colorRegs;
  };
  struct Vert {
    zeus::CVector2f m_pos;
    zeus::CVector2f m_uv;
  };
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;

public:
  CThermalHotFilter();
  void setColorA(const zeus::CColor& color) { m_uniform.m_colorRegs[0] = color; }
  void setColorB(const zeus::CColor& color) { m_uniform.m_colorRegs[1] = color; }
  void setColorC(const zeus::CColor& color) { m_uniform.m_colorRegs[2] = color; }
  void draw();
};

} // namespace urde
