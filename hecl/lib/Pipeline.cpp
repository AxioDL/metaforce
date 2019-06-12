#include "hecl/Pipeline.hpp"
#include "athena/FileReader.hpp"
#include <zlib.h>

namespace hecl {

#if HECL_RUNTIME

PipelineConverterBase* conv = nullptr;

class ShaderCacheZipStream : public athena::io::IStreamReader {
  std::unique_ptr<uint8_t[]> m_compBuf;
  athena::io::FileReader m_reader;
  z_stream m_zstrm = {};

public:
  explicit ShaderCacheZipStream(const hecl::SystemChar* path) : m_reader(path) {
    if (m_reader.hasError())
      return;
    if (m_reader.readUint32Big() != SBIG('SHAD'))
      return;
    m_compBuf.reset(new uint8_t[4096]);
    m_zstrm.next_in = m_compBuf.get();
    m_zstrm.avail_in = 0;
    inflateInit(&m_zstrm);
  }
  ~ShaderCacheZipStream() { inflateEnd(&m_zstrm); }
  operator bool() const { return m_compBuf.operator bool(); }
  atUint64 readUBytesToBuf(void* buf, atUint64 len) {
    m_zstrm.next_out = (Bytef*)buf;
    m_zstrm.avail_out = len;
    m_zstrm.total_out = 0;
    while (m_zstrm.avail_out != 0) {
      if (m_zstrm.avail_in == 0) {
        atUint64 readSz = m_reader.readUBytesToBuf(m_compBuf.get(), 4096);
        m_zstrm.avail_in = readSz;
        m_zstrm.next_in = m_compBuf.get();
      }
      int inflateRet = inflate(&m_zstrm, Z_NO_FLUSH);
      if (inflateRet != Z_OK)
        break;
    }
    return m_zstrm.total_out;
  }
  void seek(atInt64, athena::SeekOrigin) {}
  atUint64 position() const { return 0; }
  atUint64 length() const { return 0; }
};

template <typename P, typename S>
void StageConverter<P, S>::loadFromStream(FactoryCtx& ctx, ShaderCacheZipStream& r) {
  uint32_t count = r.readUint32Big();
  for (uint32_t i = 0; i < count; ++i) {
    uint64_t hash = r.readUint64Big();
    uint32_t size = r.readUint32Big();
    StageBinaryData data = MakeStageBinaryData(size);
    r.readUBytesToBuf(data.get(), size);
    m_stageCache.insert(std::make_pair(hash, Do<StageTargetTp>(ctx, StageBinary<P, S>(data, size))));
  }
}

static boo::AdditionalPipelineInfo ReadAdditionalInfo(ShaderCacheZipStream& r) {
  boo::AdditionalPipelineInfo additionalInfo;
  additionalInfo.srcFac = boo::BlendFactor(r.readUint32Big());
  additionalInfo.dstFac = boo::BlendFactor(r.readUint32Big());
  additionalInfo.prim = boo::Primitive(r.readUint32Big());
  additionalInfo.depthTest = boo::ZTest(r.readUint32Big());
  additionalInfo.depthWrite = r.readBool();
  additionalInfo.colorWrite = r.readBool();
  additionalInfo.alphaWrite = r.readBool();
  additionalInfo.culling = boo::CullMode(r.readUint32Big());
  additionalInfo.patchSize = r.readUint32Big();
  additionalInfo.overwriteAlpha = r.readBool();
  additionalInfo.depthAttachment = r.readBool();
  return additionalInfo;
}

static std::vector<boo::VertexElementDescriptor> ReadVertexFormat(ShaderCacheZipStream& r) {
  std::vector<boo::VertexElementDescriptor> ret;
  uint32_t count = r.readUint32Big();
  ret.reserve(count);

  for (uint32_t i = 0; i < count; ++i) {
    ret.emplace_back();
    ret.back().semantic = boo::VertexSemantic(r.readUint32Big());
    ret.back().semanticIdx = int(r.readUint32Big());
  }

  return ret;
}

template <typename P>
bool PipelineConverter<P>::loadFromFile(FactoryCtx& ctx, const hecl::SystemChar* path) {
  ShaderCacheZipStream r(path);
  if (!r)
    return false;

  m_vertexConverter.loadFromStream(ctx, r);
  m_fragmentConverter.loadFromStream(ctx, r);
  m_geometryConverter.loadFromStream(ctx, r);
  m_controlConverter.loadFromStream(ctx, r);
  m_evaluationConverter.loadFromStream(ctx, r);

  uint32_t count = r.readUint32Big();
  for (uint32_t i = 0; i < count; ++i) {
    uint64_t hash = r.readUint64Big();
    StageRuntimeObject<P, PipelineStage::Vertex> vertex;
    StageRuntimeObject<P, PipelineStage::Fragment> fragment;
    StageRuntimeObject<P, PipelineStage::Geometry> geometry;
    StageRuntimeObject<P, PipelineStage::Control> control;
    StageRuntimeObject<P, PipelineStage::Evaluation> evaluation;
    if (uint64_t vhash = r.readUint64Big())
      vertex = m_vertexConverter.m_stageCache.find(vhash)->second;
    if (uint64_t fhash = r.readUint64Big())
      fragment = m_fragmentConverter.m_stageCache.find(fhash)->second;
    if (uint64_t ghash = r.readUint64Big())
      geometry = m_geometryConverter.m_stageCache.find(ghash)->second;
    if (uint64_t chash = r.readUint64Big())
      control = m_controlConverter.m_stageCache.find(chash)->second;
    if (uint64_t ehash = r.readUint64Big())
      evaluation = m_evaluationConverter.m_stageCache.find(ehash)->second;

    boo::AdditionalPipelineInfo additionalInfo = ReadAdditionalInfo(r);
    std::vector<boo::VertexElementDescriptor> vtxFmt = ReadVertexFormat(r);

    m_pipelineCache.insert(
        std::make_pair(hash, FinalPipeline<P>(*this, ctx,
                                              StageCollection<StageRuntimeObject<P, PipelineStage::Null>>(
                                                  vertex, fragment, geometry, control, evaluation, additionalInfo,
                                                  boo::VertexFormatInfo(vtxFmt.size(), vtxFmt.data())))));
  }

  return true;
}

#define SPECIALIZE_STAGE_CONVERTER(P)                                                                                  \
  template class StageConverter<P, PipelineStage::Vertex>;                                                             \
  template class StageConverter<P, PipelineStage::Fragment>;                                                           \
  template class StageConverter<P, PipelineStage::Geometry>;                                                           \
  template class StageConverter<P, PipelineStage::Control>;                                                            \
  template class StageConverter<P, PipelineStage::Evaluation>;

#if BOO_HAS_GL
template class PipelineConverter<PlatformType::OpenGL>;
SPECIALIZE_STAGE_CONVERTER(PlatformType::OpenGL)
#endif
#if BOO_HAS_VULKAN
template class PipelineConverter<PlatformType::Vulkan>;
SPECIALIZE_STAGE_CONVERTER(PlatformType::Vulkan)
#endif
#if _WIN32
template class PipelineConverter<PlatformType::D3D11>;
SPECIALIZE_STAGE_CONVERTER(PlatformType::D3D11)
#endif
#if BOO_HAS_METAL
template class PipelineConverter<PlatformType::Metal>;
SPECIALIZE_STAGE_CONVERTER(PlatformType::Metal)
#endif
#if BOO_HAS_NX
template class PipelineConverter<PlatformType::NX>;
SPECIALIZE_STAGE_CONVERTER(PlatformType::NX)
#endif

#endif

} // namespace hecl