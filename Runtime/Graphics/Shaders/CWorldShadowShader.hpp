#pragma once

#include <optional>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Graphics/Shaders/CTexturedQuadFilter.hpp"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"

namespace metaforce {

class CWorldShadowShader {
public:
  struct Vert {
    hsh::float3 m_pos;
  };
  struct Uniform {
    hsh::float4x4 m_matrix;
    hsh::float4 m_color;
  };

private:
  hsh::owner<hsh::render_texture2d> m_tex;
  std::optional<CTexturedQuadFilter> m_prevQuad;
  u32 m_w, m_h;

  hsh::dynamic_owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  hsh::binding m_zDataBind;
  Uniform m_uniform{};

public:
  explicit CWorldShadowShader(u32 w, u32 h);
  void bindRenderTarget();
  void drawBase(float extent);
  void lightenShadow();
  void blendPreviousShadow();
  void resolveTexture();

  u32 GetWidth() const { return m_w; }
  u32 GetHeight() const { return m_h; }

  hsh::render_texture2d GetTexture() const { return m_tex.get_color(0); }
};

} // namespace metaforce
