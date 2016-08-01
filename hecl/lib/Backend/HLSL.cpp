#include "hecl/Backend/HLSL.hpp"
#include "hecl/Runtime.hpp"
#include <athena/MemoryReader.hpp>
#include <athena/MemoryWriter.hpp>
#include <boo/graphicsdev/D3D.hpp>

static logvisor::Module Log("hecl::Backend::HLSL");

namespace hecl
{
namespace Backend
{

std::string HLSL::EmitTexGenSource2(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TexGenSrc::Position:
        return "v.posIn.xy\n";
    case TexGenSrc::Normal:
        return "v.normIn.xy\n";
    case TexGenSrc::UV:
        return hecl::Format("v.uvIn[%u]", uvIdx);
    default: break;
    }
    return std::string();
}

std::string HLSL::EmitTexGenSource4(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TexGenSrc::Position:
        return "float4(v.posIn, 1.0)\n";
    case TexGenSrc::Normal:
        return "float4(v.normIn, 1.0)\n";
    case TexGenSrc::UV:
        return hecl::Format("float4(v.uvIn[%u], 0.0, 1.0)", uvIdx);
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
        retval += hecl::Format("    float4 colIn[%u] : COLOR;\n", col);

    if (uv)
        retval += hecl::Format("    float2 uvIn[%u] : UV;\n", uv);

    if (w)
        retval += hecl::Format("    float4 weightIn[%u] : WEIGHT;\n", w);

    return retval + "};\n";
}

std::string HLSL::GenerateVertToFragStruct(size_t extTexCount) const
{
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

    return retval + "};\n";
}

std::string HLSL::GenerateVertUniformStruct(unsigned skinSlots, unsigned texMtxs) const
{
    if (skinSlots == 0)
        skinSlots = 1;
    std::string retval = hecl::Format("cbuffer HECLVertUniform : register(b0)\n"
                                      "{\n"
                                      "    float4x4 mv[%u];\n"
                                      "    float4x4 mvInv[%u];\n"
                                      "    float4x4 proj;\n"
                                      "};\n",
                                      skinSlots, skinSlots);
    if (texMtxs)
    {
        retval += hecl::Format("struct TCGMtx\n"
                               "{\n"
                               "    float4x4 mtx;\n"
                               "    float4x4 postMtx;\n"
                               "};\n"
                               "cbuffer HECLTCGMatrix : register(b1)\n"
                               "{\n"
                               "    TCGMtx texMtxs[%u];\n"
                               "};\n", texMtxs);
    }
    return retval;
}

std::string HLSL::GenerateAlphaTest() const
{
    return "    if (colorOut.a < 0.01)\n"
           "    {\n"
           "        discard;\n"
           "    }\n";
}

void HLSL::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "HLSL");
}

std::string HLSL::makeVert(unsigned col, unsigned uv, unsigned w,
                           unsigned s, unsigned tm, size_t extTexCount,
                           const TextureInfo* extTexs) const
{
    std::string retval =
            GenerateVertInStruct(col, uv, w) + "\n" +
            GenerateVertToFragStruct(extTexCount) + "\n" +
            GenerateVertUniformStruct(s, tm) + "\n" +
            "VertToFrag main(in VertData v)\n"
            "{\n"
            "    VertToFrag vtf;\n";

    if (s)
    {
        /* skinned */
        retval += "    float4 posAccum = float4(0.0,0.0,0.0,0.0);\n"
                  "    float4 normAccum = float4(0.0,0.0,0.0,0.0);\n";
        for (size_t i=0 ; i<s ; ++i)
            retval += hecl::Format("    posAccum += mul(mv[%" PRISize "], float4(v.posIn, 1.0)) * v.weightIn[%" PRISize "][%" PRISize "];\n"
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
            retval += hecl::Format("    vtf.tcgs[%u] = %s;\n", tcgIdx,
                                   EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
        else
            retval += hecl::Format("    vtf.tcgs[%u] = mul(texMtxs[%u].postMtx, float4(%s(mul(texMtxs[%u].mtx, %s).xyz), 1.0)).xy;\n", tcgIdx, tcg.m_mtx,
                                   tcg.m_norm ? "normalize" : "", tcg.m_mtx, EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str());
        ++tcgIdx;
    }

    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        if (extTex.mtxIdx < 0)
            retval += hecl::Format("    vtf.extTcgs[%u] = %s;\n", i,
                                   EmitTexGenSource2(extTex.src, extTex.uvIdx).c_str());
        else
            retval += hecl::Format("    vtf.extTcgs[%u] = mul(texMtxs[%u].postMtx, float4(%s(mul(texMtxs[%u].mtx, %s).xyz), 1.0)).xy;\n",
                                   i, extTex.mtxIdx, extTex.normalize ? "normalize" : "",
                                   extTex.mtxIdx, EmitTexGenSource4(extTex.src, extTex.uvIdx).c_str());
    }

    return retval + "    return vtf;\n"
                    "}\n";
}

std::string HLSL::makeFrag(bool alphaTest, const ShaderFunction& lighting) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;
    else
        lightingSrc = "cbuffer LightingUniform : register(b2)\n"
                      "{\n"
                      "    float4 colorReg0;\n"
                      "    float4 colorReg1;\n"
                      "    float4 colorReg2;\n"
                      "};\n";

    std::string texMapDecl;
    if (m_texMapEnd)
        texMapDecl = hecl::Format("Texture2D texs[%u] : register(t0);\n", m_texMapEnd);

    std::string retval =
            "SamplerState samp : register(s0);\n" +
            GenerateVertToFragStruct(0) +
            texMapDecl + "\n" +
            lightingSrc + "\n" +
            (!alphaTest ? "\n[earlydepthstencil]\n" : "\n") +
            "float4 main(in VertToFrag vtf) : SV_Target0\n{\n";


    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    float4 lighting = %s(vtf.mvPos, vtf.mvNorm);\n", lighting.m_entry);
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    float4 sampling%u = texs[%u].Sample(samp, vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    retval += "    float4 colorOut;\n";
    if (m_alphaExpr.size())
        retval += "    colorOut = float4(" + m_colorExpr + ", " + m_alphaExpr + ");\n";
    else
        retval += "    colorOut = float4(" + m_colorExpr + ", 1.0);\n";

    return retval + (alphaTest ? GenerateAlphaTest() : "") + "    return colorOut;\n}\n";
}

std::string HLSL::makeFrag(bool alphaTest, const ShaderFunction& lighting,
                           const ShaderFunction& post, size_t extTexCount,
                           const TextureInfo* extTexs) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;
    else
        lightingSrc = "cbuffer LightingUniform : register(b2)\n"
                      "{\n"
                      "    float4 colorReg0;\n"
                      "    float4 colorReg1;\n"
                      "    float4 colorReg2;\n"
                      "};\n";

    std::string postSrc;
    if (post.m_source)
        postSrc = post.m_source;

    std::string postEntry;
    if (post.m_entry)
        postEntry = post.m_entry;

    std::string texMapDecl;
    if (m_texMapEnd)
        texMapDecl = hecl::Format("Texture2D texs[%u] : register(t0);\n", m_texMapEnd);

    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        texMapDecl += hecl::Format("Texture2D extTex%u : register(t%u);\n",
                                   extTex.mapIdx, extTex.mapIdx);
    }

    std::string retval =
            "SamplerState samp : register(s0);\n" +
            GenerateVertToFragStruct(extTexCount) +
            texMapDecl + "\n" +
            lightingSrc + "\n" +
            postSrc +
            (!alphaTest ? "\n[earlydepthstencil]\n" : "\n") +
            "float4 main(in VertToFrag vtf) : SV_Target0\n{\n";


    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    float4 lighting = %s(vtf.mvPos, vtf.mvNorm);\n", lighting.m_entry);
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    float4 sampling%u = texs[%u].Sample(samp, vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    retval += "    float4 colorOut;\n";
    if (m_alphaExpr.size())
        retval += "    colorOut = " + postEntry + "(" + (postEntry.size() ? "vtf, " : "") + "float4(" + m_colorExpr + ", " + m_alphaExpr + "));\n";
    else
        retval += "    colorOut = " + postEntry + "(" + (postEntry.size() ? "vtf, " : "") + "float4(" + m_colorExpr + ", 1.0));\n";

    return retval + (alphaTest ? GenerateAlphaTest() : "") + "    return colorOut;\n}\n";
}

}
namespace Runtime
{

struct HLSLBackendFactory : IShaderBackendFactory
{
    Backend::HLSL m_backend;

    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const hecl::Frontend::IR& ir,
                                       hecl::Frontend::Diagnostics& diag,
                                       boo::IGraphicsDataFactory::Context& ctx,
                                       boo::IShaderPipeline*& objOut)
    {
        m_backend.reset(ir, diag);

        std::string vertSource =
        m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount(), 0, nullptr);

        std::string fragSource = m_backend.makeFrag(tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha);
        ComPtr<ID3DBlob> vertBlob;
        ComPtr<ID3DBlob> fragBlob;
        ComPtr<ID3DBlob> pipelineBlob;
        objOut =
        static_cast<boo::ID3DDataFactory::Context&>(ctx).
            newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                              vertBlob, fragBlob, pipelineBlob,
                              tag.newVertexFormat(ctx),
                              boo::BlendFactor(m_backend.m_blendSrc),
                              boo::BlendFactor(m_backend.m_blendDst),
                              tag.getPrimType(),
                              tag.getDepthTest(), tag.getDepthWrite(),
                              tag.getBackfaceCulling());
        if (!objOut)
            Log.report(logvisor::Fatal, "unable to build shader");

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
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));

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

    boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data,
                                               boo::IGraphicsDataFactory::Context& ctx)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
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
        static_cast<boo::ID3DDataFactory::Context&>(ctx).
            newShaderPipeline(nullptr, nullptr,
                              vertBlob, fragBlob, pipelineBlob,
                              tag.newVertexFormat(ctx),
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
        m_backend.reset(ir, diag);

        struct Blobs
        {
            ComPtr<ID3DBlob> vert;
            ComPtr<ID3DBlob> frag;
            ComPtr<ID3DBlob> pipeline;
        };
        std::vector<Blobs> pipeBlobs;
        pipeBlobs.reserve(extensionSlots.size());

        size_t cachedSz = 2 + 12 * extensionSlots.size();
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::string vertSource =
            m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                               tag.getSkinSlotCount(), tag.getTexMtxCount(), slot.texCount, slot.texs);

            std::string fragSource = m_backend.makeFrag(tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
                                                        slot.lighting, slot.post, slot.texCount, slot.texs);
            pipeBlobs.emplace_back();
            Blobs& thisPipeBlobs = pipeBlobs.back();
            boo::IShaderPipeline* ret =
            static_cast<boo::ID3DDataFactory::Context&>(ctx).
                newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                  thisPipeBlobs.vert, thisPipeBlobs.frag, thisPipeBlobs.pipeline,
                                  tag.newVertexFormat(ctx),
                                  boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? m_backend.m_blendSrc : slot.srcFactor),
                                  boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? m_backend.m_blendDst : slot.dstFactor),
                                  tag.getPrimType(),
                                  tag.getDepthTest(), tag.getDepthWrite(),
                                  tag.getBackfaceCulling());
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            if (thisPipeBlobs.vert)
                cachedSz += thisPipeBlobs.vert->GetBufferSize();
            if (thisPipeBlobs.frag)
                cachedSz += thisPipeBlobs.frag->GetBufferSize();
            if (thisPipeBlobs.pipeline)
                cachedSz += thisPipeBlobs.pipeline->GetBufferSize();
            returnFunc(ret);
        }

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));

        for (const Blobs& blobs : pipeBlobs)
        {
            if (blobs.vert)
            {
                w.writeUint32Big(blobs.vert->GetBufferSize());
                w.writeUBytes((atUint8*)blobs.vert->GetBufferPointer(), blobs.vert->GetBufferSize());
            }
            else
                w.writeUint32Big(0);

            if (blobs.frag)
            {
                w.writeUint32Big(blobs.frag->GetBufferSize());
                w.writeUBytes((atUint8*)blobs.frag->GetBufferPointer(), blobs.frag->GetBufferSize());
            }
            else
                w.writeUint32Big(0);

            if (blobs.pipeline)
            {
                w.writeUint32Big(blobs.pipeline->GetBufferSize());
                w.writeUBytes((atUint8*)blobs.pipeline->GetBufferPointer(), blobs.pipeline->GetBufferSize());
            }
            else
                w.writeUint32Big(0);
        }

        return dataOut;
    }

    void buildExtendedShaderFromCache(const ShaderCachedData& data,
                                      const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                      boo::IGraphicsDataFactory::Context& ctx,
                                      FReturnExtensionShader returnFunc)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        hecl::Backend::BlendFactor blendSrc = hecl::Backend::BlendFactor(r.readUByte());
        hecl::Backend::BlendFactor blendDst = hecl::Backend::BlendFactor(r.readUByte());

        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
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
            static_cast<boo::ID3DDataFactory::Context&>(ctx).
                newShaderPipeline(nullptr, nullptr,
                                  vertBlob, fragBlob, pipelineBlob,
                                  tag.newVertexFormat(ctx),
                                  boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? blendSrc : slot.srcFactor),
                                  boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? blendDst : slot.dstFactor),
                                  tag.getPrimType(),
                                  tag.getDepthTest(), tag.getDepthWrite(),
                                  tag.getBackfaceCulling());
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }
    }
};

IShaderBackendFactory* _NewHLSLBackendFactory()
{
    return new struct HLSLBackendFactory();
}

}
}
