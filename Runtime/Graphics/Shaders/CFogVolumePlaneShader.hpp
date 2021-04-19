#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "zeus/CRectangle.hpp"
#include "zeus/CVector4f.hpp"

#include "hsh/hsh.h"

namespace metaforce {

class CFogVolumePlaneShader {
public:
  struct Vert {
    hsh::float4 pos;
    Vert(hsh::float4 pos) : pos(pos) {}
  };

private:
  hsh::dynamic_owner<hsh::vertex_buffer<Vert>> m_vbo;
  std::array<hsh::binding, 4> m_dataBinds;
  std::vector<Vert> m_verts;
  size_t m_vertCapacity = 0;

  void CommitResources(size_t capacity);

public:
  static const zeus::CRectangle DefaultRect;
  void reset(size_t numVerts) {
    m_verts.clear();
    m_verts.reserve(numVerts);
  }
  void addFan(const zeus::CVector3f* verts, size_t numVerts);
  void draw(size_t pass);
};

} // namespace metaforce
