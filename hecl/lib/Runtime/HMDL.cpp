#include "HECL/HMDLMeta.hpp"
#include "HECL/Runtime.hpp"
#include <Athena/MemoryReader.hpp>

namespace HECL
{
namespace Runtime
{
static LogVisor::LogModule Log("HMDL");

HMDLData::HMDLData(boo::IGraphicsDataFactory* factory,
                   const void* metaData, const void* vbo, const void* ibo)
{
    HMDLMeta meta;
    {
        Athena::io::MemoryReader r((atUint8*)metaData, HECL_HMDL_META_SZ);
        meta.read(r);
    }
    if (meta.magic != 'TACO')
        Log.report(LogVisor::FatalError, "invalid HMDL magic");

    m_vbo = factory->newStaticBuffer(boo::BufferUseVertex, vbo, meta.vertStride, meta.vertCount);
    m_ibo = factory->newStaticBuffer(boo::BufferUseIndex, ibo, 4, meta.indexCount);

    if (factory->bindingNeedsVertexFormat())
        m_vtxFmt = NewVertexFormat(factory, meta, m_vbo, m_ibo);
}

/* For binding constructors that require vertex format up front (GLSL) */
boo::IVertexFormat* HMDLData::NewVertexFormat(boo::IGraphicsDataFactory* factory, const HMDLMeta& meta,
                                              boo::IGraphicsBuffer* vbo, boo::IGraphicsBuffer* ibo)
{
    size_t elemCount = 2 + meta.colorCount + meta.uvCount + meta.weightCount;
    std::unique_ptr<boo::VertexElementDescriptor[]> vdescs(new boo::VertexElementDescriptor[elemCount]);
    for (size_t i=0 ; i<elemCount ; ++i)
    {
        vdescs[i].vertBuffer = vbo;
        vdescs[i].indexBuffer = ibo;
    }

    vdescs[0].semantic = boo::VertexSemanticPosition;
    vdescs[1].semantic = boo::VertexSemanticNormal;
    size_t e = 2;

    for (size_t i=0 ; i<meta.colorCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemanticColor;
        vdescs[e].semanticIdx = i;
    }

    for (size_t i=0 ; i<meta.uvCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemanticUV;
        vdescs[e].semanticIdx = i;
    }

    for (size_t i=0 ; i<meta.weightCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemanticWeight;
        vdescs[e].semanticIdx = i;
    }

    return factory->newVertexFormat(elemCount, vdescs.get());
}

/* For shader constructors that require vertex format up-front (HLSL) */
boo::IVertexFormat* ShaderTag::newVertexFormat(boo::IGraphicsDataFactory *factory) const
{
    size_t elemCount = 2 + m_colorCount + m_uvCount + m_weightCount;
    std::unique_ptr<boo::VertexElementDescriptor[]> vdescs(new boo::VertexElementDescriptor[elemCount]);
    for (size_t i=0 ; i<elemCount ; ++i)
    {
        vdescs[i].vertBuffer = nullptr;
        vdescs[i].indexBuffer = nullptr;
    }

    vdescs[0].semantic = boo::VertexSemanticPosition;
    vdescs[1].semantic = boo::VertexSemanticNormal;
    size_t e = 2;

    for (size_t i=0 ; i<m_colorCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemanticColor;
        vdescs[e].semanticIdx = i;
    }

    for (size_t i=0 ; i<m_uvCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemanticUV;
        vdescs[e].semanticIdx = i;
    }

    for (size_t i=0 ; i<m_weightCount ; ++i, ++e)
    {
        vdescs[e].semantic = boo::VertexSemanticWeight;
        vdescs[e].semanticIdx = i;
    }

    return factory->newVertexFormat(elemCount, vdescs.get());
}

}
}
