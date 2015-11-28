#include "HECL/Backend/Metal.hpp"
#if HECL_HAS_METAL
#include "HECL/Runtime.hpp"
#include <Athena/MemoryReader.hpp>
#include <Athena/MemoryWriter.hpp>
#include <boo/graphicsdev/Metal.hpp>

static LogVisor::LogModule Log("HECL::Backend::Metal");

namespace HECL
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
            return HECL::Format("v.uvIn%u", uvIdx);
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
            return HECL::Format("float4(v.uvIn%u, 0.0, 1.0)", uvIdx);
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
            retval += HECL::Format("    float4 colIn%u [[ attribute(%u) ]];\n", i, idx);
    }
    
    if (uv)
    {
        for (unsigned i=0 ; i<uv ; ++i, ++idx)
            retval += HECL::Format("    float2 uvIn%u [[ attribute(%u) ]];\n", i, idx);
    }
    
    if (w)
    {
        for (unsigned i=0 ; i<w ; ++i, ++idx)
        retval += HECL::Format("    float4 weightIn%u [[ attribute(%u) ]];\n", i, idx);
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
            retval += HECL::Format("    float2 tcgs%" PRISize ";\n", i);
    }
    
    return retval + "};\n";
}

std::string Metal::GenerateVertUniformStruct(unsigned skinSlots, unsigned texMtxs) const
{
    if (skinSlots == 0)
        skinSlots = 1;
    std::string retval = HECL::Format("struct HECLVertUniform\n"
                                      "{\n"
                                      "    float4x4 mv[%u];\n"
                                      "    float4x4 mvInv[%u];\n"
                                      "    float4x4 proj;\n",
                                      skinSlots, skinSlots);
    if (texMtxs)
        retval += HECL::Format("    float4x4 texMtxs[%u];\n", texMtxs);
    return retval + "};\n";
}

void Metal::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "Metal");
}

std::string Metal::makeVert(unsigned col, unsigned uv, unsigned w,
                            unsigned s, unsigned tm) const
{
    std::string retval = "#include <metal_stdlib>\nusing namespace metal;\n" +
    GenerateVertInStruct(col, uv, w) + "\n" +
    GenerateVertToFragStruct() + "\n" +
    GenerateVertUniformStruct(s, tm) +
    "\nvertex VertToFrag vmain(VertData v [[ stage_in ]], constant HECLVertUniform& vu [[ buffer(2) ]])\n{\n"
    "    VertToFrag vtf;\n";
    
    if (s)
    {
        /* skinned */
        retval += "    float4 posAccum = float4(0.0,0.0,0.0,0.0);\n"
        "    float4 normAccum = float4(0.0,0.0,0.0,0.0);\n";
        for (size_t i=0 ; i<s ; ++i)
            retval += HECL::Format("    posAccum += (vu.mv[%" PRISize "] * float4(v.posIn, 1.0)) * v.weightIn%" PRISize "[%" PRISize "];\n"
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
            retval += HECL::Format("    vtf.tcgs%u = %s;\n", tcgIdx,
                                   EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
        else
            retval += HECL::Format("    vtf.tcgs%u = (vu.texMtxs[%u] * %s).xy;\n", tcgIdx, tcg.m_mtx,
                                   EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str());
        ++tcgIdx;
    }
    
    return retval + "    return vtf;\n}\n";
}

std::string Metal::makeFrag(const ShaderFunction& lighting) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;
    
    std::string texMapDecl;
    if (m_texMapEnd)
    {
        for (int i=0 ; i<m_texMapEnd ; ++i)
            texMapDecl += HECL::Format("\n, texture2d<float> tex%u [[ texture(%u) ]]", i, i);
    }
    
    std::string retval = "#include <metal_stdlib>\nusing namespace metal;\n"
    "constexpr sampler samp(address::repeat);\n" +
    GenerateVertToFragStruct() + "\n" +
    lightingSrc + "\n" +
    "fragment float4 fmain(VertToFrag vtf [[ stage_in ]]" + texMapDecl + ")\n{\n";
    
    
    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += HECL::Format("    float4 lighting = %s();\n", lighting.m_entry);
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }
    
    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += HECL::Format("    float4 sampling%u = tex%u.sample(samp, vtf.tcgs%u);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);
    
    if (m_alphaExpr.size())
        retval += "    return float4(" + m_colorExpr + ", " + m_alphaExpr + ");\n";
    else
        retval += "    return float4(" + m_colorExpr + ", 1.0);\n";
    
    return retval + "}\n";
}

std::string Metal::makeFrag(const ShaderFunction& lighting,
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
            texMapDecl += HECL::Format("texture2d<float> tex%u [[ texture(%u) ]],\n", i, i);
    }
    
    std::string retval = "#include <metal_stdlib>\nusing namespace metal;\n"
    "constexpr sampler samp(address::repeat);\n" +
    GenerateVertToFragStruct() + "\n" +
    lightingSrc + "\n" +
    "fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n" + texMapDecl + ")\n{\n";
    
    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += HECL::Format("    float4 lighting = %s();\n", lighting.m_entry);
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }
    
    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += HECL::Format("    float4 sampling%u = tex%u.sample(samp, vtf.tcgs%u);\n",
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
    boo::MetalDataFactory* m_gfxFactory;
    
    MetalBackendFactory(boo::IGraphicsDataFactory* gfxFactory)
    : m_gfxFactory(dynamic_cast<boo::MetalDataFactory*>(gfxFactory)) {}
    
    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const HECL::Frontend::IR& ir,
                                       HECL::Frontend::Diagnostics& diag,
                                       boo::IShaderPipeline** objOut)
    {
        if (!m_rtHint)
            Log.report(LogVisor::FatalError,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");
        
        m_backend.reset(ir, diag);
        size_t cachedSz = 2;
        
        std::string vertSource =
        m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());
        cachedSz += vertSource.size() + 1;
        
        std::string fragSource = m_backend.makeFrag();
        cachedSz += fragSource.size() + 1;
        *objOut =
        m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                        tag.newVertexFormat(m_gfxFactory), m_rtHint,
                                        m_backend.m_blendSrc, m_backend.m_blendDst,
                                        tag.getDepthTest(), tag.getDepthWrite(),
                                        tag.getBackfaceCulling());
        if (!*objOut)
            Log.report(LogVisor::FatalError, "unable to build shader");
        
        ShaderCachedData dataOut(tag, cachedSz);
        Athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));
        w.writeString(vertSource);
        w.writeString(fragSource);
        
        return dataOut;
    }
    
    boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data)
    {
        if (!m_rtHint)
            Log.report(LogVisor::FatalError,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");
        
        const ShaderTag& tag = data.m_tag;
        Athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());
        std::string vertSource = r.readString();
        std::string fragSource = r.readString();
        boo::IShaderPipeline* ret =
        m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                        tag.newVertexFormat(m_gfxFactory), m_rtHint,
                                        blendSrc, blendDst,
                                        tag.getDepthTest(), tag.getDepthWrite(),
                                        tag.getBackfaceCulling());
        if (!ret)
            Log.report(LogVisor::FatalError, "unable to build shader");
        return ret;
    }
    
    ShaderCachedData buildExtendedShaderFromIR(const ShaderTag& tag,
                                               const HECL::Frontend::IR& ir,
                                               HECL::Frontend::Diagnostics& diag,
                                               const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                               FReturnExtensionShader returnFunc)
    {
        if (!m_rtHint)
            Log.report(LogVisor::FatalError,
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
            fragSources.push_back(m_backend.makeFrag(slot.lighting, slot.post));
            cachedSz += fragSources.back().size() + 1;
            boo::IShaderPipeline* ret =
            m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSources.back().c_str(),
                                            tag.newVertexFormat(m_gfxFactory), m_rtHint,
                                            m_backend.m_blendSrc, m_backend.m_blendDst,
                                            tag.getDepthTest(), tag.getDepthWrite(),
                                            tag.getBackfaceCulling());
            if (!ret)
                Log.report(LogVisor::FatalError, "unable to build shader");
            returnFunc(ret);
        }
        
        ShaderCachedData dataOut(tag, cachedSz);
        Athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));
        w.writeString(vertSource);
        for (const std::string src : fragSources)
            w.writeString(src);
        
        return dataOut;
    }
    
    void buildExtendedShaderFromCache(const ShaderCachedData& data,
                                      const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                      FReturnExtensionShader returnFunc)
    {
        if (!m_rtHint)
            Log.report(LogVisor::FatalError,
                       "ShaderCacheManager::setRenderTargetHint must be called before making metal shaders");
        
        const ShaderTag& tag = data.m_tag;
        Athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());
        std::string vertSource = r.readString();
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::string fragSource = r.readString();
            boo::IShaderPipeline* ret =
            m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                            tag.newVertexFormat(m_gfxFactory), m_rtHint,
                                            blendSrc, blendDst,
                                            tag.getDepthTest(), tag.getDepthWrite(),
                                            tag.getBackfaceCulling());
            if (!ret)
                Log.report(LogVisor::FatalError, "unable to build shader");
            returnFunc(ret);
        }
    }
};

IShaderBackendFactory* _NewMetalBackendFactory(boo::IGraphicsDataFactory* gfxFactory)
{
    return new struct MetalBackendFactory(gfxFactory);
}
    
}
}

#endif
