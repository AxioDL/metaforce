#pragma once

#include <array>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

namespace metaforce {
class CElementGen;

class CElementGenShaders {
public:
  enum class EShaderClass { Tex, IndTex, NoTex };

private:
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texZTestZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texNoZTestZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texZTestNoZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texNoZTestNoZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditiveZTest;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texAdditiveNoZTest;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texRedToAlphaZTest;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texRedToAlphaNoZTest;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texZTestNoZWriteSub;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texNoZTestNoZWriteSub;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texRedToAlphaZTestSub;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_texRedToAlphaNoZTestSub;

  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_indTexZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_indTexNoZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_indTexAdditive;

  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_cindTexZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_cindTexNoZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_cindTexAdditive;

  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexZTestZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexNoZTestZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexZTestNoZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexNoZTestNoZWrite;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditiveZTest;
  static inline std::array<boo::ObjToken<boo::IShaderPipeline>, 2> m_noTexAdditiveNoZTest;

public:
  static void Initialize();
  static void Shutdown();
  static EShaderClass GetShaderClass(CElementGen& gen);
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CElementGen& gen);
};

} // namespace metaforce
