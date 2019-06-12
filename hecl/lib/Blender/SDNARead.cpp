#include "hecl/Blender/SDNARead.hpp"
#include "athena/FileReader.hpp"
#include <zlib.h>

namespace hecl::blender {

void SDNABlock::SDNAStruct::computeOffsets(const SDNABlock& block) {
  atUint32 offset = 0;
  for (SDNAField& f : fields) {
    const auto& name = block.names[f.name];
    f.offset = offset;
    if (name.front() == '*') {
      offset += 8;
    } else {
      atUint32 length = block.tlens[f.type];
      auto bracket = name.find('[');
      if (bracket != std::string::npos)
        length *= strtoul(name.data() + bracket + 1, nullptr, 10);
      offset += length;
    }
  }
}

const SDNABlock::SDNAStruct::SDNAField* SDNABlock::SDNAStruct::lookupField(const SDNABlock& block,
                                                                           const char* n) const {
  for (const SDNAField& field : fields) {
    const auto& name = block.names[field.name];
    auto bracket = name.find('[');
    if (bracket != std::string::npos) {
      if (!name.compare(0, bracket, n))
        return &field;
    } else if (!name.compare(n))
      return &field;
  }
  return nullptr;
}

const SDNABlock::SDNAStruct* SDNABlock::lookupStruct(const char* n, atUint32& idx) const {
  idx = 0;
  for (const SDNAStruct& strc : strcs) {
    const auto& name = types[strc.type];
    if (!name.compare(n))
      return &strc;
    ++idx;
  }
  return nullptr;
}

void SDNARead::enumerate(const std::function<bool(const FileBlock& block, athena::io::MemoryReader& r)>& func) const {
  athena::io::MemoryReader r(m_data.data(), m_data.size());
  r.seek(12);
  while (r.position() < r.length()) {
    FileBlock block;
    block.read(r);
    if (block.type == FOURCC('ENDB'))
      break;
    athena::io::MemoryReader r2(m_data.data() + r.position(), block.size);
    if (!func(block, r2))
      break;
    r.seek(block.size);
  }
}

SDNARead::SDNARead(SystemStringView path) {
  athena::io::FileReader r(path);
  if (r.hasError())
    return;

  atUint64 length = r.length();
  char magicBuf[7];
  r.readUBytesToBuf(magicBuf, 7);
  r.seek(0, athena::Begin);
  if (strncmp(magicBuf, "BLENDER", 7)) {
    /* Try gzip decompression */
    std::unique_ptr<uint8_t[]> compBuf(new uint8_t[4096]);
    m_data.resize((length * 2 + 4095) & ~4095);
    z_stream zstrm = {};
    inflateInit2(&zstrm, 16 + MAX_WBITS);
    zstrm.next_out = (Bytef*)m_data.data();
    zstrm.avail_out = m_data.size();
    zstrm.total_out = 0;

    atUint64 rs;
    while ((rs = r.readUBytesToBuf(compBuf.get(), 4096))) {
      int inflateRet;
      zstrm.next_in = compBuf.get();
      zstrm.avail_in = rs;
      while (zstrm.avail_in) {
        if (!zstrm.avail_out) {
          zstrm.avail_out = m_data.size();
          m_data.resize(zstrm.avail_out * 2);
          zstrm.next_out = (Bytef*)m_data.data() + zstrm.avail_out;
        }
        inflateRet = inflate(&zstrm, Z_NO_FLUSH);
        if (inflateRet == Z_STREAM_END)
          break;
        if (inflateRet != Z_OK) {
          inflateEnd(&zstrm);
          m_data = std::vector<uint8_t>();
          return;
        }
      }
      if (inflateRet == Z_STREAM_END)
        break;
    }

    inflateEnd(&zstrm);

    if (strncmp((char*)m_data.data(), "BLENDER", 7)) {
      m_data = std::vector<uint8_t>();
      return;
    }
  } else {
    m_data.resize(length);
    r.readUBytesToBuf(m_data.data(), length);
  }

  enumerate([this](const FileBlock& block, athena::io::MemoryReader& r) {
    if (block.type == FOURCC('DNA1')) {
      m_sdnaBlock.read(r);
      for (SDNABlock::SDNAStruct& s : m_sdnaBlock.strcs)
        s.computeOffsets(m_sdnaBlock);
      return false;
    }
    return true;
  });
}

BlendType GetBlendType(SystemStringView path) {
  SDNARead r(path);
  if (!r)
    return BlendType::None;

  atUint32 idPropIdx;
  const auto* idPropStruct = r.sdnaBlock().lookupStruct("IDProperty", idPropIdx);
  if (!idPropStruct)
    return BlendType::None;
  const auto* typeField = idPropStruct->lookupField(r.sdnaBlock(), "type");
  if (!typeField)
    return BlendType::None;
  atUint32 typeOffset = typeField->offset;
  const auto* nameField = idPropStruct->lookupField(r.sdnaBlock(), "name");
  if (!nameField)
    return BlendType::None;
  atUint32 nameOffset = nameField->offset;
  const auto* dataField = idPropStruct->lookupField(r.sdnaBlock(), "data");
  if (!dataField)
    return BlendType::None;
  atUint32 dataOffset = dataField->offset;

  atUint32 idPropDataIdx;
  const auto* idPropDataStruct = r.sdnaBlock().lookupStruct("IDPropertyData", idPropDataIdx);
  if (!idPropDataStruct)
    return BlendType::None;
  const auto* valField = idPropDataStruct->lookupField(r.sdnaBlock(), "val");
  if (!valField)
    return BlendType::None;
  atUint32 valOffset = dataOffset + valField->offset;

  BlendType ret = BlendType::None;
  r.enumerate(
      [idPropIdx, typeOffset, nameOffset, valOffset, &ret](const FileBlock& block, athena::io::MemoryReader& r) {
        if (block.type == FOURCC('DATA') && block.sdnaIdx == idPropIdx) {
          r.seek(typeOffset, athena::Begin);
          if (r.readUByte() != 1)
            return true;

          r.seek(nameOffset, athena::Begin);
          if (r.readString() != "hecl_type")
            return true;

          r.seek(valOffset, athena::Begin);
          ret = BlendType(r.readUint32Little());
          return false;
        }
        return true;
      });

  return ret;
}

} // namespace hecl::blender
