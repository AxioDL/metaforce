#include "DataSpec/DNACommon/WPSC.hpp"
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAParticle {

template struct PPImpl<_WPSM<UniqueID32>>;
template struct PPImpl<_WPSM<UniqueID64>>;

AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_WPSM<UniqueID32>>)
AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_WPSM<UniqueID64>>)

template <>
std::string_view WPSM<UniqueID32>::DNAType() {
  return "WPSM<UniqueID32>"sv;
}

template <>
std::string_view WPSM<UniqueID64>::DNAType() {
  return "WPSM<UniqueID64>"sv;
}

template <class IDType>
bool ExtractWPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    WPSM<IDType> wpsm;
    wpsm.read(rs);
    athena::io::ToYAMLStream(wpsm, writer);
    return true;
  }
  return false;
}
template bool ExtractWPSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractWPSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteWPSM(const WPSM<IDType>& wpsm, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  wpsm.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteWPSM<UniqueID32>(const WPSM<UniqueID32>& wpsm, const hecl::ProjectPath& outPath);
template bool WriteWPSM<UniqueID64>(const WPSM<UniqueID64>& wpsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
