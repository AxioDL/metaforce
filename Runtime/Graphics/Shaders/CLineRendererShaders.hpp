#pragma once

#include <array>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

namespace urde {
class CLineRenderer;

class CLineRendererShaders {
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAlpha;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditive;

  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAlpha;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditive;

  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAlphaZ;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditiveZ;

  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAlphaZ;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditiveZ;

  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAlphaZGEqual;

public:
  static void Initialize();
  static void Shutdown();
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CLineRenderer& renderer,
                                     const boo::ObjToken<boo::ITexture>& texture, bool additive, bool zTest,
                                     bool zGEqual);
};

} // namespace urde
