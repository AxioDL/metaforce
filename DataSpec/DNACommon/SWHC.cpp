#include "DataSpec/DNACommon/SWHC.hpp"
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAParticle {

template struct PPImpl<_SWSH<UniqueID32>>;
template struct PPImpl<_SWSH<UniqueID64>>;

AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_SWSH<UniqueID32>>)
AT_SUBSPECIALIZE_DNA_YAML(PPImpl<_SWSH<UniqueID64>>)

template <>
std::string_view SWSH<UniqueID32>::DNAType() {
  return "SWSH<UniqueID32>"sv;
}

template <>
std::string_view SWSH<UniqueID64>::DNAType() {
  return "SWSH<UniqueID64>"sv;
}

template <class IDType>
bool ExtractSWSH(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    SWSH<IDType> swsh;
    swsh.read(rs);
    athena::io::ToYAMLStream(swsh, writer);
    return true;
  }
  return false;
}
template bool ExtractSWSH<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractSWSH<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteSWSH(const SWSH<IDType>& swsh, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  swsh.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteSWSH<UniqueID32>(const SWSH<UniqueID32>& swsh, const hecl::ProjectPath& outPath);
template bool WriteSWSH<UniqueID64>(const SWSH<UniqueID64>& swsh, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
