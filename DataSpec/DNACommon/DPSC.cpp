#include "DataSpec/DNACommon/DPSC.hpp"

#include "DataSpec/DNACommon/PAK.hpp"

#include <athena/DNAYaml.hpp>
#include <logvisor/logvisor.hpp>

namespace DataSpec::DNAParticle {

template <>
std::string_view DPSM<UniqueID32>::DNAType() {
  return "DPSM<UniqueID32>"sv;
}

template <>
std::string_view DPSM<UniqueID64>::DNAType() {
  return "DPSM<UniqueID64>"sv;
}

template <class IDType>
void DPSM<IDType>::_read(athena::io::YAMLDocReader& r) {
  for (const auto& elem : r.getCurNode()->m_mapChildren) {
    if (elem.first.size() < 4) {
      LogModule.report(logvisor::Warning, fmt("short FourCC in element '{}'"), elem.first);
      continue;
    }

    if (auto rec = r.enterSubRecord(elem.first.c_str())) {
      bool loadFirstDesc = false;
      uint32_t clsId = *reinterpret_cast<const uint32_t*>(elem.first.c_str());
      switch (clsId) {
      case SBIG('1SZE'):
      case SBIG('1LFT'):
      case SBIG('1ROT'):
      case SBIG('1OFF'):
      case SBIG('1CLR'):
      case SBIG('1TEX'):
      case SBIG('1ADD'):
        loadFirstDesc = true;
        [[fallthrough]];
      case SBIG('2SZE'):
      case SBIG('2LFT'):
      case SBIG('2ROT'):
      case SBIG('2OFF'):
      case SBIG('2CLR'):
      case SBIG('2TEX'):
      case SBIG('2ADD'):
        if (loadFirstDesc)
          readQuadDecalInfo(r, clsId, x0_quad);
        else
          readQuadDecalInfo(r, clsId, x1c_quad);
        break;
      case SBIG('DMDL'):
        x38_DMDL.read(r);
        break;
      case SBIG('DLFT'):
        x48_DLFT.read(r);
        break;
      case SBIG('DMOP'):
        x4c_DMOP.read(r);
        break;
      case SBIG('DMRT'):
        x50_DMRT.read(r);
        break;
      case SBIG('DMSC'):
        x54_DMSC.read(r);
        break;
      case SBIG('DMCL'):
        x58_DMCL.read(r);
        break;
      case SBIG('DMAB'):
        x5c_24_DMAB = r.readBool();
        break;
      case SBIG('DMOO'):
        x5c_25_DMOO = r.readBool();
        break;
      }
    }
  }
}

template <class IDType>
void DPSM<IDType>::_write(athena::io::YAMLDocWriter& w) const {
  writeQuadDecalInfo(w, x0_quad, true);
  writeQuadDecalInfo(w, x1c_quad, false);

  if (x38_DMDL)
    if (auto rec = w.enterSubRecord("DMDL"))
      x38_DMDL.write(w);
  if (x48_DLFT)
    if (auto rec = w.enterSubRecord("DLFT"))
      x48_DLFT.write(w);
  if (x4c_DMOP)
    if (auto rec = w.enterSubRecord("DMOP"))
      x4c_DMOP.write(w);
  if (x50_DMRT)
    if (auto rec = w.enterSubRecord("DMRT"))
      x50_DMRT.write(w);
  if (x54_DMSC)
    if (auto rec = w.enterSubRecord("DMSC"))
      x54_DMSC.write(w);
  if (x58_DMCL)
    if (auto rec = w.enterSubRecord("DMCL"))
      x54_DMSC.write(w);

  if (x5c_24_DMAB)
    w.writeBool("DMAB", x5c_24_DMAB);
  if (x5c_25_DMOO)
    w.writeBool("DMOO", x5c_25_DMOO);
}

template <class IDType>
template <class Reader>
void DPSM<IDType>::readQuadDecalInfo(Reader& r, FourCC clsId, typename DPSM<IDType>::SQuadDescr& quad) {
  switch (clsId.toUint32()) {
  case SBIG('1LFT'):
  case SBIG('2LFT'):
    quad.x0_LFT.read(r);
    break;
  case SBIG('1SZE'):
  case SBIG('2SZE'):
    quad.x4_SZE.read(r);
    break;
  case SBIG('1ROT'):
  case SBIG('2ROT'):
    quad.x8_ROT.read(r);
    break;
  case SBIG('1OFF'):
  case SBIG('2OFF'):
    quad.xc_OFF.read(r);
    break;
  case SBIG('1CLR'):
  case SBIG('2CLR'):
    quad.x10_CLR.read(r);
    break;
  case SBIG('1TEX'):
  case SBIG('2TEX'):
    quad.x14_TEX.read(r);
    break;
  case SBIG('1ADD'):
  case SBIG('2ADD'):
    quad.x18_ADD.read(r);
    break;
  }
}

template <class IDType>
void DPSM<IDType>::writeQuadDecalInfo(athena::io::YAMLDocWriter& w, const typename DPSM<IDType>::SQuadDescr& quad,
                                      bool first) const {
  if (quad.x0_LFT)
    if (auto rec = w.enterSubRecord((first ? "1LFT" : "2LFT")))
      quad.x0_LFT.write(w);
  if (quad.x4_SZE)
    if (auto rec = w.enterSubRecord((first ? "1SZE" : "2SZE")))
      quad.x4_SZE.write(w);
  if (quad.x8_ROT)
    if (auto rec = w.enterSubRecord((first ? "1ROT" : "2ROT")))
      quad.x8_ROT.write(w);
  if (quad.xc_OFF)
    if (auto rec = w.enterSubRecord((first ? "1OFF" : "2OFF")))
      quad.xc_OFF.write(w);
  if (quad.x10_CLR)
    if (auto rec = w.enterSubRecord((first ? "1CLR" : "2CLR")))
      quad.x10_CLR.write(w);
  if (quad.x14_TEX)
    if (auto rec = w.enterSubRecord((first ? "1TEX" : "2TEX")))
      quad.x14_TEX.write(w);
  if (quad.x18_ADD)
    if (auto rec = w.enterSubRecord((first ? "1ADD" : "2ADD")))
      quad.x18_ADD.write(w);
}

template <class IDType>
void DPSM<IDType>::_binarySize(size_t& s) const {
  s += 4;
  getQuadDecalBinarySize(s, x0_quad);
  getQuadDecalBinarySize(s, x1c_quad);
  if (x38_DMDL) {
    s += 4;
    x38_DMDL.binarySize(s);
  }
  if (x48_DLFT) {
    s += 4;
    x48_DLFT.binarySize(s);
  }
  if (x4c_DMOP) {
    s += 4;
    x4c_DMOP.binarySize(s);
  }
  if (x50_DMRT) {
    s += 4;
    x50_DMRT.binarySize(s);
  }
  if (x54_DMSC) {
    s += 4;
    x54_DMSC.binarySize(s);
  }
  if (x58_DMCL) {
    x58_DMCL.binarySize(s);
  }
  if (x5c_24_DMAB)
    s += 9;
  if (x5c_25_DMOO)
    s += 9;
}

template <class IDType>
void DPSM<IDType>::getQuadDecalBinarySize(size_t& s, const typename DPSM<IDType>::SQuadDescr& quad) const {
  if (quad.x0_LFT) {
    s += 4;
    quad.x0_LFT.binarySize(s);
  }
  if (quad.x4_SZE) {
    s += 4;
    quad.x4_SZE.binarySize(s);
  }
  if (quad.x8_ROT) {
    s += 4;
    quad.x8_ROT.binarySize(s);
  }
  if (quad.xc_OFF) {
    s += 4;
    quad.xc_OFF.binarySize(s);
  }
  if (quad.x10_CLR) {
    s += 4;
    quad.x10_CLR.binarySize(s);
  }
  if (quad.x14_TEX) {
    s += 4;
    quad.x14_TEX.binarySize(s);
  }
  if (quad.x18_ADD) {
    s += 4;
    quad.x18_ADD.binarySize(s);
  }
}

template <class IDType>
void DPSM<IDType>::_read(athena::io::IStreamReader& r) {
  DNAFourCC clsId;
  clsId.read(r);
  if (clsId != SBIG('DPSM')) {
    LogModule.report(logvisor::Warning, fmt("non DPSM provided to DPSM parser"));
    return;
  }
  bool loadFirstDesc = false;
  clsId.read(r);
  while (clsId != SBIG('_END')) {
    switch (clsId.toUint32()) {
    case SBIG('1SZE'):
    case SBIG('1LFT'):
    case SBIG('1ROT'):
    case SBIG('1OFF'):
    case SBIG('1CLR'):
    case SBIG('1TEX'):
    case SBIG('1ADD'):
      loadFirstDesc = true;
      [[fallthrough]];
    case SBIG('2SZE'):
    case SBIG('2LFT'):
    case SBIG('2ROT'):
    case SBIG('2OFF'):
    case SBIG('2CLR'):
    case SBIG('2TEX'):
    case SBIG('2ADD'):
      if (loadFirstDesc)
        readQuadDecalInfo(r, clsId, x0_quad);
      else
        readQuadDecalInfo(r, clsId, x1c_quad);
      break;
    case SBIG('DMDL'):
      x38_DMDL.read(r);
      break;
    case SBIG('DLFT'):
      x48_DLFT.read(r);
      break;
    case SBIG('DMOP'):
      x4c_DMOP.read(r);
      break;
    case SBIG('DMRT'):
      x50_DMRT.read(r);
      break;
    case SBIG('DMSC'):
      x54_DMSC.read(r);
      break;
    case SBIG('DMCL'):
      x58_DMCL.read(r);
      break;
    case SBIG('DMAB'):
      r.readUint32();
      x5c_24_DMAB = r.readBool();
      break;
    case SBIG('DMOO'):
      r.readUint32();
      x5c_25_DMOO = r.readBool();
      break;
    default:
      LogModule.report(logvisor::Fatal, fmt("Unknown DPSM class {} @{}"), clsId, r.position());
      break;
    }
    clsId.read(r);
  }
}

template <class IDType>
void DPSM<IDType>::_write(athena::io::IStreamWriter& w) const {
  w.writeBytes("DPSM", 4);
  writeQuadDecalInfo(w, x0_quad, true);
  writeQuadDecalInfo(w, x1c_quad, false);
  if (x38_DMDL) {
    w.writeBytes("DMDL", 4);
    x38_DMDL.write(w);
  }
  if (x48_DLFT) {
    w.writeBytes("DLFT", 4);
    x48_DLFT.write(w);
  }
  if (x4c_DMOP) {
    w.writeBytes("DMOP", 4);
    x4c_DMOP.write(w);
  }
  if (x50_DMRT) {
    w.writeBytes("DMRT", 4);
    x50_DMRT.write(w);
  }
  if (x54_DMSC) {
    w.writeBytes("DMSC", 4);
    x54_DMSC.write(w);
  }
  if (x58_DMCL) {
    w.writeBytes("DMCL", 4);
    x58_DMCL.write(w);
  }
  if (x5c_24_DMAB)
    w.writeBytes("DMABCNST\x01", 9);
  if (x5c_25_DMOO)
    w.writeBytes("DMOOCNST\x01", 9);
  w.writeBytes("_END", 4);
}

template <class IDType>
void DPSM<IDType>::writeQuadDecalInfo(athena::io::IStreamWriter& w, const typename DPSM<IDType>::SQuadDescr& quad,
                                      bool first) const {
  if (quad.x0_LFT) {
    w.writeBytes((first ? "1LFT" : "2LFT"), 4);
    quad.x0_LFT.write(w);
  }
  if (quad.x4_SZE) {
    w.writeBytes((first ? "1SZE" : "2SZE"), 4);
    quad.x4_SZE.write(w);
  }
  if (quad.x8_ROT) {
    w.writeBytes((first ? "1ROT" : "2ROT"), 4);
    quad.x8_ROT.write(w);
  }
  if (quad.xc_OFF) {
    w.writeBytes((first ? "1OFF" : "2OFF"), 4);
    quad.xc_OFF.write(w);
  }
  if (quad.x10_CLR) {
    w.writeBytes((first ? "1CLR" : "2CLR"), 4);
    quad.x10_CLR.write(w);
  }
  if (quad.x14_TEX) {
    w.writeBytes((first ? "1TEX" : "2TEX"), 4);
    quad.x14_TEX.write(w);
  }
  if (quad.x18_ADD) {
    w.writeBytes((first ? "1ADD" : "2ADD"), 4);
    quad.x18_ADD.write(w);
  }
}

template <class IDType>
void DPSM<IDType>::gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const {
  if (x0_quad.x14_TEX.m_elem)
    x0_quad.x14_TEX.m_elem->gatherDependencies(pathsOut);
  if (x1c_quad.x14_TEX.m_elem)
    x1c_quad.x14_TEX.m_elem->gatherDependencies(pathsOut);
  g_curSpec->flattenDependencies(x38_DMDL.id, pathsOut);
}

AT_SUBSPECIALIZE_DNA_YAML(DPSM<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(DPSM<UniqueID64>)
template struct DPSM<UniqueID32>;
template struct DPSM<UniqueID64>;

template <class IDType>
bool ExtractDPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    DPSM<IDType> dpsm;
    dpsm.read(rs);
    athena::io::ToYAMLStream(dpsm, writer);
    return true;
  }
  return false;
}
template bool ExtractDPSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractDPSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteDPSM(const DPSM<IDType>& dpsm, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  dpsm.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteDPSM<UniqueID32>(const DPSM<UniqueID32>& dpsm, const hecl::ProjectPath& outPath);
template bool WriteDPSM<UniqueID64>(const DPSM<UniqueID64>& dpsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
