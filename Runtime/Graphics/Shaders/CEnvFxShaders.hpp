#pragma once

#include <array>

namespace urde {
class CEnvFxManager;
class CEnvFxManagerGrid;

class CEnvFxShaders {
public:
  struct Instance {
    std::array<hsh::float3, 4> positions;
    hsh::float4 color;
    std::array<hsh::float2, 4> uvs;
  };
  struct Uniform {
    hsh::float4x4 mv;
    hsh::float4x4 proj;
    hsh::float4x4 envMtx;
    hsh::float4 moduColor;
  };

  static void BuildShaderDataBinding(CEnvFxManager& fxMgr,
                                     CEnvFxManagerGrid& grid);
};

} // namespace urde
