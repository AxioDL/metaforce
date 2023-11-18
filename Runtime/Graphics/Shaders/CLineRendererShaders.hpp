#pragma once

#include <array>

#include "Runtime/Graphics/CGraphics.hpp"

namespace metaforce {
class CLineRenderer;

class CLineRendererShaders {
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAlpha;
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditive;
//
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAlpha;
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditive;
//
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAlphaZ;
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditiveZ;
//
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAlphaZ;
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditiveZ;
//
//  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAlphaZGEqual;

public:
  static void Initialize();
  static void Shutdown();
  static void BuildShaderDataBinding(CLineRenderer& renderer,
                                     CTexture& texture, bool additive, bool zTest,
                                     bool zGEqual);
};

} // namespace metaforce
