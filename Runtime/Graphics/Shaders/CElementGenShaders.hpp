#pragma once

#include <array>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

namespace urde {
class CElementGen;

class CElementGenShaders {
public:
  enum class EShaderClass { Tex, IndTex, NoTex };

private:
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texZTestZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texNoZTestZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texZTestNoZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texNoZTestNoZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditiveZTest;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditiveNoZTest;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texRedToAlphaZTest;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texRedToAlphaNoZTest;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texZTestNoZWriteSub;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texNoZTestNoZWriteSub;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texRedToAlphaZTestSub;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texRedToAlphaNoZTestSub;

  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_indTexZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_indTexNoZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_indTexAdditive;

  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_cindTexZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_cindTexNoZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_cindTexAdditive;

  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexZTestZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexNoZTestZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexZTestNoZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexNoZTestNoZWrite;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditiveZTest;
  static std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditiveNoZTest;

public:
  static void Initialize();
  static void Shutdown();
  static EShaderClass GetShaderClass(CElementGen& gen);
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CElementGen& gen);
};

} // namespace urde
