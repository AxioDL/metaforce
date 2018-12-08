#pragma once

#include "CColoredQuadFilter.hpp"
#include "CTexturedQuadFilter.hpp"

namespace urde {

class CWorldShadowShader {
  boo::ObjToken<boo::ITextureR> m_tex;
  std::experimental::optional<CTexturedQuadFilter> m_prevQuad;
  u32 m_w, m_h;

  struct Uniform {
    zeus::CMatrix4f m_matrix;
    zeus::CColor m_color;
  };
  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  boo::ObjToken<boo::IShaderDataBinding> m_zDataBind;
  Uniform m_uniform;

public:
  static void Initialize();
  static void Shutdown();
  CWorldShadowShader(u32 w, u32 h);
  void bindRenderTarget();
  void drawBase(float extent);
  void lightenShadow();
  void blendPreviousShadow();
  void resolveTexture();

  u32 GetWidth() const { return m_w; }
  u32 GetHeight() const { return m_h; }

  const boo::ObjToken<boo::ITextureR>& GetTexture() const { return m_tex; }
};

} // namespace urde
