#include "DataSpec/DNACommon/ELSC.hpp"
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAParticle {

template struct PPImpl<_ELSM<UniqueID32>>;
template struct PPImpl<_ELSM<UniqueID64>>;

AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_ELSM<UniqueID32>>)
AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_ELSM<UniqueID64>>)

template <>
std::string_view ELSM<UniqueID32>::DNAType() {
  return "urde::ELSM<UniqueID32>"sv;
}

template <>
std::string_view ELSM<UniqueID64>::DNAType() {
  return "urde::ELSM<UniqueID64>"sv;
}

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
