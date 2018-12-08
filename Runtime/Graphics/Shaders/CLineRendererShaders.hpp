#pragma once

#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde {
class CLineRenderer;

class CLineRendererShaders {
  static boo::ObjToken<boo::IShaderPipeline> m_texAlpha;
  static boo::ObjToken<boo::IShaderPipeline> m_texAdditive;

  static boo::ObjToken<boo::IShaderPipeline> m_noTexAlpha;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditive;

  static boo::ObjToken<boo::IShaderPipeline> m_texAlphaZ;
  static boo::ObjToken<boo::IShaderPipeline> m_texAdditiveZ;

  static boo::ObjToken<boo::IShaderPipeline> m_noTexAlphaZ;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditiveZ;

  static boo::ObjToken<boo::IShaderPipeline> m_noTexAlphaZGEqual;

public:
  static void Initialize();
  static void Shutdown();
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CLineRenderer& renderer,
                                     const boo::ObjToken<boo::ITexture>& texture, bool additive, bool zTest,
                                     bool zGEqual);
};

} // namespace urde
