#include "hecl/Backend/Metal.hpp"
#if BOO_HAS_METAL
#include <athena/MemoryReader.hpp>
#include <athena/MemoryWriter.hpp>
#include <boo/graphicsdev/Metal.hpp>

static logvisor::Module Log("hecl::Backend::Metal");

namespace hecl
{
namespace Backend
{

std::string Metal::EmitTexGenSource2(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
        case TexGenSrc::Position:
            return "v.posIn.xy\n";
        case TexGenSrc::Normal:
            return "v.normIn.xy\n";
        case TexGenSrc::UV:
            return hecl::Format("v.uvIn%u", uvIdx);
        default: break;
    }
    return std::string();
}

std::string Metal::EmitTexGenSource4(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
        case TexGenSrc::Position:
            return "float4(v.posIn, 1.0)\n";
        case TexGenSrc::Normal:
            return "float4(v.normIn, 1.0)\n";
        case TexGenSrc::UV:
            return hecl::Format("float4(v.uvIn%u, 0.0, 1.0)", uvIdx);
        default: break;
    }
    return std::string();
}

std::string Metal::GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const
{
    std::string retval =
    "struct VertData\n"
    "{\n"
    "    float3 posIn [[ attribute(0) ]];\n"
    "    float3 normIn [[ attribute(1) ]];\n";

    unsigned idx = 2;
    if (col)
    {
        for (unsigned i=0 ; i<col ; ++i, ++idx)
            retval += hecl::Format("    float4 colIn%u [[ attribute(%u) ]];\n", i, idx);
    }

    if (uv)
    {
        for (unsigned i=0 ; i<uv ; ++i, ++idx)
            retval += hecl::Format("    float2 uvIn%u [[ attribute(%u) ]];\n", i, idx);
    }

    if (w)
    {
        for (unsigned i=0 ; i<w ; ++i, ++idx)
            retval += hecl::Format("    float4 weightIn%u [[ attribute(%u) ]];\n", i, idx);
    }

    return retval + "};\n";
}

std::string Metal::GenerateVertToFragStruct(size_t extTexCount, bool reflectionCoords) const
{
    std::string retval =
    "struct VertToFrag\n"
    "{\n"
    "    float4 mvpPos [[ position ]];\n"
    "    float4 mvPos;\n"
    "    float4 mvNorm;\n";

    if (m_tcgs.size())
        for (size_t i=0 ; i<m_tcgs.size() ; ++i)
            retval += hecl::Format("    float2 tcgs%" PRISize ";\n", i);
    if (extTexCount)
        for (size_t i=0 ; i<extTexCount ; ++i)
            retval += hecl::Format("    float2 extTcgs%" PRISize ";\n", i);

    if (reflectionCoords)
        retval += "    float2 reflectTcgs0;\n"
                  "    float2 reflectTcgs1;\n"
                  "    float reflectAlpha;\n";

    return retval + "};\n";
}

std::string Metal::GenerateVertUniformStruct(unsigned skinSlots) const
{
    if (skinSlots == 0)
        skinSlots = 1;
    std::string retval = hecl::Format("struct HECLVertUniform\n"
                                      "{\n"
                                      "    float4x4 mv[%u];\n"
                                      "    float4x4 mvInv[%u];\n"
                                      "    float4x4 proj;\n"
                                      "};\n"
                                      "struct TexMtxs {float4x4 mtx; float4x4 postMtx;};\n"
                                      "struct ReflectTexMtxs {float4x4 indMtx; float4x4 reflectMtx; float reflectAlpha;};\n",
                                      skinSlots, skinSlots);
    return retval;
}

std::string Metal::GenerateFragOutStruct() const
{
    return "struct FragOut\n"
           "{\n"
           "    float4 color [[ color(0) ]];\n"
           "    //float depth [[ depth(less) ]];\n"
           "};\n";
}

std::string Metal::GenerateAlphaTest() const
{
    return "    if (out.color.a < 0.01)\n"
           "    {\n"
           "        discard_fragment();\n"
           "    }\n";
}

std::string Metal::GenerateReflectionExpr(ReflectionType type) const
{
    switch (type)
    {
    case ReflectionType::None:
    default:
        return "float3(0.0, 0.0, 0.0)";
    case ReflectionType::Simple:
        return "reflectionTex.sample(samp, vtf.reflectTcgs1).rgb * vtf.reflectAlpha";
    case ReflectionType::Indirect:
        return "reflectionTex.sample(samp, (reflectionIndTex.sample(samp, vtf.reflectTcgs0).rg - "
               "float2(0.5, 0.5)) * float2(0.5, 0.5) + vtf.reflectTcgs1).rgb * vtf.reflectAlpha";
    }
}

void Metal::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "Metal");
}

std::string Metal::makeVert(unsigned col, unsigned uv, unsigned w,
                            unsigned s, unsigned tm, size_t extTexCount,
                            const TextureInfo* extTexs, ReflectionType reflectionType) const
{
    std::string tmStr;
    if (tm)
        tmStr = hecl::Format(",\nconstant TexMtxs* texMtxs [[ buffer(3) ]]");
    if (reflectionType != ReflectionType::None)
        tmStr += ",\nconstant ReflectTexMtxs& reflectMtxs [[ buffer(5) ]]";
    std::string retval = "#include <metal_stdlib>\nusing namespace metal;\n" +
    GenerateVertInStruct(col, uv, w) + "\n" +
    GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) + "\n" +
    GenerateVertUniformStruct(s) +
    "\nvertex VertToFrag vmain(VertData v [[ stage_in ]],\n"
    "                          constant HECLVertUniform& vu [[ buffer(2) ]]" + tmStr + ")\n"
    "{\n"
    "    VertToFrag vtf;\n";

    if (s)
    {
        /* skinned */
        retval += "    float4 posAccum = float4(0.0,0.0,0.0,0.0);\n"
        "    float4 normAccum = float4(0.0,0.0,0.0,0.0);\n";
        for (size_t i=0 ; i<s ; ++i)
            retval += hecl::Format("    posAccum += (vu.mv[%" PRISize "] * float4(v.posIn, 1.0)) * v.weightIn%" PRISize "[%" PRISize "];\n"
                                   "    normAccum += (vu.mvInv[%" PRISize "] * float4(v.normIn, 1.0)) * v.weightIn%" PRISize "[%" PRISize "];\n",
                                   i, i/4, i%4, i, i/4, i%4);
        retval += "    posAccum[3] = 1.0;\n"
        "    vtf.mvPos = posAccum;\n"
        "    vtf.mvNorm = float4(normalize(normAccum.xyz), 0.0);\n"
        "    vtf.mvpPos = vu.proj * posAccum;\n";
    }
    else
    {
        /* non-skinned */
        retval += "    vtf.mvPos = vu.mv[0] * float4(v.posIn, 1.0);\n"
        "    vtf.mvNorm = vu.mvInv[0] * float4(v.normIn, 0.0);\n"
        "    vtf.mvpPos = vu.proj * vtf.mvPos;\n";
    }

    retval += "    float4 tmpProj;\n";

    int tcgIdx = 0;
    for (const TexCoordGen& tcg : m_tcgs)
    {
        if (tcg.m_mtx < 0)
            retval += hecl::Format("    vtf.tcgs%u = %s;\n", tcgIdx,
                                   EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
        else
            retval += hecl::Format("    tmpProj = texMtxs[%u].postMtx * float4(%s((texMtxs[%u].mtx * %s).xyz), 1.0);\n"
                                   "    vtf.tcgs%u = (tmpProj / tmpProj.w).xy;\n", tcg.m_mtx,
                                   tcg.m_norm ? "normalize" : "", tcg.m_mtx,
                                   EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str(), tcgIdx);
        ++tcgIdx;
    }

    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        if (extTex.mtxIdx < 0)
            retval += hecl::Format("    vtf.extTcgs%u = %s;\n", i,
                                   EmitTexGenSource2(extTex.src, extTex.uvIdx).c_str());
        else
            retval += hecl::Format("    tmpProj = texMtxs[%u].postMtx * float4(%s((texMtxs[%u].mtx * %s).xyz), 1.0);\n"
                                   "    vtf.extTcgs%u = (tmpProj / tmpProj.w).xy;\n", extTex.mtxIdx,
                                   extTex.normalize ? "normalize" : "", extTex.mtxIdx,
                                   EmitTexGenSource4(extTex.src, extTex.uvIdx).c_str(), i);
    }

    if (reflectionType != ReflectionType::None)
        retval += "    vtf.reflectTcgs0 = normalize((reflectMtxs.indMtx * float4(v.posIn, 1.0)).xz) * float2(0.5, 0.5) + float2(0.5, 0.5);\n"
                  "    vtf.reflectTcgs1 = (reflectMtxs.reflectMtx * float4(v.posIn, 1.0)).xy;\n"
                  "    vtf.reflectAlpha = reflectMtxs.reflectAlpha;\n";

    return retval + "    return vtf;\n}\n";
}

std::string Metal::makeFrag(size_t blockCount, const char** blockNames, bool alphaTest,
                            ReflectionType reflectionType, const ShaderFunction& lighting) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;

    std::string texMapDecl;
    if (m_texMapEnd)
        for (int i=0 ; i<m_texMapEnd ; ++i)
            texMapDecl += hecl::Format(",\ntexture2d<float> tex%u [[ texture(%u) ]]", i, i);
    if (reflectionType == ReflectionType::Indirect)
        texMapDecl += hecl::Format(",\ntexture2d<float> reflectionIndTex [[ texture(%u) ]]\n"
                                   ",\ntexture2d<float> reflectionTex [[ texture(%u) ]]\n",
                                   m_texMapEnd, m_texMapEnd+1);
    else if (reflectionType == ReflectionType::Simple)
        texMapDecl += hecl::Format(",\ntexture2d<float> reflectionTex [[ texture(%u) ]]\n",
                                   m_texMapEnd);

    std::string blockCall;
    for (size_t i=0 ; i<blockCount ; ++i)
    {
        texMapDecl += hecl::Format(",\nconstant %s& block%" PRISize " [[ buffer(%" PRISize ") ]]", blockNames[i], i, i + 4);
        if (blockCall.size())
            blockCall += ", ";
        blockCall += hecl::Format("block%" PRISize, i);
    }

    std::string retval = "#include <metal_stdlib>\nusing namespace metal;\n"
    "constexpr sampler samp(address::repeat, filter::linear, mip_filter::linear);\n"
    "constexpr sampler clampSamp(address::clamp_to_border, border_color::opaque_white, filter::linear, mip_filter::linear);\n" +
    GenerateVertToFragStruct(0, reflectionType != ReflectionType::None) + "\n" +
    GenerateFragOutStruct() + "\n" +
    lightingSrc + "\n" +
    "fragment FragOut fmain(VertToFrag vtf [[ stage_in ]]" + texMapDecl + ")\n"
    "{\n"
    "    FragOut out;\n";

    if (lighting.m_source)
    {
        retval += "    float4 colorReg0 = block0.colorReg0;\n"
                  "    float4 colorReg1 = block0.colorReg1;\n"
                  "    float4 colorReg2 = block0.colorReg2;\n"
                  "    float4 mulColor = block0.mulColor;\n";
    }
    else
    {
        retval += "    float4 colorReg0 = float4(1.0, 1.0, 1.0, 1.0);\n"
                  "    float4 colorReg1 = float4(1.0, 1.0, 1.0, 1.0);\n"
                  "    float4 colorReg2 = float4(1.0, 1.0, 1.0, 1.0);\n"
                  "    float4 mulColor = float4(1.0, 1.0, 1.0, 1.0);\n";
    }

    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    float4 lighting = %s(%s, vtf.mvPos, vtf.mvNorm, vtf);\n", lighting.m_entry, blockCall.c_str());
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    float4 sampling%u = tex%u.sample(samp, vtf.tcgs%u);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

    if (m_alphaExpr.size())
        retval += "    out.color = float4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr + ") * mulColor;\n";
    else
        retval += "    out.color = float4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0) * mulColor;\n";

    return retval + (alphaTest ? GenerateAlphaTest() : "") +
           "    //out.depth = 1.0 - float(int((1.0 - vtf.mvpPos.z) * 16777216.0)) / 16777216.0;\n"
           "    return out;\n"
           "}\n";
}

std::string Metal::makeFrag(size_t blockCount, const char** blockNames, bool alphaTest,
                            ReflectionType reflectionType, const ShaderFunction& lighting,
                            const ShaderFunction& post, size_t extTexCount,
                            const TextureInfo* extTexs) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;

    std::string postSrc;
    if (post.m_source)
        postSrc = post.m_source;

    std::string lightingEntry;
    if (lighting.m_entry)
        lightingEntry = lighting.m_entry;

    std::string postEntry;
    if (post.m_entry)
        postEntry = post.m_entry;

    int extTexBits = 0;
    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        extTexBits |= 1 << extTex.mapIdx;
    }

    std::string texMapDecl;
    if (m_texMapEnd)
        for (int i=0 ; i<m_texMapEnd ; ++i)
            if (!(extTexBits & (1 << i)))
                texMapDecl += hecl::Format(",\ntexture2d<float> tex%u [[ texture(%u) ]]", i, i);
    if (reflectionType == ReflectionType::Indirect)
        texMapDecl += hecl::Format(",\ntexture2d<float> reflectionIndTex [[ texture(%u) ]]\n"
                                   ",\ntexture2d<float> reflectionTex [[ texture(%u) ]]\n",
                                   m_texMapEnd, m_texMapEnd+1);
    else if (reflectionType == ReflectionType::Simple)
        texMapDecl += hecl::Format(",\ntexture2d<float> reflectionTex [[ texture(%u) ]]\n",
                                   m_texMapEnd);

    std::string extTexCall;
    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        if (extTexCall.size())
            extTexCall += ", ";
        extTexCall += hecl::Format("tex%u", extTex.mapIdx);
        texMapDecl += hecl::Format(",\ntexture2d<float> tex%u [[ texture(%u) ]]", extTex.mapIdx, extTex.mapIdx);
        extTexBits |= 1 << extTex.mapIdx;
    }

    std::string blockCall;
    for (size_t i=0 ; i<blockCount ; ++i)
    {
        texMapDecl += hecl::Format(",\nconstant %s& block%" PRISize " [[ buffer(%" PRISize ") ]]", blockNames[i], i, i + 4);
        if (blockCall.size())
            blockCall += ", ";
        blockCall += hecl::Format("block%" PRISize, i);
    }

    std::string retval = "#include <metal_stdlib>\nusing namespace metal;\n"
    "constexpr sampler samp(address::repeat, filter::linear, mip_filter::linear);\n"
    "constexpr sampler clampSamp(address::clamp_to_border, border_color::opaque_white, filter::linear, mip_filter::linear);\n" +
    GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) + "\n" +
    GenerateFragOutStruct() + "\n" +
    lightingSrc + "\n" +
    postSrc + "\n" +
    "fragment FragOut fmain(VertToFrag vtf [[ stage_in ]]" + texMapDecl + ")\n"
    "{\n"
    "    FragOut out;\n";

    if (lighting.m_source)
    {
        retval += "    float4 colorReg0 = block0.colorReg0;\n"
                  "    float4 colorReg1 = block0.colorReg1;\n"
                  "    float4 colorReg2 = block0.colorReg2;\n"
                  "    float4 mulColor = block0.mulColor;\n";
    }
    else
    {
        retval += "    float4 colorReg0 = float4(1.0, 1.0, 1.0, 1.0);\n"
                  "    float4 colorReg1 = float4(1.0, 1.0, 1.0, 1.0);\n"
                  "    float4 colorReg2 = float4(1.0, 1.0, 1.0, 1.0);\n"
                  "    float4 mulColor = float4(1.0, 1.0, 1.0, 1.0);\n";
    }

    if (m_lighting)
    {
        if (lighting.m_entry)
        {
                retval += "    float4 lighting = " + lightingEntry + "(" + blockCall + ", vtf.mvPos, vtf.mvNorm, vtf" +
                    (!strncmp(lighting.m_entry, "EXT", 3) ? (extTexCall.size() ? (", " + extTexCall) : "") : "") + ");\n";
        }
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    float4 sampling%u = tex%u.sample(samp, vtf.tcgs%u);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

    if (m_alphaExpr.size())
    {
        retval += "    out.color = " + postEntry + "(" +
                  (postEntry.size() ? ("vtf, " + (blockCall.size() ? (blockCall + ", ") : "") +
                      (!strncmp(post.m_entry, "EXT", 3) ? (extTexCall.size() ? (extTexCall + ", ") : "") : "")) : "") +
                  "float4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr + ")) * mulColor;\n";
    }
    else
    {
        retval += "    out.color = " + postEntry + "(" +
                  (postEntry.size() ? ("vtf, " + (blockCall.size() ? (blockCall + ", ") : "") +
                      (!strncmp(post.m_entry, "EXT", 3) ? (extTexCall.size() ? (extTexCall + ", ") : "") : "")) : "") +
                  "float4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0)) * mulColor;\n";
    }

    return retval + (alphaTest ? GenerateAlphaTest() : "") +
           "    //out.depth = 1.0 - float(int((1.0 - vtf.mvpPos.z) * 16777216.0)) / 16777216.0;\n"
           "    return out;\n"
           "}\n";
}

}
namespace Runtime
{

struct MetalBackendFactory : IShaderBackendFactory
{
    Backend::Metal m_backend;

    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const hecl::Frontend::IR& ir,
                                       hecl::Frontend::Diagnostics& diag,
                                       boo::IGraphicsDataFactory::Context& ctx,
                                       boo::ObjToken<boo::IShaderPipeline>& objOut)
    {
        if (!m_rtHint)
            Log.report(logvisor::Fatal,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");

        m_backend.reset(ir, diag);
        size_t cachedSz = 2;

        std::string vertSource =
        m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount(), 0, nullptr, tag.getReflectionType());

        std::string fragSource = m_backend.makeFrag(0, nullptr,
            tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
            tag.getReflectionType());

        std::vector<uint8_t> vertBlob;
        std::vector<uint8_t> fragBlob;
        objOut =
        static_cast<boo::MetalDataFactory::Context&>(ctx).
            newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                              &vertBlob, &fragBlob,
                              tag.newVertexFormat(ctx), m_rtHint,
                              boo::BlendFactor(m_backend.m_blendSrc),
                              boo::BlendFactor(m_backend.m_blendDst),
                              tag.getPrimType(),
                              tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None, tag.getDepthWrite(), true, true,
                              tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None);
        if (!objOut)
            Log.report(logvisor::Fatal, "unable to build shader");

        cachedSz += vertBlob.size() + 4;
        cachedSz += fragBlob.size() + 4;

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));
        w.writeUint32Big(vertBlob.size());
        w.writeUBytes(vertBlob.data(), vertBlob.size());
        w.writeUint32Big(fragBlob.size());
        w.writeUBytes(fragBlob.data(), fragBlob.size());

        return dataOut;
    }

    boo::ObjToken<boo::IShaderPipeline> buildShaderFromCache(const ShaderCachedData& data,
                                                             boo::IGraphicsDataFactory::Context& ctx)
    {
        if (!m_rtHint)
            Log.report(logvisor::Fatal,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");

        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz, false, false);
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());
        std::vector<uint8_t> vertBlob;
        std::vector<uint8_t> fragBlob;
        atUint32 vertLen = r.readUint32Big();
        if (vertLen)
        {
            vertBlob.resize(vertLen);
            r.readUBytesToBuf(&vertBlob[0], vertLen);
        }
        atUint32 fragLen = r.readUint32Big();
        if (fragLen)
        {
            fragBlob.resize(fragLen);
            r.readUBytesToBuf(&fragBlob[0], fragLen);
        }

        if (r.hasError())
            return nullptr;

        auto ret =
        static_cast<boo::MetalDataFactory::Context&>(ctx).
            newShaderPipeline(nullptr, nullptr,
                              &vertBlob, &fragBlob,
                              tag.newVertexFormat(ctx), m_rtHint,
                              blendSrc, blendDst, tag.getPrimType(),
                              tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None, tag.getDepthWrite(), true, true,
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
        if (!m_rtHint)
            Log.report(logvisor::Fatal,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");

        m_backend.reset(ir, diag);
        size_t cachedSz = 2;

        std::vector<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> blobs;
        blobs.reserve(extensionSlots.size());
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::string vertSource =
                m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                                   tag.getSkinSlotCount(), tag.getTexMtxCount(), slot.texCount, slot.texs,
                                   slot.noReflection ? Backend::ReflectionType::None : tag.getReflectionType());
            std::string fragSource =
                m_backend.makeFrag(slot.blockCount, slot.blockNames,
                                   tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
                                   slot.noReflection ? Backend::ReflectionType::None : tag.getReflectionType(),
                                   slot.lighting, slot.post, slot.texCount, slot.texs);

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

            blobs.emplace_back();
            auto ret =
            static_cast<boo::MetalDataFactory::Context&>(ctx).
                newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                  &blobs.back().first, &blobs.back().second,
                                  tag.newVertexFormat(ctx), m_rtHint,
                                  boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? m_backend.m_blendSrc : slot.srcFactor),
                                  boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? m_backend.m_blendDst : slot.dstFactor),
                                  tag.getPrimType(), zTest, slot.noDepthWrite ? false : tag.getDepthWrite(),
                                  !slot.noColorWrite, !slot.noAlphaWrite,
                                  (slot.cullMode == hecl::Backend::CullMode::Original) ?
                                  (tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None) :
                                  boo::CullMode(slot.cullMode));
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");

            cachedSz += blobs.back().first.size() + 4;
            cachedSz += blobs.back().second.size() + 4;
            returnFunc(ret);
        }

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));
        for (auto& blob : blobs)
        {
            w.writeUint32Big(blob.first.size());
            w.writeUBytes(blob.first.data(), blob.first.size());
            w.writeUint32Big(blob.second.size());
            w.writeUBytes(blob.second.data(), blob.second.size());
        }

        return dataOut;
    }

    bool buildExtendedShaderFromCache(const ShaderCachedData& data,
                                      const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                      boo::IGraphicsDataFactory::Context& ctx,
                                      FReturnExtensionShader returnFunc)
    {
        if (!m_rtHint)
            Log.report(logvisor::Fatal,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");

        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz, false, false);
        hecl::Backend::BlendFactor blendSrc = hecl::Backend::BlendFactor(r.readUByte());
        hecl::Backend::BlendFactor blendDst = hecl::Backend::BlendFactor(r.readUByte());

        if (r.hasError())
            return false;

        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::vector<uint8_t> vertBlob;
            std::vector<uint8_t> fragBlob;
            atUint32 vertLen = r.readUint32Big();
            if (vertLen)
            {
                vertBlob.resize(vertLen);
                r.readUBytesToBuf(&vertBlob[0], vertLen);
            }
            atUint32 fragLen = r.readUint32Big();
            if (fragLen)
            {
                fragBlob.resize(fragLen);
                r.readUBytesToBuf(&fragBlob[0], fragLen);
            }

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

            auto ret =
            static_cast<boo::MetalDataFactory::Context&>(ctx).
                newShaderPipeline(nullptr, nullptr,
                                  &vertBlob, &fragBlob,
                                  tag.newVertexFormat(ctx), m_rtHint,
                                  boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? blendSrc : slot.srcFactor),
                                  boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? blendDst : slot.dstFactor),
                                  tag.getPrimType(), zTest, slot.noDepthWrite ? false : tag.getDepthWrite(),
                                  !slot.noColorWrite, !slot.noAlphaWrite,
                                  (slot.cullMode == hecl::Backend::CullMode::Original) ?
                                  (tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None) :
                                  boo::CullMode(slot.cullMode));
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }

        return true;
    }
};

IShaderBackendFactory* _NewMetalBackendFactory()
{
    return new struct MetalBackendFactory();
}

}
}

#endif
