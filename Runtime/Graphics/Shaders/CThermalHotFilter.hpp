#pragma once

#include <array>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>

namespace urde {

class CThermalHotFilter {
  struct Uniform {
    std::array<zeus::CColor, 3> m_colorRegs;
  };
  boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;

public:
  static void Initialize();
  static void Shutdown();
  CThermalHotFilter();
  void setColorA(const zeus::CColor& color) { m_uniform.m_colorRegs[0] = color; }
  void setColorB(const zeus::CColor& color) { m_uniform.m_colorRegs[1] = color; }
  void setColorC(const zeus::CColor& color) { m_uniform.m_colorRegs[2] = color; }
  void draw();
};

} // namespace urde
