#include "DataSpec/DNACommon/ELSC.hpp"

#include <logvisor/logvisor.hpp>

namespace DataSpec::DNAParticle {

template <class IDType>
void ELSM<IDType>::_read(athena::io::IStreamReader& r) {
  DNAFourCC clsId;
  clsId.read(r);
  if (clsId != SBIG('ELSM')) {
    LogModule.report(logvisor::Warning, fmt("non ELSM provided to ELSM parser"));
    return;
  }

  clsId.read(r);
  while (clsId != SBIG('_END')) {
    switch (clsId.toUint32()) {
    case SBIG('LIFE'):
      x0_LIFE.read(r);
      break;
    case SBIG('SLIF'):
      x4_SLIF.read(r);
      break;
    case SBIG('GRAT'):
      x8_GRAT.read(r);
      break;
    case SBIG('SCNT'):
      xc_SCNT.read(r);
      break;
    case SBIG('SSEG'):
      x10_SSEG.read(r);
      break;
    case SBIG('COLR'):
      x14_COLR.read(r);
      break;
    case SBIG('IEMT'):
      x18_IEMT.read(r);
      break;
    case SBIG('FEMT'):
      x1c_FEMT.read(r);
      break;
    case SBIG('AMPL'):
      x20_AMPL.read(r);
      break;
    case SBIG('AMPD'):
      x24_AMPD.read(r);
      break;
    case SBIG('LWD1'):
      x28_LWD1.read(r);
      break;
    case SBIG('LWD2'):
      x2c_LWD2.read(r);
      break;
    case SBIG('LWD3'):
      x30_LWD3.read(r);
      break;
    case SBIG('LCL1'):
      x34_LCL1.read(r);
      break;
    case SBIG('LCL2'):
      x38_LCL2.read(r);
      break;
    case SBIG('LCL3'):
      x3c_LCL3.read(r);
      break;
    case SBIG('SSWH'):
      x40_SSWH.read(r);
      break;
    case SBIG('GPSM'):
      x50_GPSM.read(r);
      break;
    case SBIG('EPSM'):
      x60_EPSM.read(r);
      break;
    case SBIG('ZERY'):
      x70_ZERY.read(r);
      break;
    default:
      LogModule.report(logvisor::Fatal, fmt("Unknown ELSM class {} @{}"), clsId, r.position());
      break;
    }
    clsId.read(r);
  }
}

template <class IDType>
void ELSM<IDType>::_write(athena::io::IStreamWriter& w) const {
  w.writeBytes((atInt8*)"ELSM", 4);
  if (x0_LIFE) {
    w.writeBytes((atInt8*)"LIFE", 4);
    x0_LIFE.write(w);
  }
  if (x4_SLIF) {
    w.writeBytes((atInt8*)"SLIF", 4);
    x4_SLIF.write(w);
  }
  if (x8_GRAT) {
    w.writeBytes((atInt8*)"GRAT", 4);
    x8_GRAT.write(w);
  }
  if (xc_SCNT) {
    w.writeBytes((atInt8*)"SCNT", 4);
    xc_SCNT.write(w);
  }
  if (x10_SSEG) {
    w.writeBytes((atInt8*)"SSEG", 4);
    x10_SSEG.write(w);
  }
  if (x14_COLR) {
    w.writeBytes((atInt8*)"COLR", 4);
    x14_COLR.write(w);
  }
  if (x18_IEMT) {
    w.writeBytes((atInt8*)"IEMT", 4);
    x18_IEMT.write(w);
  }
  if (x1c_FEMT) {
    w.writeBytes((atInt8*)"FEMT", 4);
    x1c_FEMT.write(w);
  }
  if (x20_AMPL) {
    w.writeBytes((atInt8*)"AMPL", 4);
    x20_AMPL.write(w);
  }
  if (x24_AMPD) {
    w.writeBytes((atInt8*)"AMPD", 4);
    x24_AMPD.write(w);
  }
  if (x28_LWD1) {
    w.writeBytes((atInt8*)"LWD1", 4);
    x28_LWD1.write(w);
  }
  if (x2c_LWD2) {
    w.writeBytes((atInt8*)"LWD2", 4);
    x2c_LWD2.write(w);
  }
  if (x30_LWD3) {
    w.writeBytes((atInt8*)"LWD3", 4);
    x30_LWD3.write(w);
  }
  if (x34_LCL1) {
    w.writeBytes((atInt8*)"LCL1", 4);
    x34_LCL1.write(w);
  }
  if (x38_LCL2) {
    w.writeBytes((atInt8*)"LCL2", 4);
    x38_LCL2.write(w);
  }
  if (x3c_LCL3) {
    w.writeBytes((atInt8*)"LCL3", 4);
    x3c_LCL3.write(w);
  }
  if (x40_SSWH) {
    w.writeBytes((atInt8*)"SSWH", 4);
    x40_SSWH.write(w);
  }
  if (x50_GPSM) {
    w.writeBytes((atInt8*)"GPSM", 4);
    x50_GPSM.write(w);
  }
  if (x60_EPSM) {
    w.writeBytes((atInt8*)"EPSM", 4);
    x60_EPSM.write(w);
  }
  if (x70_ZERY) {
    w.writeBytes((atInt8*)"ZERY", 4);
    x70_ZERY.write(w);
  }
  w.writeBytes("_END", 4);
}

template <class IDType>
void ELSM<IDType>::_binarySize(size_t& s) const {
  s += 4;
  if (x0_LIFE) {
    s += 4;
    x0_LIFE.binarySize(s);
  }
  if (x4_SLIF) {
    s += 4;
    x4_SLIF.binarySize(s);
  }
  if (x8_GRAT) {
    s += 4;
    x8_GRAT.binarySize(s);
  }
  if (xc_SCNT) {
    s += 4;
    xc_SCNT.binarySize(s);
  }
  if (x10_SSEG) {
    s += 4;
    x10_SSEG.binarySize(s);
  }
  if (x14_COLR) {
    s += 4;
    x14_COLR.binarySize(s);
  }
  if (x18_IEMT) {
    s += 4;
    x18_IEMT.binarySize(s);
  }
  if (x1c_FEMT) {
    s += 4;
    x1c_FEMT.binarySize(s);
  }
  if (x20_AMPL) {
    s += 4;
    x20_AMPL.binarySize(s);
  }
  if (x24_AMPD) {
    s += 4;
    x24_AMPD.binarySize(s);
  }
  if (x28_LWD1) {
    s += 4;
    x28_LWD1.binarySize(s);
  }
  if (x2c_LWD2) {
    s += 4;
    x2c_LWD2.binarySize(s);
  }
  if (x30_LWD3) {
    s += 4;
    x30_LWD3.binarySize(s);
  }
  if (x34_LCL1) {
    s += 4;
    x34_LCL1.binarySize(s);
  }
  if (x38_LCL2) {
    s += 4;
    x38_LCL2.binarySize(s);
  }
  if (x3c_LCL3) {
    s += 4;
    x3c_LCL3.binarySize(s);
  }
  if (x40_SSWH) {
    s += 4;
    x40_SSWH.binarySize(s);
  }
  if (x50_GPSM) {
    s += 4;
    x50_GPSM.binarySize(s);
  }
  if (x60_EPSM) {
    s += 4;
    x60_EPSM.binarySize(s);
  }
  if (x70_ZERY) {
    s += 4;
    x70_ZERY.binarySize(s);
  }
}

template <class IDType>
void ELSM<IDType>::_read(athena::io::YAMLDocReader& r) {
  for (const auto& elem : r.getCurNode()->m_mapChildren) {
    if (elem.first.size() < 4) {
      LogModule.report(logvisor::Warning, fmt("short FourCC in element '{}'"), elem.first);
      continue;
    }

    if (auto rec = r.enterSubRecord(elem.first.c_str())) {
      switch (*reinterpret_cast<const uint32_t*>(elem.first.data())) {
      case SBIG('LIFE'):
        x0_LIFE.read(r);
        break;
      case SBIG('SLIF'):
        x4_SLIF.read(r);
        break;
      case SBIG('GRAT'):
        x8_GRAT.read(r);
        break;
      case SBIG('SCNT'):
        xc_SCNT.read(r);
        break;
      case SBIG('SSEG'):
        x10_SSEG.read(r);
        break;
      case SBIG('COLR'):
        x14_COLR.read(r);
        break;
      case SBIG('IEMT'):
        x18_IEMT.read(r);
        break;
      case SBIG('FEMT'):
        x1c_FEMT.read(r);
        break;
      case SBIG('AMPL'):
        x20_AMPL.read(r);
        break;
      case SBIG('AMPD'):
        x24_AMPD.read(r);
        break;
      case SBIG('LWD1'):
        x28_LWD1.read(r);
        break;
      case SBIG('LWD2'):
        x2c_LWD2.read(r);
        break;
      case SBIG('LWD3'):
        x30_LWD3.read(r);
        break;
      case SBIG('LCL1'):
        x34_LCL1.read(r);
        break;
      case SBIG('LCL2'):
        x38_LCL2.read(r);
        break;
      case SBIG('LCL3'):
        x3c_LCL3.read(r);
        break;
      case SBIG('SSWH'):
        x40_SSWH.read(r);
        break;
      case SBIG('GPSM'):
        x50_GPSM.read(r);
        break;
      case SBIG('EPSM'):
        x60_EPSM.read(r);
        break;
      case SBIG('ZERY'):
        x70_ZERY.read(r);
        break;
      default:
        break;
      }
    }
  }
}

template <class IDType>
void ELSM<IDType>::_write(athena::io::YAMLDocWriter& w) const {
  if (x0_LIFE)
    if (auto rec = w.enterSubRecord("LIFE"))
      x0_LIFE.write(w);
  if (x4_SLIF)
    if (auto rec = w.enterSubRecord("SLIF"))
      x4_SLIF.write(w);
  if (x8_GRAT)
    if (auto rec = w.enterSubRecord("GRAT"))
      x8_GRAT.write(w);
  if (xc_SCNT)
    if (auto rec = w.enterSubRecord("SCNT"))
      xc_SCNT.write(w);
  if (x10_SSEG)
    if (auto rec = w.enterSubRecord("SSEG"))
      x10_SSEG.write(w);
  if (x14_COLR)
    if (auto rec = w.enterSubRecord("COLR"))
      x14_COLR.write(w);
  if (x18_IEMT)
    if (auto rec = w.enterSubRecord("IEMT"))
      x18_IEMT.write(w);
  if (x1c_FEMT)
    if (auto rec = w.enterSubRecord("FEMT"))
      x1c_FEMT.write(w);
  if (x20_AMPL)
    if (auto rec = w.enterSubRecord("AMPL"))
      x20_AMPL.write(w);
  if (x24_AMPD)
    if (auto rec = w.enterSubRecord("AMPD"))
      x24_AMPD.write(w);
  if (x28_LWD1)
    if (auto rec = w.enterSubRecord("LWD1"))
      x28_LWD1.write(w);
  if (x2c_LWD2)
    if (auto rec = w.enterSubRecord("LWD2"))
      x2c_LWD2.write(w);
  if (x30_LWD3)
    if (auto rec = w.enterSubRecord("LWD3"))
      x30_LWD3.write(w);
  if (x34_LCL1)
    if (auto rec = w.enterSubRecord("LCL1"))
      x34_LCL1.write(w);
  if (x38_LCL2)
    if (auto rec = w.enterSubRecord("LCL2"))
      x38_LCL2.write(w);
  if (x3c_LCL3)
    if (auto rec = w.enterSubRecord("LCL3"))
      x3c_LCL3.write(w);
  if (x40_SSWH)
    if (auto rec = w.enterSubRecord("SSWH"))
      x40_SSWH.write(w);
  if (x50_GPSM)
    if (auto rec = w.enterSubRecord("GPSM"))
      x50_GPSM.write(w);
  if (x60_EPSM)
    if (auto rec = w.enterSubRecord("EPSM"))
      x60_EPSM.write(w);
  if (x70_ZERY)
    if (auto rec = w.enterSubRecord("ZERY"))
      x70_ZERY.write(w);
}

AT_SUBSPECIALIZE_DNA_YAML(ELSM<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(ELSM<UniqueID64>)

template <>
std::string_view ELSM<UniqueID32>::DNAType() {
  return "urde::ELSM<UniqueID32>"sv;
}

template <>
std::string_view ELSM<UniqueID64>::DNAType() {
  return "urde::ELSM<UniqueID64>"sv;
}

template <class IDType>
void ELSM<IDType>::gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const {
  g_curSpec->flattenDependencies(x40_SSWH.id, pathsOut);
  g_curSpec->flattenDependencies(x50_GPSM.id, pathsOut);
  g_curSpec->flattenDependencies(x60_EPSM.id, pathsOut);
}

template struct ELSM<UniqueID32>;
template struct ELSM<UniqueID64>;

template <class IDType>
bool ExtractELSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    ELSM<IDType> elsm;
    elsm.read(rs);
    athena::io::ToYAMLStream(elsm, writer);
    return true;
  }
  return false;
}
template bool ExtractELSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractELSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteELSM(const ELSM<IDType>& elsm, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  elsm.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteELSM<UniqueID32>(const ELSM<UniqueID32>& gpsm, const hecl::ProjectPath& outPath);
template bool WriteELSM<UniqueID64>(const ELSM<UniqueID64>& gpsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
