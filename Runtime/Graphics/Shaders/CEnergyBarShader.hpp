#pragma once

#include <vector>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CTexture;

class CEnergyBarShader {
public:
  struct Vertex {
    zeus::CVector3f pos;
    zeus::CVector2f uv;
  };

private:
  struct Uniform {
    zeus::CMatrix4f m_matrix;
    zeus::CColor m_color;
  };
  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf[3];
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind[3];
  Uniform m_uniform;
  const CTexture* m_tex = nullptr;
  size_t m_maxVerts = 0;

public:
  static void Initialize();
  static void Shutdown();
  void updateModelMatrix();
  void draw(const zeus::CColor& color0, const std::vector<Vertex>& verts0, const zeus::CColor& color1,
            const std::vector<Vertex>& verts1, const zeus::CColor& color2, const std::vector<Vertex>& verts2,
            const CTexture* tex);
};

} // namespace urde
