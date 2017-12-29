#include "hecl/HMDLMeta.hpp"
#include "hecl/Runtime.hpp"
#include <athena/MemoryReader.hpp>

namespace hecl::Runtime
{
static logvisor::Module HMDL_Log("HMDL");

HMDLData::HMDLData(boo::IGraphicsDataFactory::Context& ctx,
                   const void* metaData, const void* vbo, const void* ibo)
{
    HMDLMeta meta;
    {
        athena::io::MemoryReader r((atUint8*)metaData, HECL_HMDL_META_SZ);
        meta.read(r);
    }
    if (meta.magic != 'TACO')
        HMDL_Log.report(logvisor::Fatal, "invalid HMDL magic");

    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, vbo, meta.vertStride, meta.vertCount);
    m_ibo = ctx.newStaticBuffer(boo::BufferUse::Index, ibo, 4, meta.indexCount);

    if (ctx.bindingNeedsVertexFormat())
        m_vtxFmt = NewVertexFormat(ctx, meta, m_vbo.get(), m_ibo.get());
}

/* For binding constructors that require vertex format up front (GLSL) */
boo::ObjToken<boo::IVertexFormat>
HMDLData::NewVertexFormat(boo::IGraphicsDataFactory::Context& ctx, const HMDLMeta& meta,
                          const boo::ObjToken<boo::IGraphicsBuffer>& vbo,
                          const boo::ObjToken<boo::IGraphicsBuffer>& ibo)
{
    size_t elemCount = 2 + meta.colorCount + meta.uvCount + meta.weightCount;
    std::unique_ptr<boo::VertexElementDescriptor[]> vdescs(new boo::VertexElementDescriptor[elemCount]);
    for (size_t i=0 ; i<elemCount ; ++i)
    {
        vdescs[i].vertBuffer = vbo;
        vdescs[i].indexBuffer = ibo;
    }

    vdescs[0].semantic = boo::VertexSemantic::Position3;
    vdescs[1].semantic = boo::VertexSemantic::Normal3;
    size_t e = 2;

    for (size_t i=0 ; i<meta.colorCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemantic::ColorUNorm;
        vdescs[e].semanticIdx = i;
    }

    for (size_t i=0 ; i<meta.uvCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemantic::UV2;
        vdescs[e].semanticIdx = i;
    }

    for (size_t i=0 ; i<meta.weightCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemantic::Weight;
        vdescs[e].semanticIdx = i;
    }

    return ctx.newVertexFormat(elemCount, vdescs.get());
}

/* For shader constructors that require vertex format up-front (HLSL/Metal/Vulkan) */
boo::ObjToken<boo::IVertexFormat> ShaderTag::newVertexFormat(boo::IGraphicsDataFactory::Context& ctx) const
{
    size_t elemCount = 2 + m_colorCount + m_uvCount + m_weightCount;
    std::unique_ptr<boo::VertexElementDescriptor[]> vdescs(new boo::VertexElementDescriptor[elemCount]);
    for (size_t i=0 ; i<elemCount ; ++i)
    {
        vdescs[i].vertBuffer = nullptr;
        vdescs[i].indexBuffer = nullptr;
    }

    vdescs[0].semantic = boo::VertexSemantic::Position3;
    vdescs[1].semantic = boo::VertexSemantic::Normal3;
    size_t e = 2;

    for (size_t i=0 ; i<m_colorCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemantic::ColorUNorm;
        vdescs[e].semanticIdx = i;
    }

    for (size_t i=0 ; i<m_uvCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemantic::UV2;
        vdescs[e].semanticIdx = i;
    }

    for (size_t i=0 ; i<m_weightCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemantic::Weight;
        vdescs[e].semanticIdx = i;
    }

    return ctx.newVertexFormat(elemCount, vdescs.get());
}

}
