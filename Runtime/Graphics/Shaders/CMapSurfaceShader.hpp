#pragma once

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>

namespace metaforce {

class CMapSurfaceShader {
  struct Uniform {
    zeus::CMatrix4f mtx;
    zeus::CColor color;
  };

//  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
//  boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
//  boo::ObjToken<boo::IGraphicsBufferS> m_ibo;
//  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;

public:
  static void Initialize();
  static void Shutdown();
  CMapSurfaceShader(aurora::ArrayRef<zeus::CVector3f> vbo,
                    aurora::ArrayRef<uint16_t> ibo);
  void draw(const zeus::CColor& color, u32 start, u32 count);
};

} // namespace metaforce
