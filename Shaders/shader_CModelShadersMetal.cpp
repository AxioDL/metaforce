#include <cstdint>
#include <cstdlib>
#include <sstream>
#include "shader_CModelShaders.hpp"
#include "Runtime/Graphics/Shaders/CModelShaders.hpp"

extern "C" const uint8_t CMODELSHADERS_COMMON_METAL[];
extern "C" size_t CMODELSHADERS_COMMON_METAL_SZ;
static std::string_view CMODELSHADERS_COMMON_METAL_SV((char*)CMODELSHADERS_COMMON_METAL, CMODELSHADERS_COMMON_METAL_SZ);

extern "C" const uint8_t CMODELSHADERS_VERT_METAL[];
extern "C" size_t CMODELSHADERS_VERT_METAL_SZ;
static std::string_view CMODELSHADERS_VERT_METAL_SV((char*)CMODELSHADERS_VERT_METAL, CMODELSHADERS_VERT_METAL_SZ);

extern "C" const uint8_t CMODELSHADERS_FRAG_METAL[];
extern "C" size_t CMODELSHADERS_FRAG_METAL_SZ;
static std::string_view CMODELSHADERS_FRAG_METAL_SV((char*)CMODELSHADERS_FRAG_METAL, CMODELSHADERS_FRAG_METAL_SZ);

using BlendMaterial = SModelShadersInfo::Material::BlendMaterial;
using TexCoordSource = BlendMaterial::TexCoordSource;

static std::string_view EmitTexGenSource2(TexCoordSource src) {
  switch (src) {
  case TexCoordSource::Position:
    return "objPos.xy"sv;
  case TexCoordSource::Normal:
    return "objNorm.xy"sv;
  case TexCoordSource::Tex0:
    return "v.uvIn0"sv;
  case TexCoordSource::Tex1:
    return "v.uvIn1"sv;
  case TexCoordSource::Tex2:
    return "v.uvIn2"sv;
  case TexCoordSource::Tex3:
    return "v.uvIn3"sv;
  case TexCoordSource::Tex4:
    return "v.uvIn4"sv;
  case TexCoordSource::Tex5:
    return "v.uvIn5"sv;
  case TexCoordSource::Tex6:
    return "v.uvIn6"sv;
  case TexCoordSource::Tex7:
    return "v.uvIn7"sv;
  default:
    assert(false && "Unknown source type");
    break;
  }
  return {};
}

static std::string_view EmitTexGenSource4(TexCoordSource src) {
  switch (src) {
  case TexCoordSource::Position:
    return "float4(objPos.xyz, 1.0)"sv;
  case TexCoordSource::Normal:
    return "float4(objNorm.xyz, 1.0)"sv;
  case TexCoordSource::Tex0:
    return "float4(v.uvIn0, 0.0, 1.0)"sv;
  case TexCoordSource::Tex1:
    return "float4(v.uvIn1, 0.0, 1.0)"sv;
  case TexCoordSource::Tex2:
    return "float4(v.uvIn2, 0.0, 1.0)"sv;
  case TexCoordSource::Tex3:
    return "float4(v.uvIn3, 0.0, 1.0)"sv;
  case TexCoordSource::Tex4:
    return "float4(v.uvIn4, 0.0, 1.0)"sv;
  case TexCoordSource::Tex5:
    return "float4(v.uvIn5, 0.0, 1.0)"sv;
  case TexCoordSource::Tex6:
    return "float4(v.uvIn6, 0.0, 1.0)"sv;
  case TexCoordSource::Tex7:
    return "float4(v.uvIn7, 0.0, 1.0)"sv;
  default:
    assert(false && "Unknown source type");
    break;
  }
  return {};
}

static std::string _BuildVS(const SModelShadersInfo& info) {
  std::stringstream vertOut;
  vertOut << CMODELSHADERS_COMMON_METAL_SV;
  vertOut << "#define URDE_COL_SLOTS "sv << unsigned(info.m_tag.getColorCount()) << '\n';
  vertOut << "#define URDE_UV_SLOTS "sv << unsigned(info.m_tag.getUvCount()) << '\n';
  vertOut << "#define URDE_SKIN_SLOTS "sv << unsigned(info.m_tag.getSkinSlotCount()) << '\n';
  vertOut << "#define URDE_WEIGHT_SLOTS "sv << unsigned(info.m_tag.getWeightCount()) << '\n';

  vertOut << "#define URDE_VERT_DATA_DECL struct VertData {"
             "float3 posIn [[ attribute(0) ]];"
             "float3 normIn [[ attribute(1) ]];"sv;
  for (unsigned i = 0; i < unsigned(info.m_tag.getColorCount()); ++i)
    vertOut << "float4 colIn"sv << i << " [[ attribute("sv << 2 + i << ") ]];"sv;
  for (unsigned i = 0; i < unsigned(info.m_tag.getUvCount()); ++i)
    vertOut << "float2 uvIn"sv << i << " [[ attribute("sv << 2 + info.m_tag.getColorCount() + i << ") ]];"sv;
  for (unsigned i = 0; i < unsigned(info.m_tag.getWeightCount()); ++i)
    vertOut << "float4 weightIn"sv << i << " [[ attribute("sv <<
    2 + info.m_tag.getColorCount() + info.m_tag.getUvCount() + i << ") ]];"sv;
  vertOut << "};\n"sv;

  if (info.m_tag.getSkinSlotCount()) {
    vertOut << "#define URDE_WEIGHTING_EXPR "sv;
    for (unsigned i = 0; i < info.m_tag.getSkinSlotCount(); ++i) {
      vertOut << "objPos += (vu.objs["sv << i << "] * float4(v.posIn, 1.0)) * v.weightIn"sv << i / 4 << "["sv << i % 4 <<"];"sv;
      vertOut << "objNorm += (vu.objsInv["sv << i << "] * float4(v.normIn, 1.0)) * v.weightIn"sv << i / 4 << "["sv << i % 4 <<"];"sv;
    }
    vertOut << '\n';
  }

  vertOut << "#define URDE_TCG_EXPR "sv;
  using UVAnimType = BlendMaterial::UVAnimType;
  using PassType = BlendMaterial::PassType;
  int mtxIdx = 0;
  for (const auto& chunk : info.m_material.chunks) {
    if (auto passChunk = chunk.get_if<SModelShadersInfo::Material::PASS>()) {
      if (passChunk->type != PassType::IndirectTex) {
        std::string_view tpStr = BlendMaterial::PassTypeToString(passChunk->type);
        if (passChunk->uvAnimType == UVAnimType::Invalid) {
          vertOut << "vtf."sv << tpStr << "Uv = "sv << EmitTexGenSource2(passChunk->source) << ";"sv;
        } else {
          vertOut << "tmpProj = tu.texMtxs["sv << mtxIdx << "].postMtx * float4("sv <<
                  (passChunk->shouldNormalizeUv() ? "normalize"sv : ""sv) << "((tu.texMtxs["sv << mtxIdx << "].mtx * "sv <<
                  EmitTexGenSource4(passChunk->source) << ").xyz), 1.0);"sv <<
                  "vtf."sv << tpStr << "Uv = (tmpProj / tmpProj.w).xy;"sv;
        }
      }
    } else if (auto clrChunk = chunk.get_if<SModelShadersInfo::Material::CLR>()) {
      std::string_view tpStr = BlendMaterial::PassTypeToString(clrChunk->type);
      vertOut << "vtf."sv << tpStr << "Uv = float2(0.0,0.0);"sv;
    }
  }
  if (!info.m_extension.noReflection && info.m_tag.getReflectionType() != hecl::Backend::ReflectionType::None)
    vertOut << "vtf.dynReflectionUvs0 = normalize((ru.indMtx * float4(objPos.xyz, 1.0)).xz) * float2(0.5, 0.5) + float2(0.5, 0.5);"
               "vtf.dynReflectionUvs1 = (ru.reflectMtx * float4(objPos.xyz, 1.0)).xy;"
               "vtf.dynReflectionAlpha = ru.reflectAlpha;";

  for (size_t i = 0; i < info.m_extension.texCount; ++i) {
    const auto& extTex = info.m_extension.texs[i];
    if (extTex.mtxIdx == 0xff)
      vertOut << "vtf.extUvs"sv << i << " = "sv << EmitTexGenSource2(extTex.src) << ";"sv;
    else
      vertOut << "tmpProj = tu.texMtxs["sv << unsigned(extTex.mtxIdx) << "].postMtx * float4("sv <<
              (extTex.normalize ? "normalize"sv : ""sv) << "((tu.texMtxs["sv << unsigned(extTex.mtxIdx) << "].mtx * "sv <<
              EmitTexGenSource4(extTex.src) << ").xyz), 1.0);"sv <<
              "vtf.extUvs"sv << i << " = (tmpProj / tmpProj.w).xy;"sv;
  }
  vertOut << '\n';

  vertOut << CMODELSHADERS_VERT_METAL_SV;
  return vertOut.str();
}

static std::string _BuildFS(const SModelShadersInfo& info) {
  std::stringstream fragOut;
  fragOut << CMODELSHADERS_COMMON_METAL_SV;
  fragOut << "#define URDE_MAX_LIGHTS " _XSTR(URDE_MAX_LIGHTS) "\n";
  fragOut << "#define " << info.m_extension.shaderMacro << "\n";

  using ShaderType = BlendMaterial::ShaderType;
  switch (info.m_material.shaderType) {
  case ShaderType::RetroShader:
    fragOut << "#define RETRO_SHADER\n"; break;
  case ShaderType::RetroDynamicShader:
    fragOut << "#define RETRO_DYNAMIC_SHADER\n"; break;
  case ShaderType::RetroDynamicAlphaShader:
    fragOut << "#define RETRO_DYNAMIC_ALPHA_SHADER\n"; break;
  case ShaderType::RetroDynamicCharacterShader:
    fragOut << "#define RETRO_DYNAMIC_CHARACTER_SHADER\n"; break;
  default:
    assert(false && "Unknown shader type");
    break;
  }

  fragOut << "#define BLEND_SRC_"sv << hecl::Backend::BlendFactorToDefine(
      hecl::Backend::BlendFactor(info.m_additionalInfo.srcFac), hecl::Backend::BlendFactor::One) << '\n';
  fragOut << "#define BLEND_DST_"sv << hecl::Backend::BlendFactorToDefine(
      hecl::Backend::BlendFactor(info.m_additionalInfo.dstFac), hecl::Backend::BlendFactor::Zero) << '\n';

  using PassType = BlendMaterial::PassType;
  for (const auto& chunk : info.m_material.chunks) {
    if (auto passChunk = chunk.get_if<SModelShadersInfo::Material::PASS>()) {
      if (passChunk->alpha) {
        std::string_view tpStr = BlendMaterial::PassTypeToString(passChunk->type);
        fragOut << "#define ALPHA_" << tpStr << '\n';
      }
    } else if (auto clrChunk = chunk.get_if<SModelShadersInfo::Material::CLR>()) {
      if (clrChunk->type == PassType::Alpha)
        fragOut << "#define ALPHA_alpha\n";
    }
  }

  if (info.m_tag.getAlphaTest() || info.m_extension.forceAlphaTest)
    fragOut << "#define URDE_ALPHA_TEST\n";

  if (info.m_extension.diffuseOnly)
    fragOut << "#define URDE_DIFFUSE_ONLY\n";

  if (!info.m_extension.noReflection) {
    if (info.m_tag.getReflectionType() == hecl::Backend::ReflectionType::Indirect)
      fragOut << "#define URDE_REFLECTION_INDIRECT\n"sv;
    else if (info.m_tag.getReflectionType() == hecl::Backend::ReflectionType::Simple)
      fragOut << "#define URDE_REFLECTION_SIMPLE\n"sv;
  }

  fragOut << CMODELSHADERS_FRAG_METAL_SV;
  return fragOut.str();
}

template <>
std::string StageObject_CModelShaders<hecl::PlatformType::Metal, hecl::PipelineStage::Vertex>::BuildShader(
  const SModelShadersInfo& in) {
  return _BuildVS(in);
}

template <>
std::string StageObject_CModelShaders<hecl::PlatformType::Metal, hecl::PipelineStage::Fragment>::BuildShader(
  const SModelShadersInfo& in) {
  return _BuildFS(in);
}
