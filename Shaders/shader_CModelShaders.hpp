#pragma once
#include "hecl/PipelineBase.hpp"
#include "hecl/hecl.hpp"
#include "DataSpec/DNAMP1/CMDLMaterials.hpp"

struct SModelShadersInfo {
  uint64_t m_hash;
  using Material = DataSpec::DNAMP1::HMDLMaterialSet::Material;
  const Material& m_material;
  const hecl::Backend::ShaderTag& m_tag;
  const hecl::Backend::ExtensionSlot& m_extension;
  std::vector<boo::VertexElementDescriptor> m_vtxFmtData;
  boo::VertexFormatInfo m_vtxFmt;
  boo::AdditionalPipelineInfo m_additionalInfo;

  explicit SModelShadersInfo(const Material& material,
                             const hecl::Backend::ShaderTag& tag,
                             const hecl::Backend::ExtensionSlot& extension)
  : m_material(material), m_tag(tag), m_extension(extension) {
    m_hash = m_tag.val64();
    hecl::hash_combine_impl<uint64_t>(m_hash, std::hash<uint64_t>()(m_extension.hash()));

    m_vtxFmtData = tag.vertexFormat();
    m_vtxFmt = boo::VertexFormatInfo(m_vtxFmtData.size(), m_vtxFmtData.data());

    m_additionalInfo = m_tag.additionalInfo(extension, material.blendFactors());
  }

  static constexpr bool HasHash = true;
  uint64_t Hash() const { return m_hash; }
};

class Shader_CModelShaders : public hecl::GeneralShader {
  const SModelShadersInfo& m_info;
  uint64_t m_vertHash, m_fragHash;
  static uint64_t BuildVertHash(const SModelShadersInfo& in);
  static uint64_t BuildFragHash(const SModelShadersInfo& in);
public:
  Shader_CModelShaders(const SModelShadersInfo& in)
    : m_info(in)
    , m_vertHash(BuildVertHash(in))
    , m_fragHash(BuildFragHash(in))
    , VtxFmt(in.m_vtxFmt)
    , PipelineInfo(in.m_additionalInfo) {}

  const boo::VertexFormatInfo VtxFmt;
  const boo::AdditionalPipelineInfo PipelineInfo;
  static constexpr bool HasHash = true;
  uint64_t Hash() const { return m_info.m_hash; }
  static constexpr bool HasStageHash = true;
  template <typename S> uint64_t StageHash() const;
  const SModelShadersInfo& info() const { return m_info; }
};
template <> inline uint64_t Shader_CModelShaders::StageHash<hecl::PipelineStage::Vertex>() const { return m_vertHash; }
template <> inline uint64_t Shader_CModelShaders::StageHash<hecl::PipelineStage::Fragment>() const { return m_fragHash; }

template <typename P, typename S>
class StageObject_CModelShaders : public hecl::StageBinary<P, S> {
  static std::string BuildShader(const SModelShadersInfo& in);

public:
  StageObject_CModelShaders(hecl::StageConverter<P, S>& conv, hecl::FactoryCtx& ctx,
                            const Shader_CModelShaders& in)
    : hecl::StageBinary<P, S>(conv, ctx, hecl::StageSourceText<P, S>(BuildShader(in.info()))) {}
};

#define UNIVERSAL_PIPELINES_shader_CModelShaders ::Shader_CModelShaders
#define STAGES_shader_CModelShaders(P, S) ::StageObject_CModelShaders<P, S>,
