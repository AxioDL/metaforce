#pragma once

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <zeus/CColor.hpp>

namespace urde {
class CTexture;

class CPhazonSuitFilter {
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBufBlurX;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBufBlurY;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IGraphicsBufferS> m_blurVbo;
  boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
  const CTexture* m_indTex = nullptr;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBindBlurX;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBindBlurY;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;

public:
  static void Initialize();
  static void Shutdown();
  void drawBlurPasses(float radius, const CTexture* indTex);
  void draw(const zeus::CColor& color, float indScale, float indOffX, float indOffY);
};

} // namespace urde
