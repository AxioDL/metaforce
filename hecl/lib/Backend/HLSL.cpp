#include "hecl/Backend/HLSL.hpp"
#include "hecl/Runtime.hpp"
#include <athena/MemoryReader.hpp>
#include <athena/MemoryWriter.hpp>
#include <boo/graphicsdev/D3D.hpp>

static logvisor::Module Log("hecl::Backend::HLSL");

namespace hecl::Backend {

std::string HLSL::EmitTexGenSource2(TexGenSrc src, int uvIdx) const {
  switch (src) {
  case TexGenSrc::Position:
    return "objPos.xy\n";
  case TexGenSrc::Normal:
    return "objNorm.xy\n";
  case TexGenSrc::UV:
    return hecl::Format("v.uvIn[%u]", uvIdx);
  default:
    break;
  }
  return std::string();
}

std::string HLSL::EmitTexGenSource4(TexGenSrc src, int uvIdx) const {
  switch (src) {
  case TexGenSrc::Position:
    return "float4(objPos.xyz, 1.0)\n";
  case TexGenSrc::Normal:
    return "float4(objNorm.xyz, 1.0)\n";
  case TexGenSrc::UV:
    return hecl::Format("float4(v.uvIn[%u], 0.0, 1.0)", uvIdx);
  default:
    break;
  }
  return std::string();
}

std::string HLSL::GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const {
  std::string retval =
      "struct VertData\n"
      "{\n"
      "    float3 posIn : POSITION;\n"
      "    float3 normIn : NORMAL;\n";

  if (col)
    retval += hecl::Format("    float4 colIn[%u] : COLOR;\n", col);

  if (uv)
    retval += hecl::Format("    float2 uvIn[%u] : UV;\n", uv);

  if (w)
    retval += hecl::Format("    float4 weightIn[%u] : WEIGHT;\n", w);

  return retval + "};\n";
}

std::string HLSL::GenerateVertToFragStruct(size_t extTexCount, bool reflectionCoords) const {
  std::string retval =
      "struct VertToFrag\n"
      "{\n"
      "    float4 mvpPos : SV_Position;\n"
      "    float4 mvPos : POSITION;\n"
      "    float4 mvNorm : NORMAL;\n";

  if (m_tcgs.size())
    retval += hecl::Format("    float2 tcgs[%u] : UV;\n", unsigned(m_tcgs.size()));
  if (extTexCount)
    retval += hecl::Format("    float2 extTcgs[%u] : EXTUV;\n", unsigned(extTexCount));

  if (reflectionCoords)
    retval +=
        "    float2 reflectTcgs[2] : REFLECTUV;\n"
        "    float reflectAlpha : REFLECTALPHA;\n";

  return retval + "};\n";
}

std::string HLSL::GenerateVertUniformStruct(unsigned skinSlots, bool reflectionCoords) const {
  std::string retval;
  if (skinSlots == 0) {
    retval =
        "cbuffer HECLVertUniform : register(b0)\n"
        "{\n"
        "    float4x4 mv;\n"
        "    float4x4 mvInv;\n"
        "    float4x4 proj;\n"
        "};\n";
  } else {
    retval = hecl::Format(
        "cbuffer HECLVertUniform : register(b0)\n"
        "{\n"
        "    float4x4 objs[%u];\n"
        "    float4x4 objsInv[%u];\n"
        "    float4x4 mv;\n"
        "    float4x4 mvInv;\n"
        "    float4x4 proj;\n"
        "};\n",
        skinSlots, skinSlots);
  }
  retval +=
      "struct TCGMtx\n"
      "{\n"
      "    float4x4 mtx;\n"
      "    float4x4 postMtx;\n"
      "};\n"
      "cbuffer HECLTCGMatrix : register(b1)\n"
      "{\n"
      "    TCGMtx texMtxs[8];\n"
      "};\n";

  if (reflectionCoords)
    retval +=
        "cbuffer HECLReflectMtx : register(b3)\n"
        "{\n"
        "    float4x4 indMtx;\n"
        "    float4x4 reflectMtx;\n"
        "    float reflectAlpha;\n"
        "};\n"
        "\n";

  return retval;
}

std::string HLSL::GenerateAlphaTest() const {
  return "    if (colorOut.a < 0.01)\n"
         "    {\n"
         "        discard;\n"
         "    }\n";
}

std::string HLSL::GenerateReflectionExpr(ReflectionType type) const {
  switch (type) {
  case ReflectionType::None:
  default:
    return "float3(0.0, 0.0, 0.0)";
  case ReflectionType::Simple:
    return "reflectionTex.Sample(reflectSamp, vtf.reflectTcgs[1]).rgb * vtf.reflectAlpha";
  case ReflectionType::Indirect:
    return "reflectionTex.Sample(reflectSamp, (reflectionIndTex.Sample(samp, vtf.reflectTcgs[0]).rg - "
           "float2(0.5, 0.5)) * float2(0.5, 0.5) + vtf.reflectTcgs[1]).rgb * vtf.reflectAlpha";
  }
}

void HLSL::reset(const IR& ir, Diagnostics& diag) {
  /* Common programmable interpretation */
  ProgrammableCommon::reset(ir, diag, "HLSL");
}

std::string HLSL::makeVert(unsigned col, unsigned uv, unsigned w, unsigned s, size_t extTexCount,
                           const TextureInfo* extTexs, ReflectionType reflectionType) const {
  std::string retval = GenerateVertInStruct(col, uv, w) + "\n" +
                       GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) + "\n" +
                       GenerateVertUniformStruct(s, reflectionType != ReflectionType::None) + "\n" +
                       "VertToFrag main(in VertData v)\n"
                       "{\n"
                       "    VertToFrag vtf;\n";

  if (s) {
    /* skinned */
    retval +=
        "    float4 objPos = float4(0.0,0.0,0.0,0.0);\n"
        "    float4 objNorm = float4(0.0,0.0,0.0,0.0);\n";
    for (size_t i = 0; i < s; ++i)
      retval += hecl::Format(
          "    objPos += mul(objs[%" PRISize "], float4(v.posIn, 1.0)) * v.weightIn[%" PRISize "][%" PRISize
          "];\n"
          "    objNorm += mul(objsInv[%" PRISize "], float4(v.normIn, 1.0)) * v.weightIn[%" PRISize "][%" PRISize "];\n",
          i, i / 4, i % 4, i, i / 4, i % 4);
    retval +=
        "    objPos[3] = 1.0;\n"
        "    objNorm = float4(normalize(objNorm.xyz), 0.0);\n"
        "    vtf.mvPos = mul(mv, objPos);\n"
        "    vtf.mvNorm = float4(normalize(mul(mvInv, objNorm).xyz), 0.0);\n"
        "    vtf.mvpPos = mul(proj, vtf.mvPos);\n";
  } else {
    /* non-skinned */
    retval +=
        "    float4 objPos = float4(v.posIn, 1.0);\n"
        "    float4 objNorm = float4(v.normIn, 0.0);\n"
        "    vtf.mvPos = mul(mv, objPos);\n"
        "    vtf.mvNorm = mul(mvInv, objNorm);\n"
        "    vtf.mvpPos = mul(proj, vtf.mvPos);\n";
  }

  retval += "    float4 tmpProj;\n";

  int tcgIdx = 0;
  for (const TexCoordGen& tcg : m_tcgs) {
    if (tcg.m_mtx < 0)
      retval += hecl::Format("    vtf.tcgs[%u] = %s;\n", tcgIdx, EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
    else
      retval += hecl::Format(
          "    tmpProj = mul(texMtxs[%u].postMtx, float4(%s(mul(texMtxs[%u].mtx, %s).xyz), 1.0));\n"
          "    vtf.tcgs[%u] = (tmpProj / tmpProj.w).xy;\n",
          tcg.m_mtx, tcg.m_norm ? "normalize" : "", tcg.m_mtx, EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str(),
          tcgIdx);
    ++tcgIdx;
  }

  for (int i = 0; i < extTexCount; ++i) {
    const TextureInfo& extTex = extTexs[i];
    if (extTex.mtxIdx < 0)
      retval += hecl::Format("    vtf.extTcgs[%u] = %s;\n", i, EmitTexGenSource2(extTex.src, extTex.uvIdx).c_str());
    else
      retval += hecl::Format(
          "    tmpProj = mul(texMtxs[%u].postMtx, float4(%s(mul(texMtxs[%u].mtx, %s).xyz), 1.0));\n"
          "    vtf.extTcgs[%u] = (tmpProj / tmpProj.w).xy;\n",
          extTex.mtxIdx, extTex.normalize ? "normalize" : "", extTex.mtxIdx,
          EmitTexGenSource4(extTex.src, extTex.uvIdx).c_str(), i);
  }

  if (reflectionType != ReflectionType::None)
    retval +=
        "    vtf.reflectTcgs[0] = normalize(mul(indMtx, float4(v.posIn, 1.0)).xz) * float2(0.5, 0.5) + float2(0.5, "
        "0.5);\n"
        "    vtf.reflectTcgs[1] = mul(reflectMtx, float4(v.posIn, 1.0)).xy;\n"
        "    vtf.reflectAlpha = reflectAlpha;\n";

  return retval +
         "    return vtf;\n"
         "}\n";
}

std::string HLSL::makeFrag(size_t blockCount, const char** blockNames, bool alphaTest, ReflectionType reflectionType,
                           BlendFactor srcFactor, BlendFactor dstFactor, const Function& lighting) const {
  std::string lightingSrc;
  if (!lighting.m_source.empty())
    lightingSrc = lighting.m_source;
  else
    lightingSrc =
        "static const float4 colorReg0 = float4(1.0, 1.0, 1.0, 1.0);\n"
        "static const float4 colorReg1 = float4(1.0, 1.0, 1.0, 1.0);\n"
        "static const float4 colorReg2 = float4(1.0, 1.0, 1.0, 1.0);\n"
        "static const float4 mulColor = float4(1.0, 1.0, 1.0, 1.0);\n";

  std::string texMapDecl;
  if (m_texMapEnd)
    texMapDecl = hecl::Format("Texture2D texs[%u] : register(t0);\n", m_texMapEnd);
  if (reflectionType == ReflectionType::Indirect)
    texMapDecl += hecl::Format(
        "Texture2D reflectionIndTex : register(t%u);\n"
        "Texture2D reflectionTex : register(t%u);\n",
        m_texMapEnd, m_texMapEnd + 1);
  else if (reflectionType == ReflectionType::Simple)
    texMapDecl += hecl::Format("Texture2D reflectionTex : register(t%u);\n", m_texMapEnd);
  std::string retval = std::string("#define BLEND_SRC_") + BlendFactorToDefine(srcFactor, m_blendSrc) + "\n" +
                       "#define BLEND_DST_" + BlendFactorToDefine(dstFactor, m_blendDst) + "\n" +
                       "SamplerState samp : register(s0);\n"
                       "SamplerState clampSamp : register(s1);\n"
                       "SamplerState reflectSamp : register(s2);\n" +
                       GenerateVertToFragStruct(0, reflectionType != ReflectionType::None) + texMapDecl + "\n" +
                       lightingSrc + "\n" + (!alphaTest ? "\n[earlydepthstencil]\n" : "\n") +
                       "float4 main(in VertToFrag vtf) : SV_Target0\n{\n";

  if (m_lighting) {
    if (!lighting.m_entry.empty())
      retval += hecl::Format("    float4 lighting = %s(vtf.mvPos.xyz, normalize(vtf.mvNorm.xyz), vtf);\n",
                             lighting.m_entry.data());
    else
      retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
  }

  unsigned sampIdx = 0;
  for (const TexSampling& sampling : m_texSamplings)
    retval += hecl::Format("    float4 sampling%u = texs[%u].Sample(samp, vtf.tcgs[%u]);\n", sampIdx++, sampling.mapIdx,
                           sampling.tcgIdx);

  std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

  retval += "    float4 colorOut;\n";
  if (m_alphaExpr.size())
    retval += "    colorOut = float4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr + ") * mulColor;\n";
  else
    retval += "    colorOut = float4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0) * mulColor;\n";

  return retval + (alphaTest ? GenerateAlphaTest() : "") + "    return colorOut;\n}\n";
}

std::string HLSL::makeFrag(size_t blockCount, const char** blockNames, bool alphaTest, ReflectionType reflectionType,
                           BlendFactor srcFactor, BlendFactor dstFactor, const Function& lighting, const Function& post,
                           size_t extTexCount, const TextureInfo* extTexs) const {
  std::string lightingSrc;
  if (!lighting.m_source.empty())
    lightingSrc = lighting.m_source;
  else
    lightingSrc =
        "static const float4 colorReg0 = float4(1.0, 1.0, 1.0, 1.0);\n"
        "static const float4 colorReg1 = float4(1.0, 1.0, 1.0, 1.0);\n"
        "static const float4 colorReg2 = float4(1.0, 1.0, 1.0, 1.0);\n"
        "static const float4 mulColor = float4(1.0, 1.0, 1.0, 1.0);\n";

  std::string postSrc;
  if (!post.m_source.empty())
    postSrc = post.m_source;

  std::string postEntry;
  if (!post.m_entry.empty())
    postEntry = post.m_entry;

  std::string texMapDecl;
  if (m_texMapEnd)
    texMapDecl = hecl::Format("Texture2D texs[%u] : register(t0);\n", m_texMapEnd);
  if (reflectionType == ReflectionType::Indirect)
    texMapDecl += hecl::Format(
        "Texture2D reflectionIndTex : register(t%u);\n"
        "Texture2D reflectionTex : register(t%u);\n",
        m_texMapEnd, m_texMapEnd + 1);
  else if (reflectionType == ReflectionType::Simple)
    texMapDecl += hecl::Format("Texture2D reflectionTex : register(t%u);\n", m_texMapEnd);

  uint32_t extTexBits = 0;
  for (int i = 0; i < extTexCount; ++i) {
    const TextureInfo& extTex = extTexs[i];
    if (!(extTexBits & (1 << extTex.mapIdx))) {
      texMapDecl += hecl::Format("Texture2D extTex%u : register(t%u);\n", extTex.mapIdx, extTex.mapIdx);
      extTexBits |= (1 << extTex.mapIdx);
    }
  }

  std::string retval = std::string("#define BLEND_SRC_") + BlendFactorToDefine(srcFactor, m_blendSrc) + "\n" +
                       "#define BLEND_DST_" + BlendFactorToDefine(dstFactor, m_blendDst) + "\n" +
                       "SamplerState samp : register(s0);\n"
                       "SamplerState clampSamp : register(s1);\n"
                       "SamplerState reflectSamp : register(s2);\n" +
                       GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) + texMapDecl +
                       "\n" + lightingSrc + "\n" + postSrc + (!alphaTest ? "\n[earlydepthstencil]\n" : "\n") +
                       "float4 main(in VertToFrag vtf) : SV_Target0\n{\n";

  if (m_lighting) {
    if (!lighting.m_entry.empty())
      retval += hecl::Format("    float4 lighting = %s(vtf.mvPos.xyz, normalize(vtf.mvNorm.xyz), vtf);\n",
                             lighting.m_entry.data());
    else
      retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
  }

  unsigned sampIdx = 0;
  for (const TexSampling& sampling : m_texSamplings)
    retval += hecl::Format("    float4 sampling%u = texs[%u].Sample(samp, vtf.tcgs[%u]);\n", sampIdx++, sampling.mapIdx,
                           sampling.tcgIdx);

  std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

  retval += "    float4 colorOut;\n";
  if (m_alphaExpr.size())
    retval += "    colorOut = " + postEntry + "(" + (postEntry.size() ? "vtf, " : "") + "float4(" + m_colorExpr +
              " + " + reflectionExpr + ", " + m_alphaExpr + ")) * mulColor;\n";
  else
    retval += "    colorOut = " + postEntry + "(" + (postEntry.size() ? "vtf, " : "") + "float4(" + m_colorExpr +
              " + " + reflectionExpr + ", 1.0)) * mulColor;\n";

  return retval + (alphaTest ? GenerateAlphaTest() : "") + "    return colorOut;\n}\n";
}

} // namespace hecl::Backend
