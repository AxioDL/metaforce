#pragma once

#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde {
class CParticleSwoosh;

class CParticleSwooshShaders {
public:
  enum class EShaderClass { Tex, NoTex };

  struct Vert {
    zeus::CVector3f m_pos;
    zeus::CVector2f m_uv;
    zeus::CColor m_color;
  };

private:
  static boo::ObjToken<boo::IShaderPipeline> m_texZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_texNoZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_texAdditiveZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_texAdditiveNoZWrite;

  static boo::ObjToken<boo::IShaderPipeline> m_noTexZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexNoZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditiveZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditiveNoZWrite;

public:
  static void Initialize();
  static void Shutdown();
  static EShaderClass GetShaderClass(CParticleSwoosh& gen);
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CParticleSwoosh& gen);
};

} // namespace urde
