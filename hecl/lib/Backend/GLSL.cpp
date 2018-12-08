#include "hecl/Backend/GLSL.hpp"
#include "hecl/Runtime.hpp"
#include "athena/MemoryReader.hpp"
#include "athena/MemoryWriter.hpp"
#include "boo/graphicsdev/GLSLMacros.hpp"

static logvisor::Module Log("hecl::Backend::GLSL");

namespace hecl::Backend {

std::string GLSL::EmitTexGenSource2(TexGenSrc src, int uvIdx) const {
  switch (src) {
  case TexGenSrc::Position:
    return "objPos.xy";
  case TexGenSrc::Normal:
    return "objNorm.xy";
  case TexGenSrc::UV:
    return hecl::Format("uvIn[%u]", uvIdx);
  default:
    break;
  }
  return std::string();
}

std::string GLSL::EmitTexGenSource4(TexGenSrc src, int uvIdx) const {
  switch (src) {
  case TexGenSrc::Position:
    return "vec4(objPos.xyz, 1.0)";
  case TexGenSrc::Normal:
    return "vec4(objNorm.xyz, 1.0)";
  case TexGenSrc::UV:
    return hecl::Format("vec4(uvIn[%u], 0.0, 1.0)", uvIdx);
  default:
    break;
  }
  return std::string();
}

std::string GLSL::GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const {
  std::string retval =
      "layout(location=0) in vec3 posIn;\n"
      "layout(location=1) in vec3 normIn;\n";

  unsigned idx = 2;
  if (col) {
    retval += hecl::Format("layout(location=%u) in vec4 colIn[%u];\n", idx, col);
    idx += col;
  }

  if (uv) {
    retval += hecl::Format("layout(location=%u) in vec2 uvIn[%u];\n", idx, uv);
    idx += uv;
  }

  if (w) {
    retval += hecl::Format("layout(location=%u) in vec4 weightIn[%u];\n", idx, w);
  }

  return retval;
}

std::string GLSL::GenerateVertToFragStruct(size_t extTexCount, bool reflectionCoords) const {
  std::string retval =
      "struct VertToFrag\n"
      "{\n"
      "    vec4 mvPos;\n"
      "    vec4 mvNorm;\n";

  if (m_tcgs.size())
    retval += hecl::Format("    vec2 tcgs[%u];\n", unsigned(m_tcgs.size()));
  if (extTexCount)
    retval += hecl::Format("    vec2 extTcgs[%u];\n", unsigned(extTexCount));

  if (reflectionCoords)
    retval +=
        "    vec2 reflectTcgs[2];\n"
        "    float reflectAlpha;\n";

  return retval + "};\n";
}

std::string GLSL::GenerateVertUniformStruct(unsigned skinSlots, bool reflectionCoords) const {
  std::string retval;
  if (skinSlots == 0) {
    retval =
        "UBINDING0 uniform HECLVertUniform\n"
        "{\n"
        "    mat4 mv;\n"
        "    mat4 mvInv;\n"
        "    mat4 proj;\n"
        "};\n";
  } else {
    retval = hecl::Format(
        "UBINDING0 uniform HECLVertUniform\n"
        "{\n"
        "    mat4 objs[%u];\n"
        "    mat4 objsInv[%u];\n"
        "    mat4 mv;\n"
        "    mat4 mvInv;\n"
        "    mat4 proj;\n"
        "};\n",
        skinSlots, skinSlots);
  }

  retval +=
      "struct HECLTCGMatrix\n"
      "{\n"
      "    mat4 mtx;\n"
      "    mat4 postMtx;\n"
      "};\n"
      "UBINDING1 uniform HECLTexMtxUniform\n"
      "{\n"
      "    HECLTCGMatrix texMtxs[8];\n"
      "};\n";

  if (reflectionCoords)
    retval +=
        "UBINDING3 uniform HECLReflectMtx\n"
        "{\n"
        "    mat4 indMtx;\n"
        "    mat4 reflectMtx;\n"
        "    float reflectAlpha;\n"
        "};\n"
        "\n";

  return retval;
}

std::string GLSL::GenerateAlphaTest() const {
  return "    if (colorOut.a < 0.01)\n"
         "    {\n"
         "        discard;\n"
         "    }\n";
}

std::string GLSL::GenerateReflectionExpr(ReflectionType type) const {
  switch (type) {
  case ReflectionType::None:
  default:
    return "vec3(0.0, 0.0, 0.0)";
  case ReflectionType::Simple:
    return "texture(reflectionTex, vtf.reflectTcgs[1]).rgb * vtf.reflectAlpha";
  case ReflectionType::Indirect:
    return "texture(reflectionTex, (texture(reflectionIndTex, vtf.reflectTcgs[0]).rg - "
           "vec2(0.5, 0.5)) * vec2(0.5, 0.5) + vtf.reflectTcgs[1]).rgb * vtf.reflectAlpha";
  }
}

void GLSL::reset(const IR& ir, Diagnostics& diag) {
  /* Common programmable interpretation */
  ProgrammableCommon::reset(ir, diag, "GLSL");
}

std::string GLSL::makeVert(unsigned col, unsigned uv, unsigned w, unsigned s, size_t extTexCount,
                           const TextureInfo* extTexs, ReflectionType reflectionType) const {
  extTexCount = std::min(int(extTexCount), BOO_GLSL_MAX_TEXTURE_COUNT - int(m_tcgs.size()));
  std::string retval = GenerateVertInStruct(col, uv, w) + "\n" +
                       GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) + "\n" +
                       GenerateVertUniformStruct(s, reflectionType != ReflectionType::None) +
                       "SBINDING(0) out VertToFrag vtf;\n\n"
                       "void main()\n{\n";

  if (s) {
    /* skinned */
    retval +=
        "    vec4 objPos = vec4(0.0,0.0,0.0,0.0);\n"
        "    vec4 objNorm = vec4(0.0,0.0,0.0,0.0);\n";
    for (size_t i = 0; i < s; ++i)
      retval += hecl::Format("    objPos += (objs[%" PRISize "] * vec4(posIn, 1.0)) * weightIn[%" PRISize "][%" PRISize
                             "];\n"
                             "    objNorm += (objsInv[%" PRISize "] * vec4(normIn, 1.0)) * weightIn[%" PRISize
                             "][%" PRISize "];\n",
                             i, i / 4, i % 4, i, i / 4, i % 4);
    retval +=
        "    objPos[3] = 1.0;\n"
        "    objNorm = vec4(normalize(objNorm.xyz), 0.0);\n"
        "    vtf.mvPos = mv * objPos;\n"
        "    vtf.mvNorm = vec4(normalize((mvInv * objNorm).xyz), 0.0);\n"
        "    gl_Position = proj * vtf.mvPos;\n";
  } else {
    /* non-skinned */
    retval +=
        "    vec4 objPos = vec4(posIn, 1.0);\n"
        "    vec4 objNorm = vec4(normIn, 0.0);\n"
        "    vtf.mvPos = mv * objPos;\n"
        "    vtf.mvNorm = mvInv * objNorm;\n"
        "    gl_Position = proj * vtf.mvPos;\n";
  }

  retval += "    vec4 tmpProj;\n";

  int tcgIdx = 0;
  for (const TexCoordGen& tcg : m_tcgs) {
    if (tcg.m_mtx < 0)
      retval += hecl::Format("    vtf.tcgs[%u] = %s;\n", tcgIdx, EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
    else
      retval += hecl::Format(
          "    tmpProj = texMtxs[%u].postMtx * vec4(%s((texMtxs[%u].mtx * %s).xyz), 1.0);\n"
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
          "    tmpProj = texMtxs[%u].postMtx * vec4(%s((texMtxs[%u].mtx * %s).xyz), 1.0);\n"
          "    vtf.extTcgs[%u] = (tmpProj / tmpProj.w).xy;\n",
          extTex.mtxIdx, extTex.normalize ? "normalize" : "", extTex.mtxIdx,
          EmitTexGenSource4(extTex.src, extTex.uvIdx).c_str(), i);
  }

  if (reflectionType != ReflectionType::None)
    retval +=
        "    vtf.reflectTcgs[0] = normalize((indMtx * vec4(posIn, 1.0)).xz) * vec2(0.5, 0.5) + vec2(0.5, 0.5);\n"
        "    vtf.reflectTcgs[1] = (reflectMtx * vec4(posIn, 1.0)).xy;\n"
        "    vtf.reflectAlpha = reflectAlpha;\n";

  return retval + "}\n";
}

std::string GLSL::makeFrag(size_t blockCount, const char** blockNames, bool alphaTest, ReflectionType reflectionType,
                           BlendFactor srcFactor, BlendFactor dstFactor, const Function& lighting) const {
  std::string lightingSrc;
  if (!lighting.m_source.empty())
    lightingSrc = lighting.m_source;
  else
    lightingSrc =
        "const vec4 colorReg0 = vec4(1.0);\n"
        "const vec4 colorReg1 = vec4(1.0);\n"
        "const vec4 colorReg2 = vec4(1.0);\n"
        "const vec4 mulColor = vec4(1.0);\n"
        "\n";

  std::string texMapDecl;
  for (unsigned i = 0; i < m_texMapEnd; ++i)
    texMapDecl += hecl::Format("TBINDING%u uniform sampler2D tex%u;\n", i, i);
  if (reflectionType == ReflectionType::Indirect)
    texMapDecl += hecl::Format(
        "TBINDING%u uniform sampler2D reflectionIndTex;\n"
        "TBINDING%u uniform sampler2D reflectionTex;\n",
        m_texMapEnd, m_texMapEnd + 1);
  else if (reflectionType == ReflectionType::Simple)
    texMapDecl += hecl::Format("TBINDING%u uniform sampler2D reflectionTex;\n", m_texMapEnd);

  std::string retval = std::string("#extension GL_ARB_shader_image_load_store: enable\n") + "#define BLEND_SRC_" +
                       BlendFactorToDefine(srcFactor, m_blendSrc) + "\n" + "#define BLEND_DST_" +
                       BlendFactorToDefine(dstFactor, m_blendDst) + "\n" +
                       GenerateVertToFragStruct(0, reflectionType != ReflectionType::None) +
                       (!alphaTest ? "#ifdef GL_ARB_shader_image_load_store\n"
                                     "layout(early_fragment_tests) in;\n"
                                     "#endif\n"
                                   : "") +
                       "layout(location=0) out vec4 colorOut;\n" + texMapDecl + "SBINDING(0) in VertToFrag vtf;\n\n" +
                       lightingSrc + "\n" + "void main()\n{\n";

  if (m_lighting) {
    if (!lighting.m_entry.empty())
      retval +=
          hecl::Format("    vec4 lighting = %s(vtf.mvPos.xyz, normalize(vtf.mvNorm.xyz));\n", lighting.m_entry.data());
    else
      retval += "    vec4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
  }

  unsigned sampIdx = 0;
  for (const TexSampling& sampling : m_texSamplings)
    retval += hecl::Format("    vec4 sampling%u = texture(tex%u, vtf.tcgs[%u]);\n", sampIdx++, sampling.mapIdx,
                           sampling.tcgIdx);

  std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

  if (m_alphaExpr.size())
    retval += "    colorOut = vec4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr + ") * mulColor;\n";
  else
    retval += "    colorOut = vec4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0) * mulColor;\n";

  return retval + (alphaTest ? GenerateAlphaTest() : "") + "}\n";
}

std::string GLSL::makeFrag(size_t blockCount, const char** blockNames, bool alphaTest, ReflectionType reflectionType,
                           BlendFactor srcFactor, BlendFactor dstFactor, const Function& lighting, const Function& post,
                           size_t extTexCount, const TextureInfo* extTexs) const {
  std::string lightingSrc;
  if (!lighting.m_source.empty())
    lightingSrc = lighting.m_source;
  else
    lightingSrc =
        "const vec4 colorReg0 = vec4(1.0);\n"
        "const vec4 colorReg1 = vec4(1.0);\n"
        "const vec4 colorReg2 = vec4(1.0);\n"
        "const vec4 mulColor = vec4(1.0);\n"
        "\n";

  std::string postSrc;
  if (!post.m_source.empty())
    postSrc = post.m_source;

  std::string postEntry;
  if (!post.m_entry.empty())
    postEntry = post.m_entry;

  std::string texMapDecl;
  for (unsigned i = 0; i < m_texMapEnd; ++i)
    texMapDecl += hecl::Format("TBINDING%u uniform sampler2D tex%u;\n", i, i);
  if (reflectionType == ReflectionType::Indirect)
    texMapDecl += hecl::Format(
        "TBINDING%u uniform sampler2D reflectionIndTex;\n"
        "TBINDING%u uniform sampler2D reflectionTex;\n",
        m_texMapEnd, m_texMapEnd + 1);
  else if (reflectionType == ReflectionType::Simple)
    texMapDecl += hecl::Format("TBINDING%u uniform sampler2D reflectionTex;\n", m_texMapEnd);

  uint32_t extTexBits = 0;
  for (int i = 0; i < extTexCount; ++i) {
    const TextureInfo& extTex = extTexs[i];
    if (!(extTexBits & (1 << extTex.mapIdx))) {
      texMapDecl += hecl::Format("TBINDING%u uniform sampler2D extTex%u;\n", extTex.mapIdx, extTex.mapIdx);
      extTexBits |= (1 << extTex.mapIdx);
    }
  }

  std::string retval = std::string("#extension GL_ARB_shader_image_load_store: enable\n") + "#define BLEND_SRC_" +
                       BlendFactorToDefine(srcFactor, m_blendSrc) + "\n" + "#define BLEND_DST_" +
                       BlendFactorToDefine(dstFactor, m_blendDst) + "\n" +
                       GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) +
                       (!alphaTest ? "\n#ifdef GL_ARB_shader_image_load_store\n"
                                     "layout(early_fragment_tests) in;\n"
                                     "#endif\n"
                                   : "") +
                       "\nlayout(location=0) out vec4 colorOut;\n" + texMapDecl + "SBINDING(0) in VertToFrag vtf;\n\n" +
                       lightingSrc + "\n" + postSrc + "\nvoid main()\n{\n";

  if (m_lighting) {
    if (!lighting.m_entry.empty())
      retval +=
          hecl::Format("    vec4 lighting = %s(vtf.mvPos.xyz, normalize(vtf.mvNorm.xyz));\n", lighting.m_entry.data());
    else
      retval += "    vec4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
  }

  unsigned sampIdx = 0;
  for (const TexSampling& sampling : m_texSamplings)
    retval += hecl::Format("    vec4 sampling%u = texture(tex%u, vtf.tcgs[%u]);\n", sampIdx++, sampling.mapIdx,
                           sampling.tcgIdx);

  std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

  if (m_alphaExpr.size())
    retval += "    colorOut = " + postEntry + "(vec4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr +
              ")) * mulColor;\n";
  else
    retval += "    colorOut = " + postEntry + "(vec4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0)) * mulColor;\n";

  return retval + (alphaTest ? GenerateAlphaTest() : "") + "}\n";
}

} // namespace hecl::Backend
