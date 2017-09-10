#include "hecl/Backend/GLSL.hpp"
#include "hecl/Runtime.hpp"
#include <athena/MemoryReader.hpp>
#include <athena/MemoryWriter.hpp>
#include <boo/graphicsdev/GL.hpp>
#include <boo/graphicsdev/Vulkan.hpp>

static logvisor::Module Log("hecl::Backend::GLSL");

namespace hecl
{
namespace Backend
{

std::string GLSL::EmitTexGenSource2(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TexGenSrc::Position:
        return "posIn.xy";
    case TexGenSrc::Normal:
        return "normIn.xy";
    case TexGenSrc::UV:
        return hecl::Format("uvIn[%u]", uvIdx);
    default: break;
    }
    return std::string();
}

std::string GLSL::EmitTexGenSource4(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TexGenSrc::Position:
        return "vec4(posIn, 1.0)";
    case TexGenSrc::Normal:
        return "vec4(normIn, 1.0)";
    case TexGenSrc::UV:
        return hecl::Format("vec4(uvIn[%u], 0.0, 1.0)", uvIdx);
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
        retval += hecl::Format("layout(location=%u) in vec4 colIn[%u];\n", idx, col);
        idx += col;
    }

    if (uv)
    {
        retval += hecl::Format("layout(location=%u) in vec2 uvIn[%u];\n", idx, uv);
        idx += uv;
    }

    if (w)
    {
        retval += hecl::Format("layout(location=%u) in vec4 weightIn[%u];\n", idx, w);
    }

    return retval;
}

std::string GLSL::GenerateVertToFragStruct(size_t extTexCount, bool reflectionCoords) const
{
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
        retval += "    vec2 reflectTcgs[2];\n"
                  "    float reflectAlpha;\n";

    return retval + "};\n";
}

std::string GLSL::GenerateVertUniformStruct(unsigned skinSlots, unsigned texMtxs, bool reflectionCoords) const
{
    if (skinSlots == 0)
        skinSlots = 1;
    std::string retval = hecl::Format("UBINDING0 uniform HECLVertUniform\n"
                                      "{\n"
                                      "    mat4 mv[%u];\n"
                                      "    mat4 mvInv[%u];\n"
                                      "    mat4 proj;\n"
                                      "};\n",
                                      skinSlots, skinSlots);
    if (texMtxs)
        retval += hecl::Format("struct HECLTCGMatrix\n"
                               "{\n"
                               "    mat4 mtx;\n"
                               "    mat4 postMtx;\n"
                               "};\n"
                               "UBINDING1 uniform HECLTexMtxUniform\n"
                               "{\n"
                               "    HECLTCGMatrix texMtxs[%u];\n"
                               "};\n", texMtxs);

    if (reflectionCoords)
        retval += "UBINDING3 uniform HECLReflectMtx\n"
                  "{\n"
                  "    mat4 indMtx;\n"
                  "    mat4 reflectMtx;\n"
                  "    float reflectAlpha;\n"
                  "};\n"
                  "\n";

    return retval;
}

std::string GLSL::GenerateAlphaTest() const
{
    return "    if (colorOut.a < 0.01)\n"
           "    {\n"
           "        discard;\n"
           "    }\n";
}

std::string GLSL::GenerateReflectionExpr(ReflectionType type) const
{
    switch (type)
    {
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

void GLSL::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "GLSL");
}

std::string GLSL::makeVert(const char* glslVer, unsigned col, unsigned uv, unsigned w,
                           unsigned s, unsigned tm, size_t extTexCount,
                           const TextureInfo* extTexs, ReflectionType reflectionType) const
{
    extTexCount = std::min(int(extTexCount), BOO_GLSL_MAX_TEXTURE_COUNT - int(m_tcgs.size()));
    std::string retval = std::string(glslVer) + "\n" BOO_GLSL_BINDING_HEAD +
            GenerateVertInStruct(col, uv, w) + "\n" +
            GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) + "\n" +
            GenerateVertUniformStruct(s, tm, reflectionType != ReflectionType::None) +
            "SBINDING(0) out VertToFrag vtf;\n\n"
            "void main()\n{\n";

    if (s)
    {
        /* skinned */
        retval += "    vec4 posAccum = vec4(0.0,0.0,0.0,0.0);\n"
                  "    vec4 normAccum = vec4(0.0,0.0,0.0,0.0);\n";
        for (size_t i=0 ; i<s ; ++i)
            retval += hecl::Format("    posAccum += (mv[%" PRISize "] * vec4(posIn, 1.0)) * weightIn[%" PRISize "][%" PRISize "];\n"
                                   "    normAccum += (mvInv[%" PRISize "] * vec4(normIn, 1.0)) * weightIn[%" PRISize "][%" PRISize "];\n",
                                   i, i/4, i%4, i, i/4, i%4);
        retval += "    posAccum[3] = 1.0;\n"
                  "    vtf.mvPos = posAccum;\n"
                  "    vtf.mvNorm = vec4(normalize(normAccum.xyz), 0.0);\n"
                  "    gl_Position = proj * posAccum;\n";
    }
    else
    {
        /* non-skinned */
        retval += "    vtf.mvPos = mv[0] * vec4(posIn, 1.0);\n"
                  "    vtf.mvNorm = mvInv[0] * vec4(normIn, 0.0);\n"
                  "    gl_Position = proj * vtf.mvPos;\n";
    }

    int tcgIdx = 0;
    for (const TexCoordGen& tcg : m_tcgs)
    {
        if (tcg.m_mtx < 0)
            retval += hecl::Format("    vtf.tcgs[%u] = %s;\n", tcgIdx,
                                   EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
        else
            retval += hecl::Format("    vtf.tcgs[%u] = (texMtxs[%u].postMtx * vec4(%s((texMtxs[%u].mtx * %s).xyz), 1.0)).xy;\n",
                                   tcgIdx, tcg.m_mtx, tcg.m_norm ? "normalize" : "",
                                   tcg.m_mtx, EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str());
        ++tcgIdx;
    }

    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        if (extTex.mtxIdx < 0)
            retval += hecl::Format("    vtf.extTcgs[%u] = %s;\n", i,
                                   EmitTexGenSource2(extTex.src, extTex.uvIdx).c_str());
        else
            retval += hecl::Format("    vtf.extTcgs[%u] = (texMtxs[%u].postMtx * vec4(%s((texMtxs[%u].mtx * %s).xyz), 1.0)).xy;\n",
                                   i, extTex.mtxIdx, extTex.normalize ? "normalize" : "",
                                   extTex.mtxIdx, EmitTexGenSource4(extTex.src, extTex.uvIdx).c_str());
    }

    if (reflectionType != ReflectionType::None)
        retval += "    vtf.reflectTcgs[0] = normalize((indMtx * vec4(v.posIn, 1.0)).xz) * vec2(0.5, 0.5) + vec2(0.5, 0.5);\n"
                  "    vtf.reflectTcgs[1] = (reflectMtx * vec4(v.posIn, 1.0)).xy;\n"
                  "    vtf.reflectAlpha = reflectAlpha;\n";

    return retval + "}\n";
}

std::string GLSL::makeFrag(const char* glslVer, bool alphaTest,
                           ReflectionType reflectionType, const ShaderFunction& lighting) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;
    else
        lightingSrc = "const vec4 colorReg0 = vec4(1.0);\n"
                      "const vec4 colorReg1 = vec4(1.0);\n"
                      "const vec4 colorReg2 = vec4(1.0);\n"
                      "const vec4 mulColor = vec4(1.0);\n"
                      "\n";

    std::string texMapDecl;
    for (unsigned i=0 ; i<m_texMapEnd ; ++i)
        texMapDecl += hecl::Format("TBINDING%u uniform sampler2D tex%u;\n", i, i);
    if (reflectionType == ReflectionType::Indirect)
        texMapDecl += hecl::Format("TBINDING%u uniform sampler2D reflectionIndTex;\n"
                                   "TBINDING%u uniform sampler2D reflectionTex;\n",
                                   m_texMapEnd, m_texMapEnd+1);
    else if (reflectionType == ReflectionType::Simple)
        texMapDecl += hecl::Format("TBINDING%u uniform sampler2D reflectionTex;\n",
                                   m_texMapEnd);

    std::string retval = std::string(glslVer) +
            "\n#extension GL_ARB_shader_image_load_store: enable\n" BOO_GLSL_BINDING_HEAD +
            GenerateVertToFragStruct(0, reflectionType != ReflectionType::None) +
            (!alphaTest ?
            "#ifdef GL_ARB_shader_image_load_store\n"
            "layout(early_fragment_tests) in;\n"
            "#endif\n" : "") +
            "layout(location=0) out vec4 colorOut;\n" +
            texMapDecl +
            "SBINDING(0) in VertToFrag vtf;\n\n" +
            lightingSrc + "\n" +
            "void main()\n{\n";


    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    vec4 lighting = %s(vtf.mvPos, vtf.mvNorm);\n", lighting.m_entry);
        else
            retval += "    vec4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    vec4 sampling%u = texture(tex%u, vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

    if (m_alphaExpr.size())
        retval += "    colorOut = vec4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr + ") * mulColor;\n";
    else
        retval += "    colorOut = vec4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0) * mulColor;\n";

    return retval + (alphaTest ? GenerateAlphaTest() : "") + "}\n";
}

std::string GLSL::makeFrag(const char* glslVer, bool alphaTest,
                           ReflectionType reflectionType,
                           const ShaderFunction& lighting,
                           const ShaderFunction& post,
                           size_t extTexCount, const TextureInfo* extTexs) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;
    else
        lightingSrc = "const vec4 colorReg0 = vec4(1.0);\n"
                      "const vec4 colorReg1 = vec4(1.0);\n"
                      "const vec4 colorReg2 = vec4(1.0);\n"
                      "const vec4 mulColor = vec4(1.0);\n"
                      "\n";

    std::string postSrc;
    if (post.m_source)
        postSrc = post.m_source;

    std::string postEntry;
    if (post.m_entry)
        postEntry = post.m_entry;

    std::string texMapDecl;
    for (unsigned i=0 ; i<m_texMapEnd ; ++i)
        texMapDecl += hecl::Format("TBINDING%u uniform sampler2D tex%u;\n", i, i);
    if (reflectionType == ReflectionType::Indirect)
        texMapDecl += hecl::Format("TBINDING%u uniform sampler2D reflectionIndTex;\n"
                                   "TBINDING%u uniform sampler2D reflectionTex;\n",
                                   m_texMapEnd, m_texMapEnd+1);
    else if (reflectionType == ReflectionType::Simple)
        texMapDecl += hecl::Format("TBINDING%u uniform sampler2D reflectionTex;\n",
                                   m_texMapEnd);

    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        texMapDecl += hecl::Format("TBINDING%u uniform sampler2D extTex%u;\n",
                                   extTex.mapIdx, extTex.mapIdx);
    }

    std::string retval = std::string(glslVer) +
            "\n#extension GL_ARB_shader_image_load_store: enable\n" BOO_GLSL_BINDING_HEAD +
            GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) +
            (!alphaTest ?
            "\n#ifdef GL_ARB_shader_image_load_store\n"
            "layout(early_fragment_tests) in;\n"
            "#endif\n" : "") +
            "\nlayout(location=0) out vec4 colorOut;\n" +
            texMapDecl +
            "SBINDING(0) in VertToFrag vtf;\n\n" +
            lightingSrc + "\n" +
            postSrc +
            "\nvoid main()\n{\n";

    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    vec4 lighting = %s(vtf.mvPos, vtf.mvNorm);\n", lighting.m_entry);
        else
            retval += "    vec4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    vec4 sampling%u = texture(tex%u, vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

    if (m_alphaExpr.size())
        retval += "    colorOut = " + postEntry + "(vec4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr + ")) * mulColor;\n";
    else
        retval += "    colorOut = " + postEntry + "(vec4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0)) * mulColor;\n";

    return retval + (alphaTest ? GenerateAlphaTest() : "") + "}\n";
}

}
namespace Runtime
{

static const char* STD_BLOCKNAMES[] = {HECL_GLSL_VERT_UNIFORM_BLOCK_NAME,
                                       HECL_GLSL_TEXMTX_UNIFORM_BLOCK_NAME};

static const char* STD_TEXNAMES[] =
{
    "tex0",
    "tex1",
    "tex2",
    "tex3",
    "tex4",
    "tex5",
    "tex6",
    "tex7"
};

struct GLSLBackendFactory : IShaderBackendFactory
{
    Backend::GLSL m_backend;

    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const hecl::Frontend::IR& ir,
                                       hecl::Frontend::Diagnostics& diag,
                                       boo::IGraphicsDataFactory::Context& ctx,
                                       boo::IShaderPipeline*& objOut)
    {
        m_backend.reset(ir, diag);
        size_t cachedSz = 3;

        std::string vertSource =
        m_backend.makeVert("#version 330",
                           tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount(), 0, nullptr, tag.getReflectionType());
        cachedSz += vertSource.size() + 1;

        std::string fragSource = m_backend.makeFrag("#version 330",
            tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
            tag.getReflectionType());
        cachedSz += fragSource.size() + 1;

        if (m_backend.m_texMapEnd > 8)
            Log.report(logvisor::Fatal, "maximum of 8 texture maps supported");

        objOut =
        static_cast<boo::GLDataFactory::Context&>(ctx).
                newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                  m_backend.m_texMapEnd, STD_TEXNAMES,
                                  2, STD_BLOCKNAMES,
                                  boo::BlendFactor(m_backend.m_blendSrc),
                                  boo::BlendFactor(m_backend.m_blendDst),
                                  tag.getPrimType(), tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None,
                                  tag.getDepthWrite(), true, false,
                                  tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None);
        if (!objOut)
            Log.report(logvisor::Fatal, "unable to build shader");

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(m_backend.m_texMapEnd);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));
        w.writeString(vertSource);
        w.writeString(fragSource);

        return dataOut;
    }

    boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data,
                                               boo::IGraphicsDataFactory::Context& ctx)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz, false, false);
        atUint8 texMapEnd = r.readUByte();
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());
        std::string vertSource = r.readString();
        std::string fragSource = r.readString();

        if (r.hasError())
            return nullptr;

        if (texMapEnd > 8)
            Log.report(logvisor::Fatal, "maximum of 8 texture maps supported");

        boo::IShaderPipeline* ret =
        static_cast<boo::GLDataFactory::Context&>(ctx).
                newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                  texMapEnd, STD_TEXNAMES,
                                  2, STD_BLOCKNAMES,
                                  blendSrc, blendDst, tag.getPrimType(),
                                  tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None,
                                  tag.getDepthWrite(), true, false,
                                  tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None);
        if (!ret)
            Log.report(logvisor::Fatal, "unable to build shader");
        return ret;
    }

    ShaderCachedData buildExtendedShaderFromIR(const ShaderTag& tag,
                                               const hecl::Frontend::IR& ir,
                                               hecl::Frontend::Diagnostics& diag,
                                               const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                               boo::IGraphicsDataFactory::Context& ctx,
                                               FReturnExtensionShader returnFunc)
    {
        m_backend.reset(ir, diag);
        size_t cachedSz = 3;

        if (m_backend.m_texMapEnd > 8)
            Log.report(logvisor::Fatal, "maximum of 8 texture maps supported");

        std::vector<std::pair<std::string, std::string>> sources;
        sources.reserve(extensionSlots.size());
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            size_t bc = 2;
            const char** bn = STD_BLOCKNAMES;
            if (slot.blockCount)
            {
                bc = slot.blockCount;
                bn = slot.blockNames;
            }

            sources.emplace_back(m_backend.makeVert("#version 330",
                                                    tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                                                    tag.getSkinSlotCount(), tag.getTexMtxCount(), slot.texCount,
                                                    slot.texs, tag.getReflectionType()),
                                 m_backend.makeFrag("#version 330",
                                                    tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
                                                    tag.getReflectionType(), slot.lighting, slot.post, slot.texCount, slot.texs));
            cachedSz += sources.back().first.size() + 1;
            cachedSz += sources.back().second.size() + 1;

            boo::ZTest zTest;
            switch (slot.depthTest)
            {
            case hecl::Backend::ZTest::Original:
            default:
                zTest = tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None;
                break;
            case hecl::Backend::ZTest::None:
                zTest = boo::ZTest::None;
                break;
            case hecl::Backend::ZTest::LEqual:
                zTest = boo::ZTest::LEqual;
                break;
            case hecl::Backend::ZTest::Greater:
                zTest = boo::ZTest::Greater;
                break;
            case hecl::Backend::ZTest::Equal:
                zTest = boo::ZTest::Equal;
                break;
            }

            boo::IShaderPipeline* ret =
            static_cast<boo::GLDataFactory::Context&>(ctx).
                    newShaderPipeline(sources.back().first.c_str(), sources.back().second.c_str(),
                                      8, STD_TEXNAMES, bc, bn,
                                      boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? m_backend.m_blendSrc : slot.srcFactor),
                                      boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? m_backend.m_blendDst : slot.dstFactor),
                                      tag.getPrimType(), zTest, slot.noDepthWrite ? false : tag.getDepthWrite(), !slot.noColorWrite, !slot.noAlphaWrite,
                                      slot.frontfaceCull ? boo::CullMode::Frontface :
                                      (tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None));
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(m_backend.m_texMapEnd);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));
        for (const std::pair<std::string, std::string>& pair : sources)
        {
            w.writeString(pair.first);
            w.writeString(pair.second);
        }

        return dataOut;
    }

    bool buildExtendedShaderFromCache(const ShaderCachedData& data,
                                      const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                      boo::IGraphicsDataFactory::Context& ctx,
                                      FReturnExtensionShader returnFunc)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz, false, false);
        atUint8 texMapEnd = r.readUByte();
        hecl::Backend::BlendFactor blendSrc = hecl::Backend::BlendFactor(r.readUByte());
        hecl::Backend::BlendFactor blendDst = hecl::Backend::BlendFactor(r.readUByte());

        if (r.hasError())
            return false;

        if (texMapEnd > 8)
            Log.report(logvisor::Fatal, "maximum of 8 texture maps supported");

        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            size_t bc = 2;
            const char** bn = STD_BLOCKNAMES;
            if (slot.blockCount)
            {
                bc = slot.blockCount;
                bn = slot.blockNames;
            }

            std::string vertSource = r.readString();
            std::string fragSource = r.readString();

            if (r.hasError())
                return false;

            boo::ZTest zTest;
            switch (slot.depthTest)
            {
            case hecl::Backend::ZTest::Original:
            default:
                zTest = tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None;
                break;
            case hecl::Backend::ZTest::None:
                zTest = boo::ZTest::None;
                break;
            case hecl::Backend::ZTest::LEqual:
                zTest = boo::ZTest::LEqual;
                break;
            case hecl::Backend::ZTest::Greater:
                zTest = boo::ZTest::Greater;
                break;
            case hecl::Backend::ZTest::Equal:
                zTest = boo::ZTest::Equal;
                break;
            }

            boo::IShaderPipeline* ret =
            static_cast<boo::GLDataFactory::Context&>(ctx).
                    newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                      8, STD_TEXNAMES, bc, bn,
                                      boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? blendSrc : slot.srcFactor),
                                      boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? blendDst : slot.dstFactor),
                                      tag.getPrimType(), zTest, slot.noDepthWrite ? false : tag.getDepthWrite(), !slot.noColorWrite, !slot.noAlphaWrite,
                                      slot.frontfaceCull ? boo::CullMode::Frontface :
                                      (tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None));
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }

        return true;
    }
};

IShaderBackendFactory* _NewGLSLBackendFactory()
{
    return new struct GLSLBackendFactory();
}

#if BOO_HAS_VULKAN

struct SPIRVBackendFactory : IShaderBackendFactory
{
    Backend::GLSL m_backend;

    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const hecl::Frontend::IR& ir,
                                       hecl::Frontend::Diagnostics& diag,
                                       boo::IGraphicsDataFactory::Context& ctx,
                                       boo::IShaderPipeline*& objOut)
    {
        m_backend.reset(ir, diag);

        std::string vertSource =
        m_backend.makeVert("#version 330",
                           tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount(), 0, nullptr,
                           tag.getReflectionType());

        std::string fragSource = m_backend.makeFrag("#version 330",
            tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
            tag.getReflectionType());

        std::vector<unsigned int> vertBlob;
        std::vector<unsigned int> fragBlob;
        std::vector<unsigned char> pipelineBlob;

        objOut =
        static_cast<boo::VulkanDataFactory::Context&>(ctx).
                newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                  &vertBlob, &fragBlob, &pipelineBlob, tag.newVertexFormat(ctx),
                                  boo::BlendFactor(m_backend.m_blendSrc), boo::BlendFactor(m_backend.m_blendDst),
                                  tag.getPrimType(), tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None,
                                  tag.getDepthWrite(), true, false,
                                  tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None);
        if (!objOut)
            Log.report(logvisor::Fatal, "unable to build shader");


        atUint32 vertSz = vertBlob.size() * sizeof(unsigned int);
        atUint32 fragSz = fragBlob.size() * sizeof(unsigned int);
        atUint32 pipelineSz = pipelineBlob.size();

        size_t cachedSz = 15 + vertSz + fragSz + pipelineSz;

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_texMapEnd));
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));

        if (vertBlob.size())
        {
            w.writeUint32Big(vertSz);
            w.writeUBytes((atUint8*)vertBlob.data(), vertSz);
        }
        else
            w.writeUint32Big(0);

        if (fragBlob.size())
        {
            w.writeUint32Big(fragSz);
            w.writeUBytes((atUint8*)fragBlob.data(), fragSz);
        }
        else
            w.writeUint32Big(0);

        if (pipelineBlob.size())
        {
            w.writeUint32Big(pipelineSz);
            w.writeUBytes((atUint8*)pipelineBlob.data(), pipelineSz);
        }
        else
            w.writeUint32Big(0);

        return dataOut;
    }

    boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data,
                                               boo::IGraphicsDataFactory::Context& ctx)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz, false, false);
        size_t texCount = size_t(r.readByte());
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());

        atUint32 vertSz = r.readUint32Big();
        std::vector<unsigned int> vertBlob(vertSz / sizeof(unsigned int));
        if (vertSz)
            r.readUBytesToBuf(vertBlob.data(), vertSz);

        atUint32 fragSz = r.readUint32Big();
        std::vector<unsigned int> fragBlob(fragSz / sizeof(unsigned int));
        if (fragSz)
            r.readUBytesToBuf(fragBlob.data(), fragSz);

        atUint32 pipelineSz = r.readUint32Big();
        std::vector<unsigned char> pipelineBlob(pipelineSz);
        if (pipelineSz)
            r.readUBytesToBuf(pipelineBlob.data(), pipelineSz);

        if (r.hasError())
            return nullptr;

        boo::IShaderPipeline* ret =
        static_cast<boo::VulkanDataFactory::Context&>(ctx).
                newShaderPipeline(nullptr, nullptr,
                                  &vertBlob, &fragBlob, &pipelineBlob,
                                  tag.newVertexFormat(ctx),
                                  blendSrc, blendDst, tag.getPrimType(),
                                  tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None,
                                  tag.getDepthWrite(), true, false,
                                  tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None);
        if (!ret)
            Log.report(logvisor::Fatal, "unable to build shader");
        return ret;
    }

    ShaderCachedData buildExtendedShaderFromIR(const ShaderTag& tag,
                                               const hecl::Frontend::IR& ir,
                                               hecl::Frontend::Diagnostics& diag,
                                               const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                               boo::IGraphicsDataFactory::Context& ctx,
                                               FReturnExtensionShader returnFunc)
    {
        m_backend.reset(ir, diag);

        struct Blobs
        {
            std::vector<unsigned int> vert;
            std::vector<unsigned int> frag;
            std::vector<unsigned char> pipeline;
        };
        std::vector<Blobs> pipeBlobs;
        pipeBlobs.reserve(extensionSlots.size());

        size_t cachedSz = 3 + 12 * extensionSlots.size();
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::string vertSource =
            m_backend.makeVert("#version 330",
                               tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                               tag.getSkinSlotCount(), tag.getTexMtxCount(), slot.texCount, slot.texs,
                               tag.getReflectionType());

            std::string fragSource = m_backend.makeFrag("#version 330",
                                                        tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
                                                        tag.getReflectionType(), slot.lighting, slot.post, slot.texCount, slot.texs);
            pipeBlobs.emplace_back();
            Blobs& pipeBlob = pipeBlobs.back();
            boo::IShaderPipeline* ret =
            static_cast<boo::VulkanDataFactory::Context&>(ctx).
                    newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                      &pipeBlob.vert, &pipeBlob.frag, &pipeBlob.pipeline,
                                      tag.newVertexFormat(ctx),
                                      boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ?
                                                           m_backend.m_blendSrc : slot.srcFactor),
                                      boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ?
                                                           m_backend.m_blendDst : slot.dstFactor),
                                      tag.getPrimType(), tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None,
                                      slot.noDepthWrite ? false : tag.getDepthWrite(),
                                      !slot.noColorWrite, !slot.noAlphaWrite,
                                      slot.frontfaceCull ? boo::CullMode::Frontface :
                                      (tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None));
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            cachedSz += pipeBlob.vert.size() * sizeof(unsigned int);
            cachedSz += pipeBlob.frag.size() * sizeof(unsigned int);
            cachedSz += pipeBlob.pipeline.size();
            returnFunc(ret);
        }

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_texMapEnd));
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));

        for (const Blobs& pipeBlob : pipeBlobs)
        {
            size_t vertBlobSz = pipeBlob.vert.size() * sizeof(unsigned int);
            size_t fragBlobSz = pipeBlob.frag.size() * sizeof(unsigned int);
            size_t pipeBlobSz = pipeBlob.pipeline.size();

            if (vertBlobSz)
            {
                w.writeUint32Big(vertBlobSz);
                w.writeUBytes((atUint8*)pipeBlob.vert.data(), vertBlobSz);
            }
            else
                w.writeUint32Big(0);

            if (fragBlobSz)
            {
                w.writeUint32Big(fragBlobSz);
                w.writeUBytes((atUint8*)pipeBlob.frag.data(), fragBlobSz);
            }
            else
                w.writeUint32Big(0);

            if (pipeBlobSz)
            {
                w.writeUint32Big(pipeBlobSz);
                w.writeUBytes((atUint8*)pipeBlob.pipeline.data(), pipeBlobSz);
            }
            else
                w.writeUint32Big(0);
        }

        return dataOut;
    }

    bool buildExtendedShaderFromCache(const ShaderCachedData& data,
                                      const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                      boo::IGraphicsDataFactory::Context& ctx,
                                      FReturnExtensionShader returnFunc)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        size_t texCount = size_t(r.readByte());
        hecl::Backend::BlendFactor blendSrc = hecl::Backend::BlendFactor(r.readUByte());
        hecl::Backend::BlendFactor blendDst = hecl::Backend::BlendFactor(r.readUByte());

        if (r.hasError())
            return false;

        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            atUint32 vertSz = r.readUint32Big();
            std::vector<unsigned int> vertBlob(vertSz / sizeof(unsigned int));
            if (vertSz)
                r.readUBytesToBuf(vertBlob.data(), vertSz);

            atUint32 fragSz = r.readUint32Big();
            std::vector<unsigned int> fragBlob(fragSz / sizeof(unsigned int));
            if (fragSz)
                r.readUBytesToBuf(fragBlob.data(), fragSz);

            atUint32 pipelineSz = r.readUint32Big();
            std::vector<unsigned char> pipelineBlob(pipelineSz);
            if (pipelineSz)
                r.readUBytesToBuf(pipelineBlob.data(), pipelineSz);

            if (r.hasError())
                return false;

            boo::ZTest zTest;
            switch (slot.depthTest)
            {
            case hecl::Backend::ZTest::Original:
            default:
                zTest = tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None;
                break;
            case hecl::Backend::ZTest::None:
                zTest = boo::ZTest::None;
                break;
            case hecl::Backend::ZTest::LEqual:
                zTest = boo::ZTest::LEqual;
                break;
            case hecl::Backend::ZTest::Greater:
                zTest = boo::ZTest::Greater;
                break;
            case hecl::Backend::ZTest::Equal:
                zTest = boo::ZTest::Equal;
                break;
            }

            boo::IShaderPipeline* ret =
            static_cast<boo::VulkanDataFactory::Context&>(ctx).
                    newShaderPipeline(nullptr, nullptr,
                                      &vertBlob, &fragBlob, &pipelineBlob,
                                      tag.newVertexFormat(ctx),
                                      boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? blendSrc : slot.srcFactor),
                                      boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? blendDst : slot.dstFactor),
                                      tag.getPrimType(), zTest, slot.noDepthWrite ? false : tag.getDepthWrite(),
                                      !slot.noColorWrite, !slot.noAlphaWrite,
                                      slot.frontfaceCull ? boo::CullMode::Frontface :
                                      (tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None));
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }

        return true;
    }
};

IShaderBackendFactory* _NewSPIRVBackendFactory()
{
    return new struct SPIRVBackendFactory();
}

#endif

}
}
