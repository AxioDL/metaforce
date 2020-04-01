#include "DataSpec/DNACommon/PART.hpp"
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAParticle {

template struct PPImpl<_GPSM<UniqueID32>>;
template struct PPImpl<_GPSM<UniqueID64>>;

AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_GPSM<UniqueID32>>)
AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_GPSM<UniqueID64>>)

template <>
std::string_view GPSM<UniqueID32>::DNAType() {
  return "GPSM<UniqueID32>"sv;
}

template <>
std::string_view GPSM<UniqueID64>::DNAType() {
  return "GPSM<UniqueID64>"sv;
}

template <class IDType>
bool ExtractGPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    GPSM<IDType> gpsm;
    gpsm.read(rs);
    athena::io::ToYAMLStream(gpsm, writer);
    return true;
  }
  return false;
}
template bool ExtractGPSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractGPSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteGPSM(const GPSM<IDType>& gpsm, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  gpsm.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteGPSM<UniqueID32>(const GPSM<UniqueID32>& gpsm, const hecl::ProjectPath& outPath);
template bool WriteGPSM<UniqueID64>(const GPSM<UniqueID64>& gpsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
