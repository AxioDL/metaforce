#pragma once

#include <array>

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"

namespace urde {

class CThermalColdFilter {
  struct Uniform {
    zeus::CMatrix4f m_indMtx;
    std::array<zeus::CColor, 3> m_colorRegs;
    float m_randOff = 0.f;
  };
  struct Vert {
    zeus::CVector2f m_pos;
    zeus::CVector2f m_uv;
    zeus::CVector2f m_uvNoise;
  };
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;

public:
  CThermalColdFilter();
  void setNoiseOffset(unsigned shift) { m_uniform.m_randOff = float(shift); }
  void setColorA(const zeus::CColor& color) { m_uniform.m_colorRegs[0] = color; }
  void setColorB(const zeus::CColor& color) { m_uniform.m_colorRegs[1] = color; }
  void setColorC(const zeus::CColor& color) { m_uniform.m_colorRegs[2] = color; }
  void setScale(float scale) {
    scale = 0.025f * (1.f - scale);
    m_uniform.m_indMtx[0][0] = scale;
    m_uniform.m_indMtx[1][1] = scale;
  }
  void draw();
};

} // namespace urde
