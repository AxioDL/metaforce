#include "DataSpec/DNACommon/DPSC.hpp"
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAParticle {

template struct PPImpl<_DPSM<UniqueID32>>;
template struct PPImpl<_DPSM<UniqueID64>>;

AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_DPSM<UniqueID32>>)
AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_DPSM<UniqueID64>>)

template <>
std::string_view PPImpl<_DPSM<UniqueID32>>::DNAType() {
  return "DPSM<UniqueID32>"sv;
}

template <>
std::string_view PPImpl<_DPSM<UniqueID64>>::DNAType() {
  return "DPSM<UniqueID64>"sv;
}

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
