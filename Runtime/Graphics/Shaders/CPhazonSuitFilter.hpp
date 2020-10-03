#pragma once

#include "hsh/hsh.h"

namespace zeus {
class CColor;
} // namespace zeus

namespace urde {
class CTexture;

class CPhazonSuitFilter {
public:
  struct BlurUniform {
    hsh::float4 blur;
  };
  struct Uniform {
    hsh::float4 color;
    hsh::float4 indScaleOff;
  };
  struct BlurVert {
    hsh::float3 pos;
    hsh::float2 uv;
  };
  struct Vert {
    hsh::float3 pos;
    hsh::float2 screenUv;
    hsh::float2 indUv;
    hsh::float2 maskUv;
  };

private:
  hsh::dynamic_owner<hsh::uniform_buffer<BlurUniform>> m_uniBufBlurX, m_uniBufBlurY;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::owner<hsh::vertex_buffer<BlurVert>> m_blurVbo;
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  const CTexture* m_indTex = nullptr;
  hsh::binding m_dataBindBlurX, m_dataBindBlurY, m_dataBind;

public:
  void drawBlurPasses(float radius, const CTexture* indTex);
  void draw(const zeus::CColor& color, float indScale, float indOffX, float indOffY);
};

} // namespace urde
