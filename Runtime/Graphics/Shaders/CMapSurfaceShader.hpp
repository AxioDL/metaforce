#pragma once

#include "Runtime/GCNTypes.hpp"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"

#include "hsh/hsh.h"

namespace urde {

class CMapSurfaceShader {
public:
  struct Uniform {
    hsh::float4x4 mtx;
    hsh::float4 color;
  };
  struct Vert {
    hsh::float3 pos;
    Vert() = default;
    Vert(float x, float y, float z) : pos(x, y, z) {}
  };

private:
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;

public:
  CMapSurfaceShader(hsh::vertex_buffer<Vert> vbo,
                    hsh::index_buffer<uint32_t> ibo);
  void draw(const zeus::CColor& color, u32 start, u32 count);
};

} // namespace urde
