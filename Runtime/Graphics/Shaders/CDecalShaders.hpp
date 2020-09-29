#pragma once

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

namespace urde {
struct CQuadDecal;

class CDecalShaders {
private:
  static inline boo::ObjToken<boo::IShaderPipeline> m_texZTestNoZWrite;
  static inline boo::ObjToken<boo::IShaderPipeline> m_texAdditiveZTest;
  static inline boo::ObjToken<boo::IShaderPipeline> m_texRedToAlphaZTest;

  static inline boo::ObjToken<boo::IShaderPipeline> m_noTexZTestNoZWrite;
  static inline boo::ObjToken<boo::IShaderPipeline> m_noTexAdditiveZTest;

public:
  static void Initialize();
  static void Shutdown();
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CQuadDecal& decal);
};

} // namespace urde
