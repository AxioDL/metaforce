#pragma once

#include <array>

#include "Runtime/CToken.hpp"

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <zeus/CMatrix4f.hpp>

namespace urde {
class CTexture;

class CXRayBlurFilter {
  struct Uniform {
    std::array<zeus::CMatrix4f, 8> m_uv;
  };
  TLockedToken<CTexture> m_paletteTex;
  boo::ObjToken<boo::ITexture> m_booTex;
  boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;

public:
  static void Initialize();
  static void Shutdown();
  CXRayBlurFilter(TLockedToken<CTexture>& tex);
  void draw(float amount);
};

} // namespace urde
