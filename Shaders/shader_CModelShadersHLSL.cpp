#include <cstdint>
#include <cstdlib>
#include <sstream>
#include "shader_CModelShaders.hpp"
#include "Runtime/Graphics/Shaders/CModelShaders.hpp"

extern "C" const uint8_t CMODELSHADERS_COMMON_HLSL[];
extern "C" size_t CMODELSHADERS_COMMON_HLSL_SZ;
static std::string_view CMODELSHADERS_COMMON_HLSL_SV((char*)CMODELSHADERS_COMMON_HLSL, CMODELSHADERS_COMMON_HLSL_SZ);

extern "C" const uint8_t CMODELSHADERS_VERT_HLSL[];
extern "C" size_t CMODELSHADERS_VERT_HLSL_SZ;
static std::string_view CMODELSHADERS_VERT_HLSL_SV((char*)CMODELSHADERS_VERT_HLSL, CMODELSHADERS_VERT_HLSL_SZ);

extern "C" const uint8_t CMODELSHADERS_FRAG_HLSL[];
extern "C" size_t CMODELSHADERS_FRAG_HLSL_SZ;
static std::string_view CMODELSHADERS_FRAG_HLSL_SV((char*)CMODELSHADERS_FRAG_HLSL, CMODELSHADERS_FRAG_HLSL_SZ);

using BlendMaterial = SModelShadersInfo::Material::BlendMaterial;
using TexCoordSource = BlendMaterial::TexCoordSource;

static std::string_view EmitTexGenSource2(TexCoordSource src) {
  switch (src) {
  case TexCoordSource::Position:
    return "objPos.xy"sv;
  case TexCoordSource::Normal:
    return "objNorm.xy"sv;
  case TexCoordSource::Tex0:
    return "v.uvIn[0]"sv;
  case TexCoordSource::Tex1:
    return "v.uvIn[1]"sv;
  case TexCoordSource::Tex2:
    return "v.uvIn[2]"sv;
  case TexCoordSource::Tex3:
    return "v.uvIn[3]"sv;
  case TexCoordSource::Tex4:
    return "v.uvIn[4]"sv;
  case TexCoordSource::Tex5:
    return "v.uvIn[5]"sv;
  case TexCoordSource::Tex6:
    return "v.uvIn[6]"sv;
  case TexCoordSource::Tex7:
    return "v.uvIn[7]"sv;
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
    return "float4(v.uvIn[0], 0.0, 1.0)"sv;
  case TexCoordSource::Tex1:
    return "float4(v.uvIn[1], 0.0, 1.0)"sv;
  case TexCoordSource::Tex2:
    return "float4(v.uvIn[2], 0.0, 1.0)"sv;
  case TexCoordSource::Tex3:
    return "float4(v.uvIn[3], 0.0, 1.0)"sv;
  case TexCoordSource::Tex4:
    return "float4(v.uvIn[4], 0.0, 1.0)"sv;
  case TexCoordSource::Tex5:
    return "float4(v.uvIn[5], 0.0, 1.0)"sv;
  case TexCoordSource::Tex6:
    return "float4(v.uvIn[6], 0.0, 1.0)"sv;
  case TexCoordSource::Tex7:
    return "float4(v.uvIn[7], 0.0, 1.0)"sv;
  default:
    assert(false && "Unknown source type");
    break;
  }
  return {};
}

static std::string _BuildVS(const SModelShadersInfo& info) {
  std::stringstream vertOut;
  vertOut << CMODELSHADERS_COMMON_HLSL_SV;
  vertOut << "#define URDE_COL_SLOTS "sv << unsigned(info.m_tag.getColorCount()) << '\n';
  vertOut << "#define URDE_UV_SLOTS "sv << unsigned(info.m_tag.getUvCount()) << '\n';
  vertOut << "#define URDE_SKIN_SLOTS "sv << unsigned(info.m_tag.getSkinSlotCount()) << '\n';
  vertOut << "#define URDE_WEIGHT_SLOTS "sv << unsigned(info.m_tag.getWeightCount()) << '\n';
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
          vertOut << "tmpProj = mul(texMtxs["sv << mtxIdx << "].postMtx, float4("sv <<
                  (passChunk->shouldNormalizeUv() ? "normalize"sv : ""sv) << "(mul(texMtxs["sv << mtxIdx << "].mtx, "sv <<
                  EmitTexGenSource4(passChunk->source) << ").xyz), 1.0));"sv <<
                  "vtf."sv << tpStr << "Uv = (tmpProj / tmpProj.w).xy;"sv;
        }
      }
    } else if (auto clrChunk = chunk.get_if<SModelShadersInfo::Material::CLR>()) {
      std::string_view tpStr = BlendMaterial::PassTypeToString(clrChunk->type);
      vertOut << "vtf."sv << tpStr << "Uv = float2(0.0,0.0);"sv;
    }
  }
  if (!info.m_extension.noReflection && info.m_tag.getReflectionType() != hecl::Backend::ReflectionType::None)
    vertOut << "vtf.dynReflectionUvs[0] = normalize(mul(indMtx, float4(objPos.xyz, 1.0)).xz) * float2(0.5, 0.5) + float2(0.5, 0.5);"
               "vtf.dynReflectionUvs[1] = mul(reflectMtx, float4(objPos.xyz, 1.0)).xy;"
               "vtf.dynReflectionAlpha = reflectAlpha;";

  for (size_t i = 0; i < info.m_extension.texCount; ++i) {
    const auto& extTex = info.m_extension.texs[i];
    if (extTex.mtxIdx == 0xff)
      vertOut << "vtf.extUvs["sv << i << "] = "sv << EmitTexGenSource2(extTex.src) << ";"sv;
    else
      vertOut << "tmpProj = mul(texMtxs["sv << unsigned(extTex.mtxIdx) << "].postMtx, float4("sv <<
              (extTex.normalize ? "normalize"sv : ""sv) << "(mul(texMtxs["sv << unsigned(extTex.mtxIdx) << "].mtx, "sv <<
              EmitTexGenSource4(extTex.src) << ").xyz), 1.0));"sv <<
              "vtf.extUvs["sv << i << "] = (tmpProj / tmpProj.w).xy;"sv;
  }
  vertOut << '\n';

  vertOut << CMODELSHADERS_VERT_HLSL_SV;
  return vertOut.str();
}

static std::string _BuildFS(const SModelShadersInfo& info) {
  std::stringstream fragOut;
  fragOut << CMODELSHADERS_COMMON_HLSL_SV;
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

  fragOut << CMODELSHADERS_FRAG_HLSL_SV;
  return fragOut.str();
}

template <>
std::string StageObject_CModelShaders<hecl::PlatformType::D3D11, hecl::PipelineStage::Vertex>::BuildShader(
  const SModelShadersInfo& in) {
  return _BuildVS(in);
}

template <>
std::string StageObject_CModelShaders<hecl::PlatformType::D3D11, hecl::PipelineStage::Fragment>::BuildShader(
  const SModelShadersInfo& in) {
  return _BuildFS(in);
}
