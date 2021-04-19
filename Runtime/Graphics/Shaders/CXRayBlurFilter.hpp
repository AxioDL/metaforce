#pragma once

#include <array>

#include "Runtime/CToken.hpp"
#include "zeus/CMatrix4f.hpp"

#include "hsh/hsh.h"

namespace metaforce {
class CTexture;

class CXRayBlurFilter {
public:
  struct Vert {
    hsh::float2 m_pos;
    hsh::float2 m_uv;
  };
  struct Uniform {
    std::array<hsh::float4x4, 8> m_uv;
  };

private:
  TLockedToken<CTexture> m_paletteTex;
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;

public:
  explicit CXRayBlurFilter(TLockedToken<CTexture>& tex);
  void draw(float amount);
};

} // namespace metaforce
