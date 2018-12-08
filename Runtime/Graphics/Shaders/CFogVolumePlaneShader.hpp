#pragma once

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "zeus/CVector4f.hpp"

namespace urde {

class CFogVolumePlaneShader {
  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBinds[4];
  std::vector<zeus::CVector4f> m_verts;
  size_t m_vertCapacity = 0;

  void CommitResources(size_t capacity);

public:
  static void Initialize();
  static void Shutdown();
  static const zeus::CRectangle DefaultRect;
  void reset(int numVerts) {
    m_verts.clear();
    m_verts.reserve(numVerts);
  }
  void addFan(const zeus::CVector3f* verts, int numVerts);
  void draw(int pass);
};

} // namespace urde
