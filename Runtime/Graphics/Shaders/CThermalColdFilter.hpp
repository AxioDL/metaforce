#pragma once

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>

namespace urde {

class CThermalColdFilter {
  struct Uniform {
    zeus::CMatrix4f m_indMtx;
    zeus::CColor m_colorRegs[3];
    float m_randOff = 0.f;
  };
  boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;

public:
  static void Initialize();
  static void Shutdown();
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
