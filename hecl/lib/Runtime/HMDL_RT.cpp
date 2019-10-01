#include "hecl/HMDLMeta.hpp"

#include "hecl/Runtime.hpp"

#include <athena/MemoryReader.hpp>
#include <logvisor/logvisor.hpp>

namespace hecl::Runtime {
static logvisor::Module HMDL_Log("HMDL");

HMDLData::HMDLData(boo::IGraphicsDataFactory::Context& ctx, const void* metaData, const void* vbo, const void* ibo) {
  HMDLMeta meta;
  {
    athena::io::MemoryReader r((atUint8*)metaData, HECL_HMDL_META_SZ);
    meta.read(r);
  }
  if (meta.magic != 'TACO')
    HMDL_Log.report(logvisor::Fatal, fmt("invalid HMDL magic"));

  m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, vbo, meta.vertStride, meta.vertCount);
  m_ibo = ctx.newStaticBuffer(boo::BufferUse::Index, ibo, 4, meta.indexCount);

  size_t elemCount = 2 + meta.colorCount + meta.uvCount + meta.weightCount;
  m_vtxFmtData.reset(new boo::VertexElementDescriptor[elemCount]);

  m_vtxFmtData[0].semantic = boo::VertexSemantic::Position3;
  m_vtxFmtData[1].semantic = boo::VertexSemantic::Normal3;
  size_t e = 2;

  for (size_t i = 0; i < meta.colorCount; ++i, ++e) {
    m_vtxFmtData[e].semantic = boo::VertexSemantic::ColorUNorm;
    m_vtxFmtData[e].semanticIdx = i;
  }

  for (size_t i = 0; i < meta.uvCount; ++i, ++e) {
    m_vtxFmtData[e].semantic = boo::VertexSemantic::UV2;
    m_vtxFmtData[e].semanticIdx = i;
  }

  for (size_t i = 0; i < meta.weightCount; ++i, ++e) {
    m_vtxFmtData[e].semantic = boo::VertexSemantic::Weight;
    m_vtxFmtData[e].semanticIdx = i;
  }

  m_vtxFmt = boo::VertexFormatInfo(elemCount, m_vtxFmtData.get());
}

} // namespace hecl::Runtime
