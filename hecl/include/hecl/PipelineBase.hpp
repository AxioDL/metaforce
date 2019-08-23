#pragma once
#include "Compilers.hpp"
#include "../extern/boo/xxhash/xxhash.h"

#define HECL_RUNTIME 1

namespace hecl {

using AdditionalPipelineInfo = boo::AdditionalPipelineInfo;

enum class StageTargetType { SourceText, Binary, Runtime };

enum class PipelineTargetType {
  StageSourceTextCollection,
  StageBinaryCollection,
  StageRuntimeCollection,
  FinalPipeline
};

template <typename P, typename S>
class StageConverter;

template <typename P>
class PipelineConverter;

#if HECL_RUNTIME
using FactoryCtx = boo::IGraphicsDataFactory::Context;
#else
struct FactoryCtx {};
#endif

template <typename P, typename S>
class StageRep {
public:
  using Platform = P;
  using Stage = S;
};

template <typename P>
class PipelineRep {
public:
  using Platform = P;
};

class GeneralShader : public hecl::PipelineRep<hecl::PlatformType::Null> {};
class TessellationShader : public hecl::PipelineRep<hecl::PlatformType::Null> {};

template <typename P, typename S>
class StageSourceText : public StageRep<P, S> {
  std::string_view m_text;
  uint64_t m_hash;

public:
  static constexpr StageTargetType TargetType = StageTargetType::SourceText;
  static constexpr PipelineTargetType PipelineTarget = PipelineTargetType::StageSourceTextCollection;
  static constexpr bool HasHash = true;
  uint64_t Hash() const { return m_hash; }

  explicit StageSourceText(std::string_view text) : m_text(text), m_hash(XXH64(m_text.data(), m_text.size(), 0)) {}
  std::string_view text() const { return m_text; }
};

template <typename P, typename S>
class StageBinary : public StageRep<P, S> {
  StageBinaryData m_ownedData;
  const uint8_t* m_data = nullptr;
  size_t m_size = 0;
  uint64_t m_hash = 0;

public:
  static constexpr StageTargetType TargetType = StageTargetType::Binary;
  static constexpr PipelineTargetType PipelineTarget = PipelineTargetType::StageBinaryCollection;
  static constexpr bool HasHash = true;
  uint64_t Hash() const { return m_hash; }

  StageBinary(const uint8_t* data, size_t size) : m_data(data), m_size(size) { m_hash = XXH64(m_data, m_size, 0); }
  StageBinary(StageBinaryData data, size_t size)
  : m_ownedData(std::move(data)), m_data(m_ownedData.get()), m_size(size) {
    m_hash = XXH64(m_data, m_size, 0);
  }
  explicit StageBinary(std::pair<StageBinaryData, size_t> data) : StageBinary(data.first, data.second) {}
  StageBinary(StageConverter<P, S>& conv, FactoryCtx& ctx, const StageSourceText<P, S>& in)
  : StageBinary(CompileShader<P, S>(in.text())) {}
  const uint8_t* data() const { return m_data; }
  size_t size() const { return m_size; }
};

template <typename P>
class FinalPipeline;

template <class T>
using __IsStageSubclass =
    typename std::disjunction<std::is_base_of<StageRep<typename T::Platform, PipelineStage::Vertex>, T>,
                              std::is_base_of<StageRep<typename T::Platform, PipelineStage::Fragment>, T>,
                              std::is_base_of<StageRep<typename T::Platform, PipelineStage::Geometry>, T>,
                              std::is_base_of<StageRep<typename T::Platform, PipelineStage::Control>, T>,
                              std::is_base_of<StageRep<typename T::Platform, PipelineStage::Evaluation>, T>>;
template <class T>
inline constexpr bool __IsStageSubclass_v = __IsStageSubclass<T>::value;

template <typename T>
class StageCollection;
template <template <typename, typename> class T, typename P, typename... Rest>
class StageCollection<T<P, Rest...>> : public PipelineRep<P> {
  using base = PipelineRep<P>;
  friend class FinalPipeline<P>;
  static_assert(__IsStageSubclass_v<T<P, PipelineStage::Vertex>>,
                "Stage Collection may only be specialized with StageRep subclasses");
  T<P, PipelineStage::Vertex> m_vertex;
  T<P, PipelineStage::Fragment> m_fragment;
  T<P, PipelineStage::Geometry> m_geometry;
  T<P, PipelineStage::Control> m_control;
  T<P, PipelineStage::Evaluation> m_evaluation;
  AdditionalPipelineInfo m_additionalInfo;
  std::vector<boo::VertexElementDescriptor> m_vtxFmtData;
  boo::VertexFormatInfo m_vtxFmt;
  uint64_t m_hash;

public:
  static constexpr PipelineTargetType TargetType = T<P, PipelineStage::Vertex>::PipelineTarget;
  static constexpr bool HasHash = T<P, PipelineStage::Vertex>::HasHash;
  template <typename U = StageCollection<T<P, Rest...>>>
  std::enable_if_t<U::HasHash, uint64_t> Hash() const {
    return m_hash;
  }
  template <typename U = StageCollection<T<P, Rest...>>>
  void MakeHash(std::enable_if_t<!U::HasHash>* = 0) {}
  template <typename U = StageCollection<T<P, Rest...>>>
  void MakeHash(std::enable_if_t<U::HasHash>* = 0) {
    m_hash = 0;
    m_hash ^= m_vertex.Hash();
    m_hash ^= m_fragment.Hash();
    m_hash ^= m_geometry.Hash();
    m_hash ^= m_control.Hash();
    m_hash ^= m_evaluation.Hash();
    m_hash ^= XXH64(&m_additionalInfo, sizeof(m_additionalInfo), 0);
  }
  template <typename I>
  StageCollection(PipelineConverter<P>& conv, FactoryCtx& ctx, const I& in,
                  typename std::enable_if_t<std::is_base_of_v<GeneralShader, I>>* = 0) {
    m_vertex = conv.getVertexConverter().convert(ctx, in);
    m_fragment = conv.getFragmentConverter().convert(ctx, in);
    m_vtxFmt = in.VtxFmt;
    m_additionalInfo = in.PipelineInfo;
    MakeHash();
  }
  template <typename I>
  StageCollection(
      PipelineConverter<P>& conv, FactoryCtx& ctx, const I& in,
      typename std::enable_if_t<std::conjunction_v<std::is_base_of<TessellationShader, I>,
                                                   std::negation<std::is_same<P, PlatformType::Metal>>>>* = 0) {
    m_vertex = conv.getVertexConverter().convert(ctx, in);
    m_fragment = conv.getFragmentConverter().convert(ctx, in);
    if (in.HasTessellation) {
      m_control = conv.getControlConverter().convert(ctx, in);
      m_evaluation = conv.getEvaluationConverter().convert(ctx, in);
    }
    m_vtxFmt = in.VtxFmt;
    m_additionalInfo = in.PipelineInfo;
    MakeHash();
  }
  template <typename I>
  StageCollection(PipelineConverter<P>& conv, FactoryCtx& ctx, const I& in,
                  typename std::enable_if_t<std::conjunction_v<std::is_base_of<TessellationShader, I>,
                                                               std::is_same<P, PlatformType::Metal>>>* = 0) {
    if (in.HasTessellation) {
      m_control = conv.getControlConverter().convert(ctx, in);
      m_evaluation = conv.getEvaluationConverter().convert(ctx, in);
    } else {
      m_vertex = conv.getVertexConverter().convert(ctx, in);
    }
    m_fragment = conv.getFragmentConverter().convert(ctx, in);
    m_vtxFmt = in.VtxFmt;
    m_additionalInfo = in.PipelineInfo;
    MakeHash();
  }
  StageCollection(const T<P, PipelineStage::Vertex>& vertex, const T<P, PipelineStage::Fragment>& fragment,
                  const T<P, PipelineStage::Geometry>& geometry, const T<P, PipelineStage::Control>& control,
                  const T<P, PipelineStage::Evaluation>& evaluation, const AdditionalPipelineInfo& info,
                  const boo::VertexFormatInfo& vtxFmt)
  : m_vertex(vertex)
  , m_fragment(fragment)
  , m_geometry(geometry)
  , m_control(control)
  , m_evaluation(evaluation)
  , m_additionalInfo(info)
  , m_vtxFmt(vtxFmt) {}
};

} // namespace hecl

#ifndef _WIN32
#define _STAGEOBJECT_PROTOTYPE_DECLARATIONS(T, P)                                                                      \
  template <>                                                                                                          \
  const hecl::StageBinary<P, hecl::PipelineStage::Vertex> T<P, hecl::PipelineStage::Vertex>::Prototype;                \
  template <>                                                                                                          \
  const hecl::StageBinary<P, hecl::PipelineStage::Fragment> T<P, hecl::PipelineStage::Fragment>::Prototype;            \
  template <>                                                                                                          \
  const hecl::StageBinary<P, hecl::PipelineStage::Geometry> T<P, hecl::PipelineStage::Geometry>::Prototype;            \
  template <>                                                                                                          \
  const hecl::StageBinary<P, hecl::PipelineStage::Control> T<P, hecl::PipelineStage::Control>::Prototype;              \
  template <>                                                                                                          \
  const hecl::StageBinary<P, hecl::PipelineStage::Evaluation> T<P, hecl::PipelineStage::Evaluation>::Prototype;
#else
#define _STAGEOBJECT_PROTOTYPE_DECLARATIONS(T, P)
#endif

#define STAGEOBJECT_PROTOTYPE_DECLARATIONS(T)                                                                          \
  _STAGEOBJECT_PROTOTYPE_DECLARATIONS(T, hecl::PlatformType::OpenGL)                                                   \
  _STAGEOBJECT_PROTOTYPE_DECLARATIONS(T, hecl::PlatformType::Vulkan)                                                   \
  _STAGEOBJECT_PROTOTYPE_DECLARATIONS(T, hecl::PlatformType::D3D11)                                                    \
  _STAGEOBJECT_PROTOTYPE_DECLARATIONS(T, hecl::PlatformType::Metal)                                                    \
  _STAGEOBJECT_PROTOTYPE_DECLARATIONS(T, hecl::PlatformType::NX)
