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
        return "posIn.xy\n";
    case TexGenSrc::Normal:
        return "normIn.xy\n";
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
        return "vec4(posIn, 1.0)\n";
    case TexGenSrc::Normal:
        return "vec4(normIn, 1.0)\n";
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

std::string GLSL::GenerateVertToFragStruct() const
{
    std::string retval =
    "struct VertToFrag\n"
    "{\n"
    "    vec4 mvPos;\n"
    "    vec4 mvNorm;\n";

    if (m_tcgs.size())
        retval += hecl::Format("    vec2 tcgs[%u];\n", unsigned(m_tcgs.size()));

    return retval + "};\n";
}

std::string GLSL::GenerateVertUniformStruct(unsigned skinSlots, unsigned texMtxs) const
{
    if (skinSlots == 0)
        skinSlots = 1;
    std::string retval = hecl::Format("UBINDING0 uniform HECLVertUniform\n"
                                      "{\n"
                                      "    mat4 mv[%u];\n"
                                      "    mat4 mvInv[%u];\n"
                                      "    mat4 proj;\n",
                                      skinSlots, skinSlots);
    if (texMtxs)
        retval += hecl::Format("    mat4 texMtxs[%u];\n", texMtxs);
    return retval + "};\n";
}

void GLSL::reset(const IR& ir, Diagnostics& diag)
{
    /* Common programmable interpretation */
    ProgrammableCommon::reset(ir, diag, "GLSL");
}

std::string GLSL::makeVert(const char* glslVer, unsigned col, unsigned uv, unsigned w,
                           unsigned s, unsigned tm) const
{
    std::string retval = std::string(glslVer) + "\n" BOO_GLSL_BINDING_HEAD +
            GenerateVertInStruct(col, uv, w) + "\n" +
            GenerateVertToFragStruct() + "\n" +
            GenerateVertUniformStruct(s, tm) +
            "out VertToFrag vtf;\n\n"
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
            retval += hecl::Format("    vtf.tcgs[%u] = (texMtxs[%u] * %s).xy;\n", tcgIdx, tcg.m_mtx,
                                   EmitTexGenSource4(tcg.m_src, tcg.m_uvIdx).c_str());
        ++tcgIdx;
    }

    return retval + "}\n";
}

std::string GLSL::makeFrag(const char* glslVer,
                           const ShaderFunction& lighting) const
{
    std::string lightingSrc;
    if (lighting.m_source)
        lightingSrc = lighting.m_source;

    std::string texMapDecl;
    if (m_texMapEnd)
        texMapDecl = hecl::Format("TBINDING0 uniform sampler2D texs[%u];\n", m_texMapEnd);

    std::string retval = std::string(glslVer) + "\n" BOO_GLSL_BINDING_HEAD +
            GenerateVertToFragStruct() +
            "\nlayout(location=0) out vec4 colorOut;\n" +
            texMapDecl +
            "in VertToFrag vtf;\n\n" +
            lightingSrc + "\n" +
            "void main()\n{\n";


    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    vec4 lighting = %s();\n", lighting.m_entry);
        else
            retval += "    vec4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    vec4 sampling%u = texture(texs[%u], vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    if (m_alphaExpr.size())
        retval += "    colorOut = vec4(" + m_colorExpr + ", " + m_alphaExpr + ");\n";
    else
        retval += "    colorOut = vec4(" + m_colorExpr + ", 1.0);\n";

    return retval + "}\n";
}

std::string GLSL::makeFrag(const char* glslVer,
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
        texMapDecl = hecl::Format("TBINDING0 uniform sampler2D texs[%u];\n", m_texMapEnd);

    std::string retval = std::string(glslVer) + "\n" BOO_GLSL_BINDING_HEAD +
            GenerateVertToFragStruct() +
            "\nlayout(location=0) out vec4 colorOut;\n" +
            texMapDecl +
            "in VertToFrag vtf;\n\n" +
            lightingSrc + "\n" +
            postSrc +
            "\nvoid main()\n{\n";

    if (m_lighting)
    {
        if (lighting.m_entry)
            retval += hecl::Format("    vec4 lighting = %s();\n", lighting.m_entry);
        else
            retval += "    vec4 lighting = vec4(1.0,1.0,1.0,1.0);\n";
    }

    unsigned sampIdx = 0;
    for (const TexSampling& sampling : m_texSamplings)
        retval += hecl::Format("    vec4 sampling%u = texture(texs[%u], vtf.tcgs[%u]);\n",
                               sampIdx++, sampling.mapIdx, sampling.tcgIdx);

    if (m_alphaExpr.size())
        retval += "    colorOut = " + postEntry + "(vec4(" + m_colorExpr + ", " + m_alphaExpr + "));\n";
    else
        retval += "    colorOut = " + postEntry + "(vec4(" + m_colorExpr + ", 1.0));\n";

    return retval + "}\n";
}

}
namespace Runtime
{

static const char* STD_BLOCKNAMES[] = {"HECLVertUniform"};

struct GLSLBackendFactory : IShaderBackendFactory
{
    Backend::GLSL m_backend;
    boo::GLDataFactory* m_gfxFactory;

    GLSLBackendFactory(boo::IGraphicsDataFactory* gfxFactory)
    : m_gfxFactory(dynamic_cast<boo::GLDataFactory*>(gfxFactory)) {}

    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const hecl::Frontend::IR& ir,
                                       hecl::Frontend::Diagnostics& diag,
                                       boo::IShaderPipeline*& objOut)
    {
        m_backend.reset(ir, diag);
        size_t cachedSz = 3;

        std::string vertSource =
        m_backend.makeVert("#version 330",
                           tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());
        cachedSz += vertSource.size() + 1;

        std::string fragSource = m_backend.makeFrag("#version 330");
        cachedSz += fragSource.size() + 1;
        objOut =
        m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                        m_backend.m_texMapEnd, "texs",
                                        1, STD_BLOCKNAMES,
                                        m_backend.m_blendSrc, m_backend.m_blendDst, boo::Primitive::TriStrips,
                                        tag.getDepthTest(), tag.getDepthWrite(),
                                        tag.getBackfaceCulling());
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

    boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        atUint8 texMapEnd = r.readUByte();
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());
        std::string vertSource = r.readString();
        std::string fragSource = r.readString();
        boo::IShaderPipeline* ret =
        m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                        texMapEnd, "texs",
                                        1, STD_BLOCKNAMES,
                                        blendSrc, blendDst, boo::Primitive::TriStrips,
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
                                               FReturnExtensionShader returnFunc)
    {
        m_backend.reset(ir, diag);
        size_t cachedSz = 3;

        std::string vertSource =
        m_backend.makeVert("#version 330",
                           tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());
        cachedSz += vertSource.size() + 1;

        std::vector<std::string> fragSources;
        fragSources.reserve(extensionSlots.size());
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            fragSources.push_back(m_backend.makeFrag("#version 330", slot.lighting, slot.post));
            cachedSz += fragSources.back().size() + 1;
            boo::IShaderPipeline* ret =
            m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSources.back().c_str(),
                                            m_backend.m_texMapEnd, "texs",
                                            1, STD_BLOCKNAMES,
                                            m_backend.m_blendSrc, m_backend.m_blendDst, boo::Primitive::TriStrips,
                                            tag.getDepthTest(), tag.getDepthWrite(),
                                            tag.getBackfaceCulling());
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(m_backend.m_texMapEnd);
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
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        atUint8 texMapEnd = r.readUByte();
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());
        std::string vertSource = r.readString();
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::string fragSource = r.readString();
            boo::IShaderPipeline* ret =
            m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                            texMapEnd, "texs",
                                            1, STD_BLOCKNAMES,
                                            blendSrc, blendDst, boo::Primitive::TriStrips,
                                            tag.getDepthTest(), tag.getDepthWrite(),
                                            tag.getBackfaceCulling());
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }
    }
};

IShaderBackendFactory* _NewGLSLBackendFactory(boo::IGraphicsDataFactory* gfxFactory)
{
    return new struct GLSLBackendFactory(gfxFactory);
}

#if BOO_HAS_VULKAN

struct SPIRVBackendFactory : IShaderBackendFactory
{
    Backend::GLSL m_backend;
    boo::VulkanDataFactory* m_gfxFactory;

    SPIRVBackendFactory(boo::IGraphicsDataFactory* gfxFactory)
    : m_gfxFactory(dynamic_cast<boo::VulkanDataFactory*>(gfxFactory)) {}

    ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                       const hecl::Frontend::IR& ir,
                                       hecl::Frontend::Diagnostics& diag,
                                       boo::IShaderPipeline*& objOut)
    {
        m_backend.reset(ir, diag);

        std::string vertSource =
        m_backend.makeVert("#version 330",
                           tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());

        std::string fragSource = m_backend.makeFrag("#version 330");

        std::vector<unsigned int> vertBlob;
        std::vector<unsigned int> fragBlob;
        std::vector<unsigned char> pipelineBlob;

        objOut =
        m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                        vertBlob, fragBlob, pipelineBlob, tag.newVertexFormat(m_gfxFactory),
                                        m_backend.m_blendSrc, m_backend.m_blendDst, boo::Primitive::TriStrips,
                                        tag.getDepthTest(), tag.getDepthWrite(),
                                        tag.getBackfaceCulling());
        if (!objOut)
            Log.report(logvisor::Fatal, "unable to build shader");


        atUint32 vertSz = vertBlob.size() * sizeof(unsigned int);
        atUint32 fragSz = fragBlob.size() * sizeof(unsigned int);
        atUint32 pipelineSz = pipelineBlob.size() * sizeof(unsigned int);

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

    boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data)
    {
        const ShaderTag& tag = data.m_tag;
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
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

        boo::IShaderPipeline* ret =
        m_gfxFactory->newShaderPipeline(nullptr, nullptr,
                                        vertBlob, fragBlob, pipelineBlob,
                                        tag.newVertexFormat(m_gfxFactory),
                                        blendSrc, blendDst, boo::Primitive::TriStrips,
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
                                               FReturnExtensionShader returnFunc)
    {
        m_backend.reset(ir, diag);

        std::string vertSource =
        m_backend.makeVert("#version 330",
                           tag.getColorCount(), tag.getUvCount(), tag.getWeightCount(),
                           tag.getSkinSlotCount(), tag.getTexMtxCount());

        std::vector<unsigned int> vertBlob;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned char>>> fragPipeBlobs;
        fragPipeBlobs.reserve(extensionSlots.size());

        size_t cachedSz = 7 + 8 * extensionSlots.size();
        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            std::string fragSource = m_backend.makeFrag("#version 330", slot.lighting, slot.post);
            fragPipeBlobs.emplace_back();
            std::pair<std::vector<unsigned int>, std::vector<unsigned char>>& fragPipeBlob = fragPipeBlobs.back();
            boo::IShaderPipeline* ret =
            m_gfxFactory->newShaderPipeline(vertSource.c_str(), fragSource.c_str(),
                                            vertBlob, fragPipeBlob.first, fragPipeBlob.second,
                                            tag.newVertexFormat(m_gfxFactory),
                                            m_backend.m_blendSrc, m_backend.m_blendDst, boo::Primitive::TriStrips,
                                            tag.getDepthTest(), tag.getDepthWrite(),
                                            tag.getBackfaceCulling());
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            cachedSz += fragPipeBlob.first.size() * sizeof(unsigned int);
            cachedSz += fragPipeBlob.second.size();
            returnFunc(ret);
        }
        size_t vertBlobSz = vertBlob.size() * sizeof(unsigned int);
        cachedSz += vertBlobSz;

        ShaderCachedData dataOut(tag, cachedSz);
        athena::io::MemoryWriter w(dataOut.m_data.get(), dataOut.m_sz);
        w.writeUByte(atUint8(m_backend.m_texMapEnd));
        w.writeUByte(atUint8(m_backend.m_blendSrc));
        w.writeUByte(atUint8(m_backend.m_blendDst));

        if (vertBlobSz)
        {

            w.writeUint32Big(vertBlobSz);
            w.writeUBytes((atUint8*)vertBlob.data(), vertBlobSz);
        }
        else
            w.writeUint32Big(0);

        for (const std::pair<std::vector<unsigned int>, std::vector<unsigned char>>& fragPipeBlob : fragPipeBlobs)
        {
            size_t fragBlobSz = fragPipeBlob.first.size() * sizeof(unsigned int);
            size_t pipeBlobSz = fragPipeBlob.second.size();
            if (fragBlobSz)
            {
                w.writeUint32Big(fragBlobSz);
                w.writeUBytes((atUint8*)fragPipeBlob.first.data(), fragBlobSz);
            }
            else
                w.writeUint32Big(0);

            if (pipeBlobSz)
            {
                w.writeUint32Big(pipeBlobSz);
                w.writeUBytes((atUint8*)fragPipeBlob.second.data(), pipeBlobSz);
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
        athena::io::MemoryReader r(data.m_data.get(), data.m_sz);
        size_t texCount = size_t(r.readByte());
        boo::BlendFactor blendSrc = boo::BlendFactor(r.readUByte());
        boo::BlendFactor blendDst = boo::BlendFactor(r.readUByte());

        atUint32 vertSz = r.readUint32Big();
        std::vector<unsigned int> vertBlob(vertSz / sizeof(unsigned int));
        if (vertSz)
            r.readUBytesToBuf(vertBlob.data(), vertSz);

        for (const ShaderCacheExtensions::ExtensionSlot& slot : extensionSlots)
        {
            atUint32 fragSz = r.readUint32Big();
            std::vector<unsigned int> fragBlob(fragSz / sizeof(unsigned int));
            if (fragSz)
                r.readUBytesToBuf(fragBlob.data(), fragSz);

            atUint32 pipelineSz = r.readUint32Big();
            std::vector<unsigned char> pipelineBlob(pipelineSz);
            if (pipelineSz)
                r.readUBytesToBuf(pipelineBlob.data(), pipelineSz);

            boo::IShaderPipeline* ret =
            m_gfxFactory->newShaderPipeline(nullptr, nullptr,
                                            vertBlob, fragBlob, pipelineBlob,
                                            tag.newVertexFormat(m_gfxFactory),
                                            blendSrc, blendDst, boo::Primitive::TriStrips,
                                            tag.getDepthTest(), tag.getDepthWrite(),
                                            tag.getBackfaceCulling());
            if (!ret)
                Log.report(logvisor::Fatal, "unable to build shader");
            returnFunc(ret);
        }
    }
};

IShaderBackendFactory* _NewSPIRVBackendFactory(boo::IGraphicsDataFactory* gfxFactory)
{
    return new struct SPIRVBackendFactory(gfxFactory);
}

#endif

}
}
