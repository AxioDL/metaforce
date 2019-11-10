#include "Runtime/Graphics/Shaders/CEnvFxShaders.hpp"

#include "Runtime/World/CEnvFxManager.hpp"

#include <hecl/Pipeline.hpp>

namespace urde {
boo::ObjToken<boo::IShaderPipeline> CEnvFxShaders::m_snowPipeline;
boo::ObjToken<boo::IShaderPipeline> CEnvFxShaders::m_underwaterPipeline;

void CEnvFxShaders::Initialize() {
  m_snowPipeline = hecl::conv->convert(Shader_CEnvFxSnowShader{});
  m_underwaterPipeline = hecl::conv->convert(Shader_CEnvFxUnderwaterShader{});
}

void CEnvFxShaders::Shutdown() {
  m_snowPipeline.reset();
  m_underwaterPipeline.reset();
}

void CEnvFxShaders::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CEnvFxManager& fxMgr,
                                           CEnvFxManagerGrid& grid) {
  const auto uBufInfo = grid.m_uniformBuf.getBufferInfo();
  const auto iBufInfo = grid.m_instBuf.getBufferInfo();

  const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 2> uniforms{{
      uBufInfo.first.get(),
      fxMgr.m_fogUniformBuf.get(),
  }};
  const std::array<size_t, 2> ubufOffsets{
      size_t(uBufInfo.second),
      0,
  };
  constexpr std::array<size_t, 2> ubufSizes{
      sizeof(CEnvFxShaders::Uniform),
      sizeof(CGraphics::g_Fog),
  };
  constexpr std::array<boo::PipelineStage, 2> uniformStages{
      boo::PipelineStage::Vertex,
      boo::PipelineStage::Fragment,
  };
  std::array<boo::ObjToken<boo::ITexture>, 2> textures{
      fxMgr.xb74_txtrSnowFlake->GetBooTexture(),
      fxMgr.x40_txtrEnvGradient->GetBooTexture(),
  };

  grid.m_snowBinding = ctx.newShaderDataBinding(
      m_snowPipeline, nullptr, iBufInfo.first.get(), nullptr, uniforms.size(), uniforms.data(), uniformStages.data(),
      ubufOffsets.data(), ubufSizes.data(), textures.size(), textures.data(), nullptr, nullptr, 0, iBufInfo.second);
  textures[0] = fxMgr.xc48_underwaterFlake->GetBooTexture();
  grid.m_underwaterBinding =
      ctx.newShaderDataBinding(m_underwaterPipeline, nullptr, iBufInfo.first.get(), nullptr, uniforms.size(),
                               uniforms.data(), uniformStages.data(), ubufOffsets.data(), ubufSizes.data(),
                               textures.size(), textures.data(), nullptr, nullptr, 0, iBufInfo.second);
}

} // namespace urde
