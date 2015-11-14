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

    size_t elemCount = 2 + meta.colorCount + meta.uvCount + meta.weightCount;
    std::unique_ptr<boo::VertexElementDescriptor[]> vdescs(new boo::VertexElementDescriptor[elemCount]);
    for (size_t i=0 ; i<elemCount ; ++i)
    {
        vdescs[i].vertBuffer = m_vbo;
        vdescs[i].indexBuffer = m_ibo;
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

    m_vtxFmt = factory->newVertexFormat(elemCount, vdescs.get());
}

}
}
