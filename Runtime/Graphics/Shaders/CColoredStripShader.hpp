#pragma once

#include "Runtime/Graphics/CGraphics.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>

namespace metaforce {

class CColoredStripShader {
public:
  enum class Mode { Alpha, Additive, FullAdditive, Subtractive };

private:
  struct Uniform {
    zeus::CMatrix4f m_matrix;
    zeus::CColor m_color;
  };
//  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
//  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
//  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;

//  void BuildResources(boo::IGraphicsDataFactory::Context& ctx, size_t maxVerts, Mode mode,
//                      boo::ObjToken<boo::ITexture> tex);

public:
  struct Vert {
    zeus::CVector3f m_pos;
    zeus::CColor m_color;
    zeus::CVector2f m_uv;
  };
  static void Initialize();
  static void Shutdown();
  CColoredStripShader(size_t maxVerts, Mode mode, CTexture& tex);
  void draw(const zeus::CColor& color, size_t numVerts, const Vert* verts);
};

} // namespace metaforce
