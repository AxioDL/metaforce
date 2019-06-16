#pragma once

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CAABox.hpp"

namespace urde {

class CAABoxShader {
  struct Uniform {
    zeus::CMatrix4f m_xf;
    zeus::CColor m_color;
  };
  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;

public:
  static void Initialize();
  static void Shutdown();
  CAABoxShader(bool zOnly = false);
  void setAABB(const zeus::CAABox& aabb);
  void draw(const zeus::CColor& color);
};

} // namespace urde
