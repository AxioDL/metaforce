#pragma once

#include <array>

#include "Runtime/GCNTypes.hpp"

#include "zeus/CVector3f.hpp"

namespace urde {

class CSpaceWarpFilter {
  struct Uniform {
    hsh::float4x4 m_matrix;
    hsh::float4x4 m_indXf;
    hsh::float3 m_strength;
  };
  struct Vert {
    hsh::float2 m_pos;
    hsh::float2 m_uv;
  };
  std::array<std::array<std::array<u8, 4>, 8>, 4> m_shiftTexture{};
  hsh::owner<hsh::texture2d> m_warpTex;
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;
  float m_strength = 1.f;

  void GenerateWarpRampTex();

public:
  CSpaceWarpFilter();
  void setStrength(float strength) { m_strength = strength; }
  void draw(const zeus::CVector3f& pt);
};

} // namespace urde
