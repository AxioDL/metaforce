#include "HECL/Backend/GLSL.hpp"
#include <map>

namespace HECL
{
namespace Backend
{

std::string GLSL::EmitTexGenSource2(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TG_POS:
        return "posIn.xy;\n";
    case TG_NRM:
        return "normIn.xy;\n";
    case TG_UV:
        return HECL::Format("uvIn[%u]", uvIdx);
    default: break;
    }
    return std::string();
}

std::string GLSL::EmitTexGenSource4(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TG_POS:
        return "vec4(posIn, 1.0);\n";
    case TG_NRM:
        return "vec4(normIn, 1.0);\n";
    case TG_UV:
        return HECL::Format("vec4(uvIn[%u], 0.0, 1.0)", uvIdx);
    default: break;
    }
    return std::string();
}

std::string GLSL::GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const
{
    std::string retval =
    "layout(location=0) in vec3 posIn;\n"
    "layout(location=1) in vec3 normIn;\n";

    unsigned idx = 2;
    if (col)
    {
        retval += HECL::Format("layout(location=%u) in vec4 colIn[%u];\n", idx, col);
        idx += col;
    }

    if (uv)
    {
        retval += HECL::Format("layout(location=%u) in vec2 uvIn[%u];\n", idx, uv);
        idx += uv;
    }

    if (w)
    {
        retval += HECL::Format("layout(location=%u) in vec4 weightIn[%u];\n", idx, w);
    }

    return retval;
}

std::string GLSL::GenerateVertToFragStruct() const
{
    std::string retval =
    "struct VertToFrag\n"
    "{\n"
    "    vec4 mvPos;\n"
    "    vec4 mvNorm;\n";

    if (m_tcgs.size())
        retval += HECL::Format("    vec2 tcgs[%u];\n", unsigned(m_tcgs.size()));

    return retval + "};\n";
}

std::string GLSL::GenerateVertUniformStruct(unsigned skinSlots, unsigned texMtxs) const
{
    if (skinSlots == 0)
        skinSlots = 1;
    std::string retval = HECL::Format("struct HECLVertUniform\n"
                                      "{\n"
                                      "    mat4 mv[%u];\n"
                                      "    mat4 mvInv[%u];\n"
                                      "    mat4 proj;\n",
                                      skinSlots, skinSlots);
    if (texMtxs)
        retval += HECL::Format("    mat4 texMtxs[%u];\n", texMtxs);
    return retval + "};\n";
}

void GLSL::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "GLSL");
}

std::string GLSL::makeVert(const char* glslVer, unsigned col, unsigned uv, unsigned w,
                           unsigned skinSlots, unsigned texMtxs) const
{
    std::string retval = std::string(glslVer) + "\n" +
            GenerateVertInStruct(col, uv, w) + "\n" +
            GenerateVertToFragStruct() + "\n" +
            GenerateVertUniformStruct(skinSlots, texMtxs) +
            "layout(location=0) uniform HECLVertUniform vu;\n"
            "out VertToFrag vtf;\n\n"
            "void main()\n{\n";

    if (skinSlots)
    {
        /* skinned */
        retval += "    vec4 posAccum = vec4(0.0,0.0,0.0,0.0);\n"
                  "    vec4 normAccum = vec4(0.0,0.0,0.0,0.0);\n";
        for (size_t i=0 ; i<skinSlots ; ++i)
            retval += HECL::Format("    posAccum += (vu.mv[%u] * vec4(posIn, 1.0)) * weightIn[%u][%u]\n"
                                   "    normAccum += (vu.mvInv[%u] * vec4(normIn, 1.0)) * weightIn[%u][%u]\n",
                                   i, i/4, i%4, i, i/4, i%4);
        retval += HECL::Format("    posAccum /= %u;\n"
                               "    normAccum /= %u;\n",
                               skinSlots, skinSlots);
        retval += "    posAccum[3] = 1.0\n"
                  "    vtf.mvPos = posAccum;\n"
                  "    vtf.mvNorm = vec4(normalize(normAccum.xyz), 0.0);\n"
                  "    gl_Position = vu.proj * posAccum;\n";
    }
    else
    {
        /* non-skinned */
        retval += "    vtf.mvPos = vu.mv[0] * vec4(posIn, 1.0);\n"
                  "    vtf.mvNorm = vu.mvInv[0] * vec4(normIn, 0.0)\n"
                  "    gl_Position = vu.proj * vtf.mvPos;\n";
    }

    int tcgIdx = 0;
    for (const TexCoordGen& tcg : m_tcgs)
    {
        if (tcg.m_mtx < 0)
            retval += HECL::Format("    vtf.tcgs[%u] = %s;\n", tcgIdx,
                                   EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
        else
            retval += HECL::Format("    vtf.tcgs[%u] = (vu.texMtxs[%u] * %s).xy;\n", tcgIdx, tcg.m_mtx,
                                   EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str());
        ++tcgIdx;
    }

    return retval + "}\n";
}

std::string GLSL::makeFrag(const char* glslVer,
                           const char* lightingSource, const char* lightingEntry) const
{
    std::string lightingSrc;
    if (lightingSource)
        lightingSrc = lightingSource;

    std::string retval = std::string(glslVer) + "\n" +
            GenerateVertToFragStruct() +
            "\nlayout(location=0) out vec4 colorOut;\n"
            "in VertToFrag vtf;\n\n" + lightingSrc +
            "\nvoid main()\n{\n";

    if (m_lighting)
    {
        if (lightingEntry)
            retval += HECL::Format("    vec4 lighting = %s();\n", lightingEntry);
        else
            retval += "    vec4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
    }

    if (m_alphaExpr.size())
        retval += "    colorOut = vec4(" + m_colorExpr + ", " + m_alphaExpr + ");\n";
    else
        retval += "    colorOut = vec4(" + m_colorExpr + ", 1.0);\n";

    return retval + "};\n";
}

std::string GLSL::makeFrag(const char* glslVer,
                           const char* lightingSource, const char* lightingEntry,
                           const char* postSource, const char* postEntry) const
{
    std::string lightingSrc;
    if (lightingSource)
        lightingSrc = lightingSource;

    std::string retval = std::string(glslVer) + "\n" +
            GenerateVertToFragStruct() +
            "\nlayout(location=0) out vec4 colorOut;\n"
            "in VertToFrag vtf;\n\n" + lightingSrc + "\n" + std::string(postSource) +
            "\nvoid main()\n{\n";

    if (m_lighting)
    {
        if (lightingEntry)
            retval += HECL::Format("    vec4 lighting = %s();\n", lightingEntry);
        else
            retval += "    vec4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
    }

    if (m_alphaExpr.size())
        retval += "    colorOut = " + std::string(postEntry) + "(vec4(" + m_colorExpr + ", " + m_alphaExpr + "));\n";
    else
        retval += "    colorOut = " + std::string(postEntry) + "(vec4(" + m_colorExpr + ", 1.0));\n";

    return retval + "};\n";
}

}
}
