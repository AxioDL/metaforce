#include "hecl/Backend/HLSL.hpp"
#include "hecl/Runtime.hpp"
#include <athena/MemoryReader.hpp>
#include <athena/MemoryWriter.hpp>
#include <boo/graphicsdev/D3D.hpp>

static logvisor::Module Log("hecl::Backend::HLSL");

namespace hecl::Backend
{

std::string HLSL::EmitTexGenSource2(TexGenSrc src, int uvIdx) const
{
    switch (src)
    {
    case TexGenSrc::Position:
        return "vtf.mvPos.xy\n";
    case TexGenSrc::Normal:
        return "vtf.mvNorm.xy\n";
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
        return "float4(vtf.mvPos.xyz, 1.0)\n";
    case TexGenSrc::Normal:
        return "float4(vtf.mvNorm.xyz, 1.0)\n";
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

std::string HLSL::GenerateVertToFragStruct(size_t extTexCount, bool reflectionCoords) const
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

    if (reflectionCoords)
        retval += "    float2 reflectTcgs[2] : REFLECTUV;\n"
                  "    float reflectAlpha : REFLECTALPHA;\n";

    return retval + "};\n";
}

std::string HLSL::GenerateVertUniformStruct(unsigned skinSlots, bool reflectionCoords) const
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
    retval += "struct TCGMtx\n"
              "{\n"
              "    float4x4 mtx;\n"
              "    float4x4 postMtx;\n"
              "};\n"
              "cbuffer HECLTCGMatrix : register(b1)\n"
              "{\n"
              "    TCGMtx texMtxs[8];\n"
              "};\n";

    if (reflectionCoords)
        retval += "cbuffer HECLReflectMtx : register(b3)\n"
                  "{\n"
                  "    float4x4 indMtx;\n"
                  "    float4x4 reflectMtx;\n"
                  "    float reflectAlpha;\n"
                  "};\n"
                  "\n";

    return retval;
}

std::string HLSL::GenerateAlphaTest() const
{
    return "    if (colorOut.a < 0.01)\n"
           "    {\n"
           "        discard;\n"
           "    }\n";
}

std::string HLSL::GenerateReflectionExpr(ReflectionType type) const
{
    switch (type)
    {
    case ReflectionType::None:
    default:
        return "float3(0.0, 0.0, 0.0)";
    case ReflectionType::Simple:
        return "reflectionTex.Sample(samp, vtf.reflectTcgs[1]).rgb * vtf.reflectAlpha";
    case ReflectionType::Indirect:
        return "reflectionTex.Sample(samp, (reflectionIndTex.Sample(samp, vtf.reflectTcgs[0]).rg - "
               "float2(0.5, 0.5)) * float2(0.5, 0.5) + vtf.reflectTcgs[1]).rgb * vtf.reflectAlpha";
    }
}

void HLSL::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "HLSL");
}

std::string HLSL::makeVert(unsigned col, unsigned uv, unsigned w,
                           unsigned s, size_t extTexCount,
                           const TextureInfo* extTexs, ReflectionType reflectionType) const
{
    std::string retval =
            GenerateVertInStruct(col, uv, w) + "\n" +
            GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) + "\n" +
            GenerateVertUniformStruct(s, reflectionType != ReflectionType::None) + "\n" +
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

    retval += "    float4 tmpProj;\n";

    int tcgIdx = 0;
    for (const TexCoordGen& tcg : m_tcgs)
    {
        if (tcg.m_mtx < 0)
            retval += hecl::Format("    vtf.tcgs[%u] = %s;\n", tcgIdx,
                                   EmitTexGenSource2(tcg.m_src, tcg.m_uvIdx).c_str());
        else
            retval += hecl::Format("    tmpProj = mul(texMtxs[%u].postMtx, float4(%s(mul(texMtxs[%u].mtx, %s).xyz), 1.0));\n"
                                   "    vtf.tcgs[%u] = (tmpProj / tmpProj.w).xy;\n", tcg.m_mtx,
                                   tcg.m_norm ? "normalize" : "", tcg.m_mtx,
                                   EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str(), tcgIdx);
        ++tcgIdx;
    }

    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        if (extTex.mtxIdx < 0)
            retval += hecl::Format("    vtf.extTcgs[%u] = %s;\n", i,
                                   EmitTexGenSource2(extTex.src, extTex.uvIdx).c_str());
        else
            retval += hecl::Format("    tmpProj = mul(texMtxs[%u].postMtx, float4(%s(mul(texMtxs[%u].mtx, %s).xyz), 1.0));\n"
                                   "    vtf.extTcgs[%u] = (tmpProj / tmpProj.w).xy;\n",
                                   extTex.mtxIdx, extTex.normalize ? "normalize" : "", extTex.mtxIdx,
                                   EmitTexGenSource4(extTex.src, extTex.uvIdx).c_str(), i);
    }

    if (reflectionType != ReflectionType::None)
        retval += "    vtf.reflectTcgs[0] = normalize(mul(indMtx, float4(v.posIn, 1.0)).xz) * float2(0.5, 0.5) + float2(0.5, 0.5);\n"
                  "    vtf.reflectTcgs[1] = mul(reflectMtx, float4(v.posIn, 1.0)).xy;\n"
                  "    vtf.reflectAlpha = reflectAlpha;\n";

    return retval + "    return vtf;\n"
                    "}\n";
}

std::string HLSL::makeFrag(bool alphaTest, ReflectionType reflectionType,
                           const ShaderFunction& lighting) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;
    else
        lightingSrc = "static const float4 colorReg0 = float4(1.0, 1.0, 1.0, 1.0);\n"
                      "static const float4 colorReg1 = float4(1.0, 1.0, 1.0, 1.0);\n"
                      "static const float4 colorReg2 = float4(1.0, 1.0, 1.0, 1.0);\n"
                      "static const float4 mulColor = float4(1.0, 1.0, 1.0, 1.0);\n";

    std::string texMapDecl;
    if (m_texMapEnd)
        texMapDecl = hecl::Format("Texture2D texs[%u] : register(t0);\n", m_texMapEnd);
    if (reflectionType == ReflectionType::Indirect)
        texMapDecl += hecl::Format("Texture2D reflectionIndTex : register(t%u);\n"
                                   "Texture2D reflectionTex : register(t%u);\n",
                                   m_texMapEnd, m_texMapEnd+1);
    else if (reflectionType == ReflectionType::Simple)
        texMapDecl += hecl::Format("Texture2D reflectionTex : register(t%u);\n",
                                   m_texMapEnd);
    std::string retval =
            "SamplerState samp : register(s0);\n"
            "SamplerState clampSamp : register(s1);\n" +
            GenerateVertToFragStruct(0, reflectionType != ReflectionType::None) +
            texMapDecl + "\n" +
            lightingSrc + "\n" +
            (!alphaTest ? "\n[earlydepthstencil]\n" : "\n") +
            "float4 main(in VertToFrag vtf) : SV_Target0\n{\n";


    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    float4 lighting = %s(vtf.mvPos, vtf.mvNorm, vtf);\n", lighting.m_entry);
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    float4 sampling%u = texs[%u].Sample(samp, vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

    retval += "    float4 colorOut;\n";
    if (m_alphaExpr.size())
        retval += "    colorOut = float4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr + ") * mulColor;\n";
    else
        retval += "    colorOut = float4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0) * mulColor;\n";

    return retval + (alphaTest ? GenerateAlphaTest() : "") + "    return colorOut;\n}\n";
}

std::string HLSL::makeFrag(bool alphaTest, ReflectionType reflectionType,
                           const ShaderFunction& lighting,
                           const ShaderFunction& post, size_t extTexCount,
                           const TextureInfo* extTexs) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;
    else
        lightingSrc = "static const float4 colorReg0 = float4(1.0, 1.0, 1.0, 1.0);\n"
                      "static const float4 colorReg1 = float4(1.0, 1.0, 1.0, 1.0);\n"
                      "static const float4 colorReg2 = float4(1.0, 1.0, 1.0, 1.0);\n"
                      "static const float4 mulColor = float4(1.0, 1.0, 1.0, 1.0);\n";

    std::string postSrc;
    if (post.m_source)
        postSrc = post.m_source;

    std::string postEntry;
    if (post.m_entry)
        postEntry = post.m_entry;

    std::string texMapDecl;
    if (m_texMapEnd)
        texMapDecl = hecl::Format("Texture2D texs[%u] : register(t0);\n", m_texMapEnd);
    if (reflectionType == ReflectionType::Indirect)
        texMapDecl += hecl::Format("Texture2D reflectionIndTex : register(t%u);\n"
                                   "Texture2D reflectionTex : register(t%u);\n",
                                   m_texMapEnd, m_texMapEnd+1);
    else if (reflectionType == ReflectionType::Simple)
        texMapDecl += hecl::Format("Texture2D reflectionTex : register(t%u);\n",
                                   m_texMapEnd);

    for (int i=0 ; i<extTexCount ; ++i)
    {
        const TextureInfo& extTex = extTexs[i];
        texMapDecl += hecl::Format("Texture2D extTex%u : register(t%u);\n",
                                   extTex.mapIdx, extTex.mapIdx);
    }

    std::string retval =
            "SamplerState samp : register(s0);\n"
            "SamplerState clampSamp : register(s1);\n" +
            GenerateVertToFragStruct(extTexCount, reflectionType != ReflectionType::None) +
            texMapDecl + "\n" +
            lightingSrc + "\n" +
            postSrc +
            (!alphaTest ? "\n[earlydepthstencil]\n" : "\n") +
            "float4 main(in VertToFrag vtf) : SV_Target0\n{\n";


    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    float4 lighting = %s(vtf.mvPos, vtf.mvNorm, vtf);\n", lighting.m_entry);
        else
            retval += "    float4 lighting = float4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    float4 sampling%u = texs[%u].Sample(samp, vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    std::string reflectionExpr = GenerateReflectionExpr(reflectionType);

    retval += "    float4 colorOut;\n";
    if (m_alphaExpr.size())
        retval += "    colorOut = " + postEntry + "(" + (postEntry.size() ? "vtf, " : "") + "float4(" + m_colorExpr + " + " + reflectionExpr + ", " + m_alphaExpr + ")) * mulColor;\n";
    else
        retval += "    colorOut = " + postEntry + "(" + (postEntry.size() ? "vtf, " : "") + "float4(" + m_colorExpr + " + " + reflectionExpr + ", 1.0)) * mulColor;\n";

    return retval + (alphaTest ? GenerateAlphaTest() : "") + "    return colorOut;\n}\n";
}

}

namespace hecl::Runtime
{

struct HLSLBackendFactory : IShaderBackendFactory
{
    Backend::HLSL m_backend;

    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const hecl::Frontend::IR& ir,
                                       hecl::Frontend::Diagnostics& diag,
                                       boo::IGraphicsDataFactory::Context& ctx,
                                       boo::ObjToken<boo::IShaderPipeline>& objOut)
    {
        m_backend.reset(ir, diag);

        std::string vertSource =
        m_backend.makeVert(tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), 0, nullptr,
                           tag.getReflectionType());

        std::string fragSource = m_backend.makeFrag(tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
                                                    tag.getReflectionType());
        ComPtr<ID3DBlob> vertBlob;
        ComPtr<ID3DBlob> fragBlob;
        ComPtr<ID3DBlob> pipelineBlob;
        objOut =
        static_cast<boo::D3DDataFactory::Context&>(ctx).
            newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                              ReferenceComPtr(vertBlob), ReferenceComPtr(fragBlob), ReferenceComPtr(pipelineBlob),
                              tag.newVertexFormat(ctx),
                              boo::BlendFactor(m_backend.m_blendSrc),
                              boo::BlendFactor(m_backend.m_blendDst),
                              tag.getPrimType(),
                              tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None, tag.getDepthWrite(), true, false,
                              tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None);
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

    boo::ObjToken<boo::IShaderPipeline>
    buildShaderFromCache(const ShaderCachedData& data,
                         boo::IGraphicsDataFactory::Context& ctx)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz, false, false);
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());

        if (r.hasError())
            return nullptr;

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

        if (r.hasError())
            return nullptr;

        boo::ObjToken<boo::IShaderPipeline> ret =
        static_cast<boo::D3DDataFactory::Context&>(ctx).
            newShaderPipeline(nullptr, nullptr,
                              ReferenceComPtr(vertBlob), ReferenceComPtr(fragBlob), ReferenceComPtr(pipelineBlob),
                              tag.newVertexFormat(ctx),
                              blendSrc, blendDst, tag.getPrimType(),
                              tag.getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None, tag.getDepthWrite(), true, false,
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
                               tag.getSkinSlotCount(), slot.texCount, slot.texs,
                               tag.getReflectionType());

            std::string fragSource = m_backend.makeFrag(tag.getDepthWrite() && m_backend.m_blendDst == hecl::Backend::BlendFactor::InvSrcAlpha,
                                                        tag.getReflectionType(), slot.lighting, slot.post, slot.texCount, slot.texs);
            pipeBlobs.emplace_back();
            Blobs& thisPipeBlobs = pipeBlobs.back();

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
            case hecl::Backend::ZTest::GEqual:
                zTest = boo::ZTest::GEqual;
                break;
            }

            boo::ObjToken<boo::IShaderPipeline> ret =
            static_cast<boo::D3DDataFactory::Context&>(ctx).
                newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                  ReferenceComPtr(thisPipeBlobs.vert), ReferenceComPtr(thisPipeBlobs.frag), ReferenceComPtr(thisPipeBlobs.pipeline),
                                  tag.newVertexFormat(ctx),
                                  boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? m_backend.m_blendSrc : slot.srcFactor),
                                  boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? m_backend.m_blendDst : slot.dstFactor),
                                  tag.getPrimType(), zTest, slot.noDepthWrite ? false : tag.getDepthWrite(),
                                  !slot.noColorWrite, !slot.noAlphaWrite,
                                  (slot.cullMode == hecl::Backend::CullMode::Original) ?
                                  (tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None) :
                                  boo::CullMode(slot.cullMode), !slot.noAlphaOverwrite);
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

    bool buildExtendedShaderFromCache(const ShaderCachedData& data,
                                      const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                      boo::IGraphicsDataFactory::Context& ctx,
                                      FReturnExtensionShader returnFunc)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz, false, false);
        hecl::Backend::BlendFactor blendSrc = hecl::Backend::BlendFactor(r.readUByte());
        hecl::Backend::BlendFactor blendDst = hecl::Backend::BlendFactor(r.readUByte());

        if (r.hasError())
            return false;

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
            case hecl::Backend::ZTest::GEqual:
                zTest = boo::ZTest::GEqual;
                break;
            }

            boo::ObjToken<boo::IShaderPipeline> ret =
            static_cast<boo::D3DDataFactory::Context&>(ctx).
                newShaderPipeline(nullptr, nullptr,
                                  ReferenceComPtr(vertBlob), ReferenceComPtr(fragBlob), ReferenceComPtr(pipelineBlob),
                                  tag.newVertexFormat(ctx),
                                  boo::BlendFactor((slot.srcFactor == hecl::Backend::BlendFactor::Original) ? blendSrc : slot.srcFactor),
                                  boo::BlendFactor((slot.dstFactor == hecl::Backend::BlendFactor::Original) ? blendDst : slot.dstFactor),
                                  tag.getPrimType(), zTest, slot.noDepthWrite ? false : tag.getDepthWrite(),
                                  !slot.noColorWrite, !slot.noAlphaWrite,
                                  (slot.cullMode == hecl::Backend::CullMode::Original) ?
                                  (tag.getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None) :
                                  boo::CullMode(slot.cullMode), !slot.noAlphaOverwrite);
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }

        return true;
    }
};

IShaderBackendFactory* _NewHLSLBackendFactory()
{
    return new struct HLSLBackendFactory();
}

}
