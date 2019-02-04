#pragma once

#include <type_traits>
#include <cassert>
#include "hecl/hecl.hpp"
#include "hecl/Backend/GLSL.hpp"
#include "hecl/Backend/HLSL.hpp"
#include "hecl/Backend/Metal.hpp"
#include "PipelineBase.hpp"

/* CMake-curated rep classes for the application */
#include "ApplicationReps.hpp"

namespace hecl {

#if HECL_RUNTIME
template <typename P, typename S>
class StageRuntimeObject : public StageRep<P, S> {
  boo::ObjToken<boo::IShaderStage> m_stage;

public:
  static constexpr StageTargetType TargetType = StageTargetType::Runtime;
  static constexpr PipelineTargetType PipelineTarget = PipelineTargetType::StageRuntimeCollection;
  static constexpr bool HasHash = false;
  StageRuntimeObject() = default;
  StageRuntimeObject(StageConverter<P, S>& conv, FactoryCtx& ctx, const StageBinary<P, S>& in) {
    m_stage = static_cast<typename P::Context&>(ctx).newShaderStage(in.data(), in.size(), S::Enum);
  }
  boo::ObjToken<boo::IShaderStage> stage() const { return m_stage; }
};
#endif

class HECLIR : public PipelineRep<PlatformType::Null> {
  const hecl::Backend::IR& m_ir;
  const hecl::Backend::ShaderTag& m_tag;
  const hecl::Backend::ExtensionSlot& m_extension;
  uint64_t m_hash;

public:
  HECLIR(const hecl::Backend::IR& ir, const hecl::Backend::ShaderTag& tag,
         const hecl::Backend::ExtensionSlot& extension)
  : m_ir(ir), m_tag(tag), m_extension(extension) {
    m_hash = tag.val64();
    m_hash ^= extension.hash();
  }
  static constexpr bool HasHash = true;
  uint64_t Hash() const { return m_hash; }

  const hecl::Backend::IR& ir() const { return m_ir; }
  const hecl::Backend::ShaderTag& tag() const { return m_tag; }
  const hecl::Backend::ExtensionSlot& extension() const { return m_extension; }
};

template <typename P, class BackendTp>
class HECLBackendImpl : public PipelineRep<P> {
  hecl::Backend::ShaderTag m_tag;
  BackendTp m_backend;
  const hecl::Backend::ExtensionSlot& m_extension;

public:
  static constexpr bool HasHash = false;
  HECLBackendImpl(PipelineConverter<P>& conv, FactoryCtx& ctx, const HECLIR& in)
  : m_tag(in.tag()), m_extension(in.extension()) {
    hecl::Backend::Diagnostics diag;
    m_backend.reset(in.ir(), diag);
  }
  std::string makeVert() const {
    return m_backend.makeVert(m_tag.getColorCount(), m_tag.getUvCount(), m_tag.getWeightCount(),
                              m_tag.getSkinSlotCount(), m_extension.texCount, m_extension.texs,
                              m_extension.noReflection ? Backend::ReflectionType::None : m_tag.getReflectionType());
  }
  std::string makeFrag() const {
    return m_backend.makeFrag(m_extension.blockCount, m_extension.blockNames,
                              m_tag.getAlphaTest() || m_extension.forceAlphaTest,
                              m_extension.noReflection ? Backend::ReflectionType::None : m_tag.getReflectionType(),
                              m_backend.m_blendSrc, m_backend.m_blendDst, m_extension.lighting, m_extension.post,
                              m_extension.texCount, m_extension.texs);
  }
  const hecl::Backend::ShaderTag& getTag() const { return m_tag; }
  const hecl::Backend::ExtensionSlot& extension() const { return m_extension; }
  std::pair<hecl::Backend::BlendFactor, hecl::Backend::BlendFactor> blendFactors() const {
    return {m_backend.m_blendSrc, m_backend.m_blendDst};
  }
};

template <typename P>
class HECLBackend : public PipelineRep<P> {
public:
  static constexpr bool HasHash = false;
};

template <>
class HECLBackend<PlatformType::OpenGL> : public HECLBackendImpl<PlatformType::OpenGL, hecl::Backend::GLSL> {
public:
  using HECLBackendImpl::HECLBackendImpl;
};

template <>
class HECLBackend<PlatformType::Vulkan> : public HECLBackendImpl<PlatformType::Vulkan, hecl::Backend::GLSL> {
public:
  using HECLBackendImpl::HECLBackendImpl;
};

template <>
class HECLBackend<PlatformType::D3D11> : public HECLBackendImpl<PlatformType::D3D11, hecl::Backend::HLSL> {
public:
  using HECLBackendImpl::HECLBackendImpl;
};

template <>
class HECLBackend<PlatformType::Metal> : public HECLBackendImpl<PlatformType::Metal, hecl::Backend::Metal> {
public:
  using HECLBackendImpl::HECLBackendImpl;
};

template <>
class HECLBackend<PlatformType::NX> : public HECLBackendImpl<PlatformType::NX, hecl::Backend::GLSL> {
public:
  using HECLBackendImpl::HECLBackendImpl;
};

template <template <typename, typename> class T, typename P, typename... Rest>
StageCollection<T<P, Rest...>>::StageCollection(PipelineConverter<P>& conv, FactoryCtx& ctx, const HECLBackend<P>& in) {
  m_vertex = conv.getVertexConverter().convert(ctx, StageSourceText<P, PipelineStage::Vertex>(in.makeVert()));
  m_fragment = conv.getFragmentConverter().convert(ctx, StageSourceText<P, PipelineStage::Fragment>(in.makeFrag()));
  m_vtxFmtData = in.getTag().vertexFormat();
  m_vtxFmt = boo::VertexFormatInfo(m_vtxFmtData.size(), m_vtxFmtData.data());
  m_additionalInfo = in.getTag().additionalInfo(in.extension(), in.blendFactors());
  MakeHash();
}

#if HECL_RUNTIME
template <typename P>
class FinalPipeline : public PipelineRep<P> {
  boo::ObjToken<boo::IShaderPipeline> m_pipeline;

public:
  static constexpr PipelineTargetType TargetType = PipelineTargetType::FinalPipeline;
  static constexpr bool HasHash = false;
  FinalPipeline(PipelineConverter<P>& conv, FactoryCtx& ctx,
                const StageCollection<StageRuntimeObject<P, PipelineStage::Null>>& in) {
    m_pipeline = static_cast<typename P::Context&>(ctx).newShaderPipeline(
        in.m_vertex.stage(), in.m_fragment.stage(), in.m_geometry.stage(), in.m_control.stage(),
        in.m_evaluation.stage(), in.m_vtxFmt, in.m_additionalInfo);
  }
  boo::ObjToken<boo::IShaderPipeline> pipeline() const { return m_pipeline; }
};
#endif

template <typename... Args>
struct pack {};
struct null_t {};
template <typename P>
struct ShaderDB {};

#define STAGE_COLLECTION_SPECIALIZATIONS(T, P) StageCollection<T<P, PipelineStage::Null>>,
#define PIPELINE_RUNTIME_SPECIALIZATIONS(P)                                                                            \
  HECLBackend<P>,                                                                                                      \
      STAGE_COLLECTION_SPECIALIZATIONS(StageSourceText, P) STAGE_COLLECTION_SPECIALIZATIONS(StageBinary, P)            \
          STAGE_COLLECTION_SPECIALIZATIONS(StageRuntimeObject, P) FinalPipeline<P>,
#define PIPELINE_OFFLINE_SPECIALIZATIONS(P) HECLBackend<P>, STAGE_COLLECTION_SPECIALIZATIONS(StageSourceText, P)
#define STAGE_RUNTIME_SPECIALIZATIONS(P, S)                                                                            \
  StageBinary<P, S>, HECL_APPLICATION_STAGE_REPS(P, S) StageRuntimeObject<P, S>,
#define STAGE_OFFLINE_SPECIALIZATIONS(P, S) HECL_APPLICATION_STAGE_REPS(P, S)

#define SPECIALIZE_RUNTIME_AVAILABLE_STAGE(P, S)                                                                       \
  template <>                                                                                                          \
  struct ShaderDB<P>::StageDB<S> {                                                                                     \
    using StageTypes = pack<STAGE_RUNTIME_SPECIALIZATIONS(P, S) null_t>;                                               \
  };
#define SPECIALIZE_RUNTIME_UNAVAILABLE_STAGE(P, S)                                                                     \
  template <>                                                                                                          \
  struct ShaderDB<P>::StageDB<S> {                                                                                     \
    using StageTypes = pack<null_t>;                                                                                   \
  };
#define SPECIALIZE_OFFLINE_STAGE(P, S)                                                                                 \
  template <>                                                                                                          \
  struct ShaderDB<P>::StageDB<S> {                                                                                     \
    using StageTypes = pack<STAGE_OFFLINE_SPECIALIZATIONS(P, S) null_t>;                                               \
  };
#define SPECIALIZE_RUNTIME_AVAILABLE_PLATFORM(P)                                                                       \
  template <>                                                                                                          \
  struct ShaderDB<P> {                                                                                                 \
    using PipelineTypes = pack<PIPELINE_RUNTIME_SPECIALIZATIONS(P) null_t>;                                            \
    template <typename S>                                                                                              \
    struct StageDB {};                                                                                                 \
  };                                                                                                                   \
  SPECIALIZE_RUNTIME_AVAILABLE_STAGE(P, hecl::PipelineStage::Vertex)                                                   \
  SPECIALIZE_RUNTIME_AVAILABLE_STAGE(P, hecl::PipelineStage::Fragment)                                                 \
  SPECIALIZE_RUNTIME_AVAILABLE_STAGE(P, hecl::PipelineStage::Geometry)                                                 \
  SPECIALIZE_RUNTIME_AVAILABLE_STAGE(P, hecl::PipelineStage::Control)                                                  \
  SPECIALIZE_RUNTIME_AVAILABLE_STAGE(P, hecl::PipelineStage::Evaluation)
#define SPECIALIZE_RUNTIME_UNAVAILABLE_PLATFORM(P)                                                                     \
  template <>                                                                                                          \
  struct ShaderDB<P> {                                                                                                 \
    using PipelineTypes = pack<null_t>;                                                                                \
    template <typename S>                                                                                              \
    struct StageDB {};                                                                                                 \
  };                                                                                                                   \
  SPECIALIZE_RUNTIME_UNAVAILABLE_STAGE(P, hecl::PipelineStage::Vertex)                                                 \
  SPECIALIZE_RUNTIME_UNAVAILABLE_STAGE(P, hecl::PipelineStage::Fragment)                                               \
  SPECIALIZE_RUNTIME_UNAVAILABLE_STAGE(P, hecl::PipelineStage::Geometry)                                               \
  SPECIALIZE_RUNTIME_UNAVAILABLE_STAGE(P, hecl::PipelineStage::Control)                                                \
  SPECIALIZE_RUNTIME_UNAVAILABLE_STAGE(P, hecl::PipelineStage::Evaluation)
#define SPECIALIZE_OFFLINE_PLATFORM(P)                                                                                 \
  template <>                                                                                                          \
  struct ShaderDB<P> {                                                                                                 \
    using PipelineTypes = pack<PIPELINE_OFFLINE_SPECIALIZATIONS(P) null_t>;                                            \
    template <typename S>                                                                                              \
    struct StageDB {};                                                                                                 \
  };                                                                                                                   \
  SPECIALIZE_OFFLINE_STAGE(P, hecl::PipelineStage::Vertex)                                                             \
  SPECIALIZE_OFFLINE_STAGE(P, hecl::PipelineStage::Fragment)                                                           \
  SPECIALIZE_OFFLINE_STAGE(P, hecl::PipelineStage::Geometry)                                                           \
  SPECIALIZE_OFFLINE_STAGE(P, hecl::PipelineStage::Control)                                                            \
  SPECIALIZE_OFFLINE_STAGE(P, hecl::PipelineStage::Evaluation)

#if HECL_RUNTIME
#if BOO_HAS_GL
SPECIALIZE_RUNTIME_AVAILABLE_PLATFORM(hecl::PlatformType::OpenGL)
#else
SPECIALIZE_RUNTIME_UNAVAILABLE_PLATFORM(hecl::PlatformType::OpenGL)
#endif
#if BOO_HAS_VULKAN
SPECIALIZE_RUNTIME_AVAILABLE_PLATFORM(hecl::PlatformType::Vulkan)
#else
SPECIALIZE_RUNTIME_UNAVAILABLE_PLATFORM(hecl::PlatformType::Vulkan)
#endif
#if _WIN32
SPECIALIZE_RUNTIME_AVAILABLE_PLATFORM(hecl::PlatformType::D3D11)
#else
SPECIALIZE_RUNTIME_UNAVAILABLE_PLATFORM(hecl::PlatformType::D3D11)
#endif
#if BOO_HAS_METAL
SPECIALIZE_RUNTIME_AVAILABLE_PLATFORM(hecl::PlatformType::Metal)
#else
SPECIALIZE_RUNTIME_UNAVAILABLE_PLATFORM(hecl::PlatformType::Metal)
#endif
#if BOO_HAS_NX
SPECIALIZE_RUNTIME_AVAILABLE_PLATFORM(hecl::PlatformType::NX)
#else
SPECIALIZE_RUNTIME_UNAVAILABLE_PLATFORM(hecl::PlatformType::NX)
#endif
#else
SPECIALIZE_OFFLINE_PLATFORM(hecl::PlatformType::OpenGL)
SPECIALIZE_OFFLINE_PLATFORM(hecl::PlatformType::Vulkan)
SPECIALIZE_OFFLINE_PLATFORM(hecl::PlatformType::D3D11)
SPECIALIZE_OFFLINE_PLATFORM(hecl::PlatformType::Metal)
SPECIALIZE_OFFLINE_PLATFORM(hecl::PlatformType::NX)
#endif

class ShaderCacheZipStream;

template <typename P, typename S>
class StageConverter {
  friend class PipelineConverter<P>;
#if HECL_RUNTIME
  using StageTargetTp = StageRuntimeObject<P, S>;
#else
  using StageTargetTp = StageBinary<P, S>;
#endif
  std::unordered_map<uint64_t, StageTargetTp> m_stageCache;

#if 0 /* Horrible compiler memory explosion - DO NOT USE! */
    template <typename ToTp, typename FromTp>
    static constexpr bool is_stage_constructible_v =
        std::is_constructible<ToTp, StageConverter<P, S>&, FactoryCtx&, FromTp>::value;

    template<typename FinalTp, typename OriginTp, typename... AllTypes>
    struct is_eventually_constructible : std::false_type {};
    template<typename FinalTp, typename OriginTp, typename ToTp, typename T, typename... Targs>
    struct _is_eventually_constructible
    : std::conditional_t<is_stage_constructible_v<FinalTp, OriginTp>,
          std::true_type,
          std::conditional_t<is_stage_constructible_v<ToTp, OriginTp>,
              is_eventually_constructible<FinalTp, ToTp, AllStageTypes>,
              _is_eventually_constructible<FinalTp, OriginTp, T, Targs...>>> {};
    template<typename FinalTp, typename OriginTp, typename ToTp>
    struct _is_eventually_constructible<FinalTp, OriginTp, ToTp, null_t> : std::false_type {};
    template<typename FinalTp, typename OriginTp, typename... AllTypes>
    struct is_eventually_constructible<FinalTp, OriginTp, pack<AllTypes...>>
    : _is_eventually_constructible<FinalTp, OriginTp, AllTypes...> {};
    template <typename FinalTp, typename OriginTp>
    static constexpr bool is_eventually_constructible_v =
        is_eventually_constructible<FinalTp, OriginTp, AllStageTypes>::value;

    template<typename FinalTp, typename OriginTp, typename ToTp, typename T, typename... Targs>
    struct _next_type { using type = std::conditional_t<is_stage_constructible_v<FinalTp, ToTp> &&
        is_eventually_constructible_v<ToTp, OriginTp>,
            ToTp,
            typename _next_type<FinalTp, OriginTp, T, Targs...>::type>; };
    template<typename FinalTp, typename OriginTp, typename ToTp>
    struct _next_type<FinalTp, OriginTp, ToTp, null_t> { using type = null_t; };
    template<typename FinalTp, typename OriginTp, typename... AllTypes>
    struct next_type { using type = null_t; };
    template<typename FinalTp, typename OriginTp, typename... AllTypes>
    struct next_type<FinalTp, OriginTp, pack<AllTypes...>> : _next_type<FinalTp, OriginTp, AllTypes...> {};
    template <typename FinalTp, typename OriginTp>
    using next_type_t = typename next_type<FinalTp, OriginTp, AllStageTypes>::type;

    template<class ToTp, class FromTp>
    std::enable_if_t<!is_stage_constructible_v<ToTp, FromTp>, ToTp>
    _Do(FactoryCtx& ctx, const FromTp& in)
    {
        using NextTp = next_type_t<ToTp, FromTp>;
        return ToTp(*this, ctx, _Do<NextTp, FromTp>(ctx, in));
    }

    template<class ToTp, class FromTp>
    std::enable_if_t<is_stage_constructible_v<ToTp, FromTp>, ToTp>
    _Do(FactoryCtx& ctx, const FromTp& in)
    {
        return ToTp(*this, ctx, in);
    }
#endif

  using StageTypes = typename ShaderDB<P>::template StageDB<S>::StageTypes;

  template <typename ToTp, typename FromTp>
  static constexpr bool is_stage_constructible_v =
      std::is_constructible<ToTp, StageConverter<P, S>&, FactoryCtx&, FromTp>::value;

  template <typename OriginTp, typename ToTp, typename T, typename... Targs>
  struct _next_type {
    using type = std::conditional_t<is_stage_constructible_v<ToTp, OriginTp>, ToTp,
                                    typename _next_type<OriginTp, T, Targs...>::type>;
  };
  template <typename OriginTp, typename ToTp>
  struct _next_type<OriginTp, ToTp, null_t> {
    using type = null_t;
  };
  template <typename OriginTp, typename... AllTypes>
  struct next_type {
    using type = null_t;
  };
  template <typename OriginTp, typename... AllTypes>
  struct next_type<OriginTp, pack<AllTypes...>> : _next_type<OriginTp, AllTypes...> {};
  template <typename OriginTp>
  using next_type_t = typename next_type<OriginTp, StageTypes>::type;

  /* StageSourceText derivative -> StageBinary */
  template <class ToTp, class FromTp, class NextTp>
  std::enable_if_t<std::is_same_v<ToTp, StageBinary<P, S>> && std::is_base_of_v<StageSourceText<P, S>, NextTp>,
                   StageBinary<P, S>>
  _DoDerivative(FactoryCtx& ctx, const FromTp& in) {
    return StageBinary<P, S>(*this, ctx, NextTp(*this, ctx, in));
  }

  /* StageBinary derivative -> StageBinary */
  template <class ToTp, class FromTp, class NextTp>
  std::enable_if_t<std::is_same_v<ToTp, StageBinary<P, S>> && std::is_base_of_v<StageBinary<P, S>, NextTp>,
                   StageBinary<P, S>>
  _DoDerivative(FactoryCtx& ctx, const FromTp& in) {
    return NextTp(*this, ctx, in);
  }

  /* Non-StageSourceText derivative -> StageBinary */
  template <class ToTp, class FromTp>
  std::enable_if_t<std::is_same_v<ToTp, StageBinary<P, S>> && !std::is_base_of_v<StageSourceText<P, S>, FromTp>,
                   StageBinary<P, S>>
  _Do(FactoryCtx& ctx, const FromTp& in) {
    using NextTp = next_type_t<FromTp>;
    static_assert(!std::is_same_v<NextTp, null_t>, "Unable to resolve StageBinary or StageSourceText derivative");
    return _DoDerivative<ToTp, FromTp, NextTp>(ctx, in);
  }

  /* StageSourceText derivative -> StageBinary */
  template <class ToTp, class FromTp>
  std::enable_if_t<std::is_same_v<ToTp, StageBinary<P, S>> && std::is_base_of_v<StageSourceText<P, S>, FromTp>,
                   StageBinary<P, S>>
  _Do(FactoryCtx& ctx, const FromTp& in) {
    return StageBinary<P, S>(*this, ctx, in);
  }

  /* Non-StageBinary derivative -> StageRuntimeObject */
  template <class ToTp, class FromTp>
  std::enable_if_t<std::is_same_v<ToTp, StageRuntimeObject<P, S>> && !std::is_base_of_v<StageBinary<P, S>, FromTp>,
                   StageRuntimeObject<P, S>>
  _Do(FactoryCtx& ctx, const FromTp& in) {
    return StageRuntimeObject<P, S>(*this, ctx, _Do<StageBinary<P, S>, FromTp>(ctx, in));
  }

  /* StageBinary derivative -> StageRuntimeObject */
  template <class ToTp, class FromTp>
  std::enable_if_t<std::is_same_v<ToTp, StageRuntimeObject<P, S>> && std::is_base_of_v<StageBinary<P, S>, FromTp>,
                   StageRuntimeObject<P, S>>
  _Do(FactoryCtx& ctx, const FromTp& in) {
    return StageRuntimeObject<P, S>(*this, ctx, in);
  }

  template <class ToTp, class FromTp>
  ToTp Do(FactoryCtx& ctx, const FromTp& in) {
    return _Do<ToTp, FromTp>(ctx, in);
  }

public:
#if HECL_RUNTIME
  void loadFromStream(FactoryCtx& ctx, ShaderCacheZipStream& r);
#endif

  template <class FromTp>
  StageTargetTp convert(FactoryCtx& ctx, const FromTp& in) {
    if (FromTp::HasHash) {
      uint64_t hash = in.Hash();
      auto search = m_stageCache.find(hash);
      if (search != m_stageCache.end())
        return search->second;
      return m_stageCache.insert(std::make_pair(hash, Do<StageTargetTp>(ctx, in))).first->second;
    }
    return Do<StageTargetTp>(ctx, in);
  }
};

class PipelineConverterBase {
  boo::IGraphicsDataFactory* m_gfxF;
  boo::IGraphicsDataFactory::Platform m_platform;

protected:
  PipelineConverterBase(boo::IGraphicsDataFactory* gfxF, boo::IGraphicsDataFactory::Platform platform)
  : m_gfxF(gfxF), m_platform(platform) {}

public:
  virtual ~PipelineConverterBase() = default;
#if HECL_RUNTIME
  template <class FromTp>
  boo::ObjToken<boo::IShaderPipeline> convert(FactoryCtx& ctx, const FromTp& in);
  template <class FromTp>
  boo::ObjToken<boo::IShaderPipeline> convert(const FromTp& in);
#endif
};

template <typename P>
class PipelineConverter : public PipelineConverterBase {
#if HECL_RUNTIME
  using PipelineTargetTp = FinalPipeline<P>;
#else
  using PipelineTargetTp = StageCollection<StageBinary<P>>;
#endif
  std::unordered_map<uint64_t, PipelineTargetTp> m_pipelineCache;
  StageConverter<P, PipelineStage::Vertex> m_vertexConverter;
  StageConverter<P, PipelineStage::Fragment> m_fragmentConverter;
  StageConverter<P, PipelineStage::Geometry> m_geometryConverter;
  StageConverter<P, PipelineStage::Control> m_controlConverter;
  StageConverter<P, PipelineStage::Evaluation> m_evaluationConverter;

  using PipelineTypes = typename ShaderDB<P>::PipelineTypes;

  template <typename ToTp, typename FromTp>
  static constexpr bool is_pipeline_constructible_v =
      std::is_constructible<ToTp, PipelineConverter<P>&, FactoryCtx&, FromTp>::value;

  template <typename FinalTp, typename OriginTp, typename... AllTypes>
  struct is_eventually_constructible : std::false_type {};
  template <typename FinalTp, typename OriginTp, typename ToTp, typename T, typename... Targs>
  struct _is_eventually_constructible
  : std::conditional_t<is_pipeline_constructible_v<FinalTp, OriginTp>, std::true_type,
                       std::conditional_t<is_pipeline_constructible_v<ToTp, OriginTp>,
                                          is_eventually_constructible<FinalTp, ToTp, PipelineTypes>,
                                          _is_eventually_constructible<FinalTp, OriginTp, T, Targs...>>> {};
  template <typename FinalTp, typename OriginTp, typename ToTp>
  struct _is_eventually_constructible<FinalTp, OriginTp, ToTp, null_t> : std::false_type {};
  template <typename FinalTp, typename OriginTp, typename... AllTypes>
  struct is_eventually_constructible<FinalTp, OriginTp, pack<AllTypes...>>
  : _is_eventually_constructible<FinalTp, OriginTp, AllTypes...> {};
  template <typename FinalTp, typename OriginTp>
  static constexpr bool is_eventually_constructible_v =
      is_eventually_constructible<FinalTp, OriginTp, PipelineTypes>::value;

  template <typename FinalTp, typename OriginTp, typename ToTp, typename T, typename... Targs>
  struct _next_type {
    using type =
        std::conditional_t<is_pipeline_constructible_v<FinalTp, ToTp> && is_eventually_constructible_v<ToTp, OriginTp>,
                           ToTp, typename _next_type<FinalTp, OriginTp, T, Targs...>::type>;
  };
  template <typename FinalTp, typename OriginTp, typename ToTp>
  struct _next_type<FinalTp, OriginTp, ToTp, null_t> {
    using type = null_t;
  };
  template <typename FinalTp, typename OriginTp, typename... AllTypes>
  struct next_type {
    using type = null_t;
  };
  template <typename FinalTp, typename OriginTp, typename... AllTypes>
  struct next_type<FinalTp, OriginTp, pack<AllTypes...>> : _next_type<FinalTp, OriginTp, AllTypes...> {};
  template <typename FinalTp, typename OriginTp>
  using next_type_t = typename next_type<FinalTp, OriginTp, PipelineTypes>::type;

  template <class ToTp, class FromTp>
  std::enable_if_t<!is_pipeline_constructible_v<ToTp, FromTp>, ToTp> _Do(FactoryCtx& ctx, const FromTp& in) {
    using NextTp = next_type_t<ToTp, FromTp>;
    return ToTp(*this, ctx, _Do<NextTp, FromTp>(ctx, in));
  }

  template <class ToTp, class FromTp>
  std::enable_if_t<is_pipeline_constructible_v<ToTp, FromTp>, ToTp> _Do(FactoryCtx& ctx, const FromTp& in) {
    return ToTp(*this, ctx, in);
  }

  template <class ToTp, class FromTp>
  ToTp Do(FactoryCtx& ctx, const FromTp& in) {
    /* No idea why this fails; it works fine with manual template arguments (clang bug?) */
    // static_assert(is_eventually_constructible_v<ToTp, FromTp>, "Unable to resolve pipeline conversion chain");
    return _Do<ToTp, FromTp>(ctx, in);
  }

public:
  PipelineConverter(boo::IGraphicsDataFactory* gfxF) : PipelineConverterBase(gfxF, P::Enum) {}
#if HECL_RUNTIME
  bool loadFromFile(FactoryCtx& ctx, const hecl::SystemChar* path);
#endif

  template <class FromTp>
  PipelineTargetTp convert(FactoryCtx& ctx, const FromTp& in) {
    if (FromTp::HasHash) {
      uint64_t hash = in.Hash();
      auto search = m_pipelineCache.find(hash);
      if (search != m_pipelineCache.end())
        return search->second;
      return m_pipelineCache.insert(std::make_pair(hash, Do<PipelineTargetTp>(ctx, in))).first->second;
    }
    return Do<PipelineTargetTp>(ctx, in);
  }

  StageConverter<P, PipelineStage::Vertex>& getVertexConverter() { return m_vertexConverter; }
  StageConverter<P, PipelineStage::Fragment>& getFragmentConverter() { return m_fragmentConverter; }
  StageConverter<P, PipelineStage::Geometry>& getGeometryConverter() { return m_geometryConverter; }
  StageConverter<P, PipelineStage::Control>& getControlConverter() { return m_controlConverter; }
  StageConverter<P, PipelineStage::Evaluation>& getEvaluationConverter() { return m_evaluationConverter; }
};

#if HECL_RUNTIME

template <class FromTp>
inline boo::ObjToken<boo::IShaderPipeline> PipelineConverterBase::convert(FactoryCtx& ctx, const FromTp& in) {
  assert(ctx.platform() == m_platform && "PipelineConverterBase platform mismatch");
  switch (m_platform) {
#if BOO_HAS_GL
  case boo::IGraphicsDataFactory::Platform::OpenGL:
    return static_cast<PipelineConverter<PlatformType::OpenGL>&>(*this).convert(ctx, in).pipeline();
#endif
#if BOO_HAS_VULKAN
  case boo::IGraphicsDataFactory::Platform::Vulkan:
    return static_cast<PipelineConverter<PlatformType::Vulkan>&>(*this).convert(ctx, in).pipeline();
#endif
#if _WIN32
  case boo::IGraphicsDataFactory::Platform::D3D11:
    return static_cast<PipelineConverter<PlatformType::D3D11>&>(*this).convert(ctx, in).pipeline();
#endif
#if BOO_HAS_METAL
  case boo::IGraphicsDataFactory::Platform::Metal:
    return static_cast<PipelineConverter<PlatformType::Metal>&>(*this).convert(ctx, in).pipeline();
#endif
#if BOO_HAS_NX
  case boo::IGraphicsDataFactory::Platform::NX:
    return static_cast<PipelineConverter<PlatformType::NX>&>(*this).convert(ctx, in).pipeline();
#endif
  default:
    return {};
  }
}

template <class FromTp>
inline boo::ObjToken<boo::IShaderPipeline> PipelineConverterBase::convert(const FromTp& in) {
  boo::ObjToken<boo::IShaderPipeline> ret;
  m_gfxF->commitTransaction([this, &ret, &in](boo::IGraphicsDataFactory::Context& ctx) {
    ret = convert(ctx, in);
    return true;
  } BooTrace);
  return ret;
}

inline std::unique_ptr<PipelineConverterBase> NewPipelineConverter(boo::IGraphicsDataFactory* gfxF) {
  switch (gfxF->platform()) {
#if BOO_HAS_GL
  case boo::IGraphicsDataFactory::Platform::OpenGL:
    return std::make_unique<PipelineConverter<PlatformType::OpenGL>>(gfxF);
#endif
#if BOO_HAS_VULKAN
  case boo::IGraphicsDataFactory::Platform::Vulkan:
    return std::make_unique<PipelineConverter<PlatformType::Vulkan>>(gfxF);
#endif
#if _WIN32
  case boo::IGraphicsDataFactory::Platform::D3D11:
    return std::make_unique<PipelineConverter<PlatformType::D3D11>>(gfxF);
#endif
#if BOO_HAS_METAL
  case boo::IGraphicsDataFactory::Platform::Metal:
    setenv("HECL_NO_METAL_COMPILER", "1", 1);
    return std::make_unique<PipelineConverter<PlatformType::Metal>>(gfxF);
#endif
#if BOO_HAS_NX
  case boo::IGraphicsDataFactory::Platform::NX:
    return std::make_unique<PipelineConverter<PlatformType::NX>>(gfxF);
#endif
  default:
    return {};
  }
}

extern PipelineConverterBase* conv;

#endif

} // namespace hecl
