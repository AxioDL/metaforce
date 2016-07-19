#include "hecl/Backend/Metal.hpp"
#if BOO_HAS_METAL
#include "hecl/Runtime.hpp"
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

std::string Metal::GenerateVertToFragStruct() const
{
    std::string retval =
    "struct VertToFrag\n"
    "{\n"
    "    float4 mvpPos [[ position ]];\n"
    "    float4 mvPos;\n"
    "    float4 mvNorm;\n";

    if (m_tcgs.size())
    {
        for (size_t i=0 ; i<m_tcgs.size() ; ++i)
            retval += hecl::Format("    float2 tcgs%" PRISize ";\n", i);
    }

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
                                      "};\n",
                                      skinSlots, skinSlots);
    return retval;
}

void Metal::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "Metal");
}

std::string Metal::makeVert(unsigned col, unsigned uv, unsigned w,
                            unsigned s, unsigned tm) const
{
    std::string tmStr;
    if (tm)
        tmStr = hecl::Format(",\nconstant float4x4 texMtxs[%u] [[ buffer(3) ]]", tm);
    std::string retval = "#include <metal_stdlib>\nusing namespace metal;\n" +
    GenerateVertInStruct(col, uv, w) + "\n" +
    GenerateVertToFragStruct() + "\n" +
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

    int tcgIdx = 0;
    for (const TexCoordGen& tcg : m_tcgs)
    {
        if (tcg.m_mtx < 0)
            retval += hecl::Format("    vtf.tcgs%u = %s;\n", tcgIdx,
                                   EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
        else
            retval += hecl::Format("    vtf.tcgs[%u] = (texMtxs[%u].postMtx * %s(texMtxs[%u].mtx * %s)).xy;\n", tcgIdx, tcg.m_mtx,
                                   tcg.m_norm ? "normalize" : "", tcg.m_mtx, EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str());
        ++tcgIdx;
    }

    return retval + "    return vtf;\n}\n";
}

std::string Metal::makeFrag(size_t blockCount, const char** blockNames,
                            const ShaderFunction& lighting) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;

    std::string texMapDecl;
    if (m_texMapEnd)
    {
        for (int i=0 ; i<m_texMapEnd ; ++i)
            texMapDecl += hecl::Format(",\ntexture2d<float> tex%u [[ texture(%u) ]]", i, i);
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
    "constexpr sampler samp(address::repeat, filter::linear, mip_filter::linear);\n" +
    GenerateVertToFragStruct() + "\n" +
    lightingSrc + "\n" +
    "fragment float4 fmain(VertToFrag vtf [[ stage_in ]]" + texMapDecl + ")\n{\n";


    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    float4 lighting = %s(%s, vtf.mvPos, vtf.mvNorm);\n", lighting.m_entry, blockCall.c_str());
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    float4 sampling%u = tex%u.sample(samp, vtf.tcgs%u);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    if (m_alphaExpr.size())
        retval += "    return float4(" + m_colorExpr + ", " + m_alphaExpr + ");\n";
    else
        retval += "    return float4(" + m_colorExpr + ", 1.0);\n";

    return retval + "}\n";
}

std::string Metal::makeFrag(size_t blockCount, const char** blockNames,
                            const ShaderFunction& lighting,
                            const ShaderFunction& post) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;

    std::string postSrc;
    if (post.m_source)
        postSrc = post.m_source;

    std::string postEntry;
    if (post.m_entry)
        postEntry = post.m_entry;

    std::string texMapDecl;
    if (m_texMapEnd)
    {
        for (int i=0 ; i<m_texMapEnd ; ++i)
            texMapDecl += hecl::Format(",\ntexture2d<float> tex%u [[ texture(%u) ]]", i, i);
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
    "constexpr sampler samp(address::repeat, filter::linear, mip_filter::linear);\n" +
    GenerateVertToFragStruct() + "\n" +
    lightingSrc + "\n" +
    "fragment float4 fmain(VertToFrag vtf [[ stage_in ]]" + texMapDecl + ")\n{\n";

    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    float4 lighting = %s(%s, vtf.mvPos, vtf.mvNorm);\n", lighting.m_entry, blockCall.c_str());
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    float4 sampling%u = tex%u.sample(samp, vtf.tcgs%u);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    if (m_alphaExpr.size())
        retval += "    return " + postEntry + "(float4(" + m_colorExpr + ", " + m_alphaExpr + "));\n";
    else
        retval += "    return " + postEntry + "(float4(" + m_colorExpr + ", 1.0));\n";

    return retval + "}\n";
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
                                       boo::IShaderPipeline*& objOut)
    {
        if (!m_rtHint)
            Log.report(logvisor::Fatal,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");

        m_backend.reset(ir, diag);
        size_t cachedSz = 2;

        std::string vertSource =
        m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());
        cachedSz += vertSource.size() + 1;

        std::string fragSource = m_backend.makeFrag(0, nullptr);
        cachedSz += fragSource.size() + 1;
        objOut =
        static_cast<boo::MetalDataFactory::Context&>(ctx).
            newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                              tag.newVertexFormat(ctx), m_rtHint,
                              m_backend.m_blendSrc, m_backend.m_blendDst,
                              tag.getPrimType(),
                              tag.getDepthTest(), tag.getDepthWrite(),
                              tag.getBackfaceCulling());
        if (!objOut)
            Log.report(logvisor::Fatal, "unable to build shader");

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));
        w.writeString(vertSource);
        w.writeString(fragSource);

        return dataOut;
    }

    boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data,
                                               boo::IGraphicsDataFactory::Context& ctx)
    {
        if (!m_rtHint)
            Log.report(logvisor::Fatal,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");

        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());
        std::string vertSource = r.readString();
        std::string fragSource = r.readString();
        boo::IShaderPipeline* ret =
        static_cast<boo::MetalDataFactory::Context&>(ctx).
            newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                              tag.newVertexFormat(ctx), m_rtHint,
                              blendSrc, blendDst, tag.getPrimType(),
                              tag.getDepthTest(), tag.getDepthWrite(),
                              tag.getBackfaceCulling());
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

        std::string vertSource =
        m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());
        cachedSz += vertSource.size() + 1;

        std::vector<std::string> fragSources;
        fragSources.reserve(extensionSlots.size());
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            fragSources.push_back(m_backend.makeFrag(slot.blockCount, slot.blockNames, slot.lighting, slot.post));
            cachedSz += fragSources.back().size() + 1;
            boo::IShaderPipeline* ret =
            static_cast<boo::MetalDataFactory::Context&>(ctx).
                newShaderPipeline(vertSource.c_str(), fragSources.back().c_str(),
                                  tag.newVertexFormat(ctx), m_rtHint,
                                  m_backend.m_blendSrc, m_backend.m_blendDst,
                                  tag.getPrimType(),
                                  tag.getDepthTest(), tag.getDepthWrite(),
                                  tag.getBackfaceCulling());
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));
        w.writeString(vertSource);
        for (const std::string src : fragSources)
            w.writeString(src);

        return dataOut;
    }

    void buildExtendedShaderFromCache(const ShaderCachedData& data,
                                      const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                      boo::IGraphicsDataFactory::Context& ctx,
                                      FReturnExtensionShader returnFunc)
    {
        if (!m_rtHint)
            Log.report(logvisor::Fatal,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");

        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());
        std::string vertSource = r.readString();
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::string fragSource = r.readString();
            boo::IShaderPipeline* ret =
            static_cast<boo::MetalDataFactory::Context&>(ctx).
                newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                  tag.newVertexFormat(ctx), m_rtHint,
                                  blendSrc, blendDst, tag.getPrimType(),
                                  tag.getDepthTest(), tag.getDepthWrite(),
                                  tag.getBackfaceCulling());
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }
    }
};

IShaderBackendFactory* _NewMetalBackendFactory()
{
    return new struct MetalBackendFactory();
}

}
}

#endif
