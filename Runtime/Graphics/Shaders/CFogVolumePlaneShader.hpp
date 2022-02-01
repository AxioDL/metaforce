#pragma once

#include <array>
#include <cstddef>
#include <vector>

//#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CRectangle.hpp>
#include <zeus/CVector4f.hpp>

namespace zeus {
class CVector3f;
} // namespace zeus

namespace metaforce {

class CFogVolumePlaneShader {
//  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
//  std::array<boo::ObjToken<boo::IShaderDataBinding>, 4> m_dataBinds;
  size_t m_vertCapacity = 0;
public:
  std::vector<zeus::CVector4f> m_verts;

public:
  static const zeus::CRectangle DefaultRect;
  void reset(int numVerts) {
    m_verts.clear();
    m_verts.reserve(numVerts);
  }
  void addFan(const zeus::CVector3f* verts, int numVerts);
  void draw(int pass);
};

} // namespace metaforce
