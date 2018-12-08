#pragma once

#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde {
class CElementGen;

class CElementGenShaders {
public:
  enum class EShaderClass { Tex, IndTex, NoTex };

private:
  static boo::ObjToken<boo::IShaderPipeline> m_texZTestZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_texNoZTestZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_texZTestNoZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_texNoZTestNoZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_texAdditiveZTest;
  static boo::ObjToken<boo::IShaderPipeline> m_texAdditiveNoZTest;
  static boo::ObjToken<boo::IShaderPipeline> m_texRedToAlphaZTest;
  static boo::ObjToken<boo::IShaderPipeline> m_texRedToAlphaNoZTest;
  static boo::ObjToken<boo::IShaderPipeline> m_texZTestNoZWriteSub;
  static boo::ObjToken<boo::IShaderPipeline> m_texNoZTestNoZWriteSub;
  static boo::ObjToken<boo::IShaderPipeline> m_texRedToAlphaZTestSub;
  static boo::ObjToken<boo::IShaderPipeline> m_texRedToAlphaNoZTestSub;

  static boo::ObjToken<boo::IShaderPipeline> m_indTexZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_indTexNoZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_indTexAdditive;

  static boo::ObjToken<boo::IShaderPipeline> m_cindTexZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_cindTexNoZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_cindTexAdditive;

  static boo::ObjToken<boo::IShaderPipeline> m_noTexZTestZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexNoZTestZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexZTestNoZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexNoZTestNoZWrite;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditiveZTest;
  static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditiveNoZTest;

public:
  static void Initialize();
  static void Shutdown();
  static EShaderClass GetShaderClass(CElementGen& gen);
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CElementGen& gen);
};

} // namespace urde
