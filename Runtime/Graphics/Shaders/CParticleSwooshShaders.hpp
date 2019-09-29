#pragma once

#include <array>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

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
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texNoZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditiveZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditiveNoZWrite;

  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexNoZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditiveZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditiveNoZWrite;

public:
  static void Initialize();
  static void Shutdown();
  static EShaderClass GetShaderClass(CParticleSwoosh& gen);
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CParticleSwoosh& gen);
};

} // namespace urde
