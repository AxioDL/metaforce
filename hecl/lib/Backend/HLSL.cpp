#include "HECL/Backend/HLSL.hpp"
#include "HECL/Runtime.hpp"
#include <Athena/MemoryReader.hpp>
#include <Athena/MemoryWriter.hpp>
#include <boo/graphicsdev/D3D.hpp>

static LogVisor::LogModule Log("HECL::Backend::HLSL");

namespace HECL
{
namespace Backend
{

std::string HLSL::EmitTexGenSource2(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TG_POS:
        return "v.posIn.xy\n";
    case TG_NRM:
        return "v.normIn.xy\n";
    case TG_UV:
        return HECL::Format("v.uvIn[%u]", uvIdx);
    default: break;
    }
    return std::string();
}

std::string HLSL::EmitTexGenSource4(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TG_POS:
        return "float4(v.posIn, 1.0)\n";
    case TG_NRM:
        return "float4(v.normIn, 1.0)\n";
    case TG_UV:
        return HECL::Format("float4(v.uvIn[%u], 0.0, 1.0)", uvIdx);
    default: break;
    }
    return std::string();
}

std::string HLSL::GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const
{
    std::string retval =
    "struct VertData\n"
    "{\n"
    "    float3 posIn : POSITION;\n"
    "    float3 normIn : NORMAL;\n";

    if (col)
        retval += HECL::Format("    float4 colIn[%u] : COLOR;\n", col);

    if (uv)
        retval += HECL::Format("    float2 uvIn[%u] : UV;\n", uv);

    if (w)
        retval += HECL::Format("    float4 weightIn[%u] : WEIGHT;\n", w);

    return retval + "};\n";
}

std::string HLSL::GenerateVertToFragStruct() const
{
    std::string retval =
    "struct VertToFrag\n"
    "{\n"
    "    float4 mvpPos : SV_Position;\n"
    "    float4 mvPos : POSITION;\n"
    "    float4 mvNorm : NORMAL;\n";

    if (m_tcgs.size())
        retval += HECL::Format("    float2 tcgs[%u] : UV;\n", unsigned(m_tcgs.size()));

    return retval + "};\n";
}

std::string HLSL::GenerateVertUniformStruct(unsigned skinSlots, unsigned texMtxs) const
{
    if (skinSlots == 0)
        skinSlots = 1;
    std::string retval = HECL::Format("cbuffer HECLVertUniform : register(b0)\n"
                                      "{\n"
                                      "    float4x4 mv[%u];\n"
                                      "    float4x4 mvInv[%u];\n"
                                      "    float4x4 proj;\n",
                                      skinSlots, skinSlots);
    if (texMtxs)
        retval += HECL::Format("    float4x4 texMtxs[%u];\n", texMtxs);
    return retval + "};\n";
}

void HLSL::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "HLSL");
}

std::string HLSL::makeVert(unsigned col, unsigned uv, unsigned w,
                           unsigned s, unsigned tm) const
{
    std::string retval =
            GenerateVertInStruct(col, uv, w) + "\n" +
            GenerateVertToFragStruct() + "\n" +
            GenerateVertUniformStruct(s, tm) + "\n" +
            "VertToFrag main(in VertData v)\n"
            "{\n"
            "    VertToFrag vtf;\n";

    if (s)
    {
        /* skinned */
        retval += "    vec4 posAccum = float4(0.0,0.0,0.0,0.0);\n"
                  "    vec4 normAccum = float4(0.0,0.0,0.0,0.0);\n";
        for (size_t i=0 ; i<s ; ++i)
            retval += HECL::Format("    posAccum += mul(mv[%" PRISize "], float4(v.posIn, 1.0)) * v.weightIn[%" PRISize "][%" PRISize "];\n"
                                   "    normAccum += mul(mvInv[%" PRISize "], float4(v.normIn, 1.0)) * v.weightIn[%" PRISize "][%" PRISize "];\n",
                                   i, i/4, i%4, i, i/4, i%4);
        retval += "    posAccum[3] = 1.0;\n"
                  "    vtf.mvPos = posAccum;\n"
                  "    vtf.mvNorm = float4(normalize(normAccum.xyz), 0.0);\n"
                  "    vtf.mvpPos = mul(proj, posAccum);\n";
    }
    else
    {
        /* non-skinned */
        retval += "    vtf.mvPos = mul(mv[0], float4(v.posIn, 1.0));\n"
                  "    vtf.mvNorm = mul(mvInv[0], float4(v.normIn, 0.0));\n"
                  "    vtf.mvpPos = mul(proj, vtf.mvPos);\n";
    }

    int tcgIdx = 0;
    for (const TexCoordGen& tcg : m_tcgs)
    {
        if (tcg.m_mtx < 0)
            retval += HECL::Format("    vtf.tcgs[%u] = %s;\n", tcgIdx,
                                   EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
        else
            retval += HECL::Format("    vtf.tcgs[%u] = mul(texMtxs[%u], %s).xy;\n", tcgIdx, tcg.m_mtx,
                                   EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str());
        ++tcgIdx;
    }

    return retval + "    return vtf;\n"
                    "}\n";
}

std::string HLSL::makeFrag(const ShaderFunction& lighting) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;

    std::string texMapDecl;
    if (m_texMapEnd)
        texMapDecl = HECL::Format("Texture2D texs[%u] : register(t0);\n", m_texMapEnd);

    std::string retval =
            "SamplerState samp : register(s0);\n" +
            GenerateVertToFragStruct() +
            texMapDecl + "\n" +
            lightingSrc + "\n" +
            "float4 main(in VertToFrag vtf) : SV_Target0\n{\n";


    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += HECL::Format("    float4 lighting = %s();\n", lighting.m_entry);
        else
            retval += "    float4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += HECL::Format("    float4 sampling%u = texs[%u].Sample(samp, vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    if (m_alphaExpr.size())
        retval += "    return float4(" + m_colorExpr + ", " + m_alphaExpr + ");\n";
    else
        retval += "    return float4(" + m_colorExpr + ", 1.0);\n";

    return retval + "}\n";
}

std::string HLSL::makeFrag(const ShaderFunction& lighting,
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
        texMapDecl = HECL::Format("Texture2D texs[%u] : register(t0);\n", m_texMapEnd);

    std::string retval =
            "SamplerState samp : register(s0);\n" +
            GenerateVertToFragStruct() +
            texMapDecl + "\n" +
            lightingSrc + "\n" +
            postSrc +
            "\nfloat4 main(in VertToFrag vtf) : SV_Target0\n{\n";

    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += HECL::Format("    float4 lighting = %s();\n", lighting.m_entry);
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += HECL::Format("    float4 sampling%u = texs[%u].Sample(samp, vtf.tcgs[%u]);\n",
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

struct HLSLBackendFactory : IShaderBackendFactory
{
    Backend::HLSL m_backend;
    boo::ID3DDataFactory* m_gfxFactory;

    HLSLBackendFactory(boo::IGraphicsDataFactory* gfxFactory)
    : m_gfxFactory(dynamic_cast<boo::ID3DDataFactory*>(gfxFactory)) {}

    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const HECL::Frontend::IR& ir,
                                       HECL::Frontend::Diagnostics& diag,
                                       boo::IShaderPipeline** objOut)
    {
        m_backend.reset(ir, diag);

        std::string vertSource =
        m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());

        std::string fragSource = m_backend.makeFrag();
        ComPtr<ID3DBlob> vertBlob;
        ComPtr<ID3DBlob> fragBlob;
        ComPtr<ID3DBlob> pipelineBlob;
        *objOut =
        m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                        vertBlob, fragBlob, pipelineBlob,
                                        tag.newVertexFormat(m_gfxFactory),
                                        m_backend.m_blendSrc, m_backend.m_blendDst,
                                        tag.getDepthTest(), tag.getDepthWrite(),
                                        tag.getBackfaceCulling());
        if (!*objOut)
            Log.report(LogVisor::FatalError, "unable to build shader");

        atUint32 vertSz = 0;
        atUint32 fragSz = 0;
        atUint32 pipelineSz = 0;
        if (vertBlob)
            vertSz = vertBlob->GetBufferSize();
        if (fragBlob)
            fragSz = fragBlob->GetBufferSize();
        if (pipelineBlob)
            pipelineSz = pipelineBlob->GetBufferSize();

        size_t cachedSz = 14 + vertSz + fragSz + pipelineSz;

        ShaderCachedData dataOut(tag, cachedSz);
        Athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(m_backend.m_blendSrc);
        w.writeUByte(m_backend.m_blendDst);

        if (vertBlob)
        {
            w.writeUint32Big(vertSz);
            w.writeUBytes((atUint8*)vertBlob->GetBufferPointer(), vertSz);
        }
        else
            w.writeUint32Big(0);

        if (fragBlob)
        {
            w.writeUint32Big(fragSz);
            w.writeUBytes((atUint8*)fragBlob->GetBufferPointer(), fragSz);
        }
        else
            w.writeUint32Big(0);

        if (pipelineBlob)
        {
            w.writeUint32Big(pipelineSz);
            w.writeUBytes((atUint8*)pipelineBlob->GetBufferPointer(), pipelineSz);
        }
        else
            w.writeUint32Big(0);

        return dataOut;
    }

    boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data)
    {
        const ShaderTag& tag = data.m_tag;
        Athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());

        atUint32 vertSz = r.readUint32Big();
        ComPtr<ID3DBlob> vertBlob;
        if (vertSz)
        {
            D3DCreateBlobPROC(vertSz, &vertBlob);
            r.readUBytesToBuf(vertBlob->GetBufferPointer(), vertSz);
        }

        atUint32 fragSz = r.readUint32Big();
        ComPtr<ID3DBlob> fragBlob;
        if (fragSz)
        {
            D3DCreateBlobPROC(fragSz, &fragBlob);
            r.readUBytesToBuf(fragBlob->GetBufferPointer(), fragSz);
        }

        atUint32 pipelineSz = r.readUint32Big();
        ComPtr<ID3DBlob> pipelineBlob;
        if (pipelineSz)
        {
            D3DCreateBlobPROC(pipelineSz, &pipelineBlob);
            r.readUBytesToBuf(pipelineBlob->GetBufferPointer(), pipelineSz);
        }

        boo::IShaderPipeline* ret =
        m_gfxFactory->newShaderPipeline(nullptr, nullptr,
                                        vertBlob, fragBlob, pipelineBlob,
                                        tag.newVertexFormat(m_gfxFactory),
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
        m_backend.reset(ir, diag);

        std::string vertSource =
        m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());

        ComPtr<ID3DBlob> vertBlob;
        std::vector<std::pair<ComPtr<ID3DBlob>, ComPtr<ID3DBlob>>> fragPipeBlobs;
        fragPipeBlobs.reserve(extensionSlots.size());

        size_t cachedSz = 6 + 8 * extensionSlots.size();
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::string fragSource = m_backend.makeFrag(slot.lighting, slot.post);
            fragPipeBlobs.emplace_back();
            std::pair<ComPtr<ID3DBlob>, ComPtr<ID3DBlob>>& fragPipeBlob = fragPipeBlobs.back();
            boo::IShaderPipeline* ret =
            m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                            vertBlob, fragPipeBlob.first, fragPipeBlob.second,
                                            tag.newVertexFormat(m_gfxFactory),
                                            m_backend.m_blendSrc, m_backend.m_blendDst,
                                            tag.getDepthTest(), tag.getDepthWrite(),
                                            tag.getBackfaceCulling());
            if (!ret)
                Log.report(LogVisor::FatalError, "unable to build shader");
            if (fragPipeBlob.first)
                cachedSz += fragPipeBlob.first->GetBufferSize();
            if (fragPipeBlob.second)
                cachedSz += fragPipeBlob.second->GetBufferSize();
            returnFunc(ret);
        }
        if (vertBlob)
            cachedSz += vertBlob->GetBufferSize();

        ShaderCachedData dataOut(tag, cachedSz);
        Athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(m_backend.m_blendSrc);
        w.writeUByte(m_backend.m_blendDst);

        if (vertBlob)
        {
            w.writeUint32Big(vertBlob->GetBufferSize());
            w.writeUBytes((atUint8*)vertBlob->GetBufferPointer(), vertBlob->GetBufferSize());
        }
        else
            w.writeUint32Big(0);

        for (const std::pair<ComPtr<ID3DBlob>, ComPtr<ID3DBlob>>& fragPipeBlob : fragPipeBlobs)
        {
            if (fragPipeBlob.first)
            {
                w.writeUint32Big(fragPipeBlob.first->GetBufferSize());
                w.writeUBytes((atUint8*)fragPipeBlob.first->GetBufferPointer(), fragPipeBlob.first->GetBufferSize());
            }
            else
                w.writeUint32Big(0);

            if (fragPipeBlob.second)
            {
                w.writeUint32Big(fragPipeBlob.second->GetBufferSize());
                w.writeUBytes((atUint8*)fragPipeBlob.second->GetBufferPointer(), fragPipeBlob.second->GetBufferSize());
            }
            else
                w.writeUint32Big(0);
        }

        return dataOut;
    }

    void buildExtendedShaderFromCache(const ShaderCachedData& data,
                                      const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                      FReturnExtensionShader returnFunc)
    {
        const ShaderTag& tag = data.m_tag;
        Athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());

        atUint32 vertSz = r.readUint32Big();
        ComPtr<ID3DBlob> vertBlob;
        if (vertSz)
        {
            D3DCreateBlobPROC(vertSz, &vertBlob);
            r.readUBytesToBuf(vertBlob->GetBufferPointer(), vertSz);
        }

        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            atUint32 fragSz = r.readUint32Big();
            ComPtr<ID3DBlob> fragBlob;
            if (fragSz)
            {
                D3DCreateBlobPROC(fragSz, &fragBlob);
                r.readUBytesToBuf(fragBlob->GetBufferPointer(), fragSz);
            }

            atUint32 pipelineSz = r.readUint32Big();
            ComPtr<ID3DBlob> pipelineBlob;
            if (pipelineSz)
            {
                D3DCreateBlobPROC(pipelineSz, &pipelineBlob);
                r.readUBytesToBuf(pipelineBlob->GetBufferPointer(), pipelineSz);
            }

            boo::IShaderPipeline* ret =
            m_gfxFactory->newShaderPipeline(nullptr, nullptr,
                                            vertBlob, fragBlob, pipelineBlob,
                                            tag.newVertexFormat(m_gfxFactory),
                                            blendSrc, blendDst,
                                            tag.getDepthTest(), tag.getDepthWrite(),
                                            tag.getBackfaceCulling());
            if (!ret)
                Log.report(LogVisor::FatalError, "unable to build shader");
            returnFunc(ret);
        }
    }
};

IShaderBackendFactory* _NewHLSLBackendFactory(boo::IGraphicsDataFactory* gfxFactory)
{
    return new struct HLSLBackendFactory(gfxFactory);
}

}
}
