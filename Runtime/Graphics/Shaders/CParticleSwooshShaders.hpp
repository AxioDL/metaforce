#pragma once

#include <array>

#include "hsh/hsh.h"

namespace urde {
class CParticleSwoosh;

class CParticleSwooshShaders {
public:
  enum class EShaderClass { Tex, NoTex };

  struct Vert {
    hsh::float3 m_pos;
    hsh::float2 m_uv;
    hsh::float4 m_color;
  };

  struct Uniform {
    hsh::float4x4 m_xf;
  };

  static EShaderClass GetShaderClass(CParticleSwoosh& gen);
  static void BuildShaderDataBinding(CParticleSwoosh& gen);
};

} // namespace urde
