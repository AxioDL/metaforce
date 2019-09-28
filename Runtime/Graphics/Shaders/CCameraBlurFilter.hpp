#pragma once

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CVector3f.hpp>
#include <zeus/CVector4f.hpp>

namespace urde {

class CCameraBlurFilter {
  struct Vert {
    zeus::CVector2f m_pos;
    zeus::CVector2f m_uv;
  };

  struct Uniform {
    zeus::CVector4f m_uv[6];
    float m_opacity = 1.f;
  };
  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;

public:
  static void Initialize();
  static void Shutdown();
  CCameraBlurFilter();
  void draw(float amount, bool clearDepth = false);
};

} // namespace urde
