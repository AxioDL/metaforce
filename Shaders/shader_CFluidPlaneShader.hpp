#pragma once
#include "hecl/PipelineBase.hpp"
#include "athena/Global.hpp"
#include "hecl/hecl.hpp"

#ifndef URDE_MAX_LIGHTS
#define URDE_MAX_LIGHTS 8
#endif

enum class EFluidType { NormalWater, PoisonWater, Lava, PhazonFluid, Four, ThickLava };

struct SFluidPlaneShaderInfo {
  EFluidType m_type;
  bool m_hasPatternTex1;
  bool m_hasPatternTex2;
  bool m_hasColorTex;
  bool m_hasBumpMap;
  bool m_hasEnvMap;
  bool m_hasEnvBumpMap;
  bool m_hasLightmap;
  bool m_tessellation;
  bool m_doubleLightmapBlend;
  bool m_additive;

  SFluidPlaneShaderInfo(EFluidType type, bool hasPatternTex1, bool hasPatternTex2, bool hasColorTex, bool hasBumpMap,
                        bool hasEnvMap, bool hasEnvBumpMap, bool hasLightmap, bool tessellation,
                        bool doubleLightmapBlend, bool additive)
  : m_type(type)
  , m_hasPatternTex1(hasPatternTex1)
  , m_hasPatternTex2(hasPatternTex2)
  , m_hasColorTex(hasColorTex)
  , m_hasBumpMap(hasBumpMap)
  , m_hasEnvMap(hasEnvMap)
  , m_hasEnvBumpMap(hasEnvBumpMap)
  , m_hasLightmap(hasLightmap)
  , m_tessellation(tessellation)
  , m_doubleLightmapBlend(doubleLightmapBlend)
  , m_additive(additive) {}
};

struct SFluidPlaneDoorShaderInfo {
  bool m_hasPatternTex1;
  bool m_hasPatternTex2;
  bool m_hasColorTex;

  SFluidPlaneDoorShaderInfo(bool hasPatternTex1, bool hasPatternTex2, bool hasColorTex)
  : m_hasPatternTex1(hasPatternTex1), m_hasPatternTex2(hasPatternTex2), m_hasColorTex(hasColorTex) {}
};

class Shader_CFluidPlaneShader : public hecl::TessellationShader {
  friend class Shader_CFluidPlaneDoorShader;
  static const boo::VertexElementDescriptor VtxFmtElems[5];
  static const boo::VertexElementDescriptor TessVtxFmtElems[3];
  const SFluidPlaneShaderInfo& m_info;

public:
  Shader_CFluidPlaneShader(const SFluidPlaneShaderInfo& in, bool tessellation)
  : m_info(in)
  , VtxFmt(tessellation ? boo::VertexFormatInfo(TessVtxFmtElems) : boo::VertexFormatInfo(VtxFmtElems))
  , PipelineInfo({in.m_additive ? boo::BlendFactor::One : boo::BlendFactor::SrcAlpha,
                  in.m_additive ? boo::BlendFactor::One : boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                  boo::ZTest::LEqual, false, true, false, boo::CullMode::None, tessellation ? 1u : 0u})
  , HasTessellation(tessellation) {}

  const boo::VertexFormatInfo VtxFmt;
  const boo::AdditionalPipelineInfo PipelineInfo;
  bool HasTessellation;
  static constexpr bool HasHash = false;
  static constexpr bool HasStageHash = false;
  const SFluidPlaneShaderInfo& info() const { return m_info; }
};

template <typename P, typename S>
class StageObject_CFluidPlaneShader : public hecl::StageBinary<P, S> {
  static std::string BuildShader(const SFluidPlaneShaderInfo& in, bool tessellation);

public:
  StageObject_CFluidPlaneShader(hecl::StageConverter<P, S>& conv, hecl::FactoryCtx& ctx,
                                const Shader_CFluidPlaneShader& in)
  : hecl::StageBinary<P, S>(conv, ctx, hecl::StageSourceText<P, S>(BuildShader(in.info(), in.HasTessellation))) {}
};

class Shader_CFluidPlaneDoorShader : public hecl::GeneralShader {
  const SFluidPlaneDoorShaderInfo& m_info;

public:
  explicit Shader_CFluidPlaneDoorShader(const SFluidPlaneDoorShaderInfo& in)
  : m_info(in)
  , VtxFmt(boo::VertexFormatInfo(Shader_CFluidPlaneShader::VtxFmtElems))
  , PipelineInfo({boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                  boo::ZTest::LEqual, false, true, false, boo::CullMode::None}) {}

  const boo::VertexFormatInfo VtxFmt;
  const boo::AdditionalPipelineInfo PipelineInfo;
  static constexpr bool HasHash = false;
  static constexpr bool HasStageHash = false;
  const SFluidPlaneDoorShaderInfo& info() const { return m_info; }
};

template <typename P, typename S>
class StageObject_CFluidPlaneDoorShader : public hecl::StageBinary<P, S> {
  static std::string BuildShader(const SFluidPlaneDoorShaderInfo& in);

public:
  StageObject_CFluidPlaneDoorShader(hecl::StageConverter<P, S>& conv, hecl::FactoryCtx& ctx,
                                    const Shader_CFluidPlaneDoorShader& in)
  : hecl::StageBinary<P, S>(conv, ctx, hecl::StageSourceText<P, S>(BuildShader(in.info()))) {}
};

#define UNIVERSAL_PIPELINES_shader_CFluidPlaneShader ::Shader_CFluidPlaneShader ::Shader_CFluidPlaneDoorShader
#define STAGES_shader_CFluidPlaneShader(P, S)                                                                          \
  ::StageObject_CFluidPlaneShader<P, S>, ::StageObject_CFluidPlaneDoorShader<P, S>,
