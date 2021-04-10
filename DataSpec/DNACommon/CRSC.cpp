#include "DataSpec/DNACommon/CRSC.hpp"
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAParticle {

template struct PPImpl<_CRSM<UniqueID32>>;
template struct PPImpl<_CRSM<UniqueID64>>;

AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_CRSM<UniqueID32>>)
AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_CRSM<UniqueID64>>)

template <>
std::string_view PPImpl<_CRSM<UniqueID32>>::DNAType() {
  return "CRSM<UniqueID32>"sv;
}

template <>
std::string_view PPImpl<_CRSM<UniqueID64>>::DNAType() {
  return "CRSM<UniqueID64>"sv;
}

template <class IDType>
bool ExtractCRSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    CRSM<IDType> crsm;
    crsm.read(rs);
    athena::io::ToYAMLStream(crsm, writer);
    return true;
  }
  return false;
}
template bool ExtractCRSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractCRSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteCRSM(const CRSM<IDType>& crsm, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  crsm.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteCRSM<UniqueID32>(const CRSM<UniqueID32>& crsm, const hecl::ProjectPath& outPath);
template bool WriteCRSM<UniqueID64>(const CRSM<UniqueID64>& crsm, const hecl::ProjectPath& outPath);
} // namespace DataSpec::DNAParticle
