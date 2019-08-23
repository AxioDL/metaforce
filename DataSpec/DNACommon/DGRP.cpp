#include "DataSpec/DNACommon/DGRP.hpp"

#include <athena/DNAYaml.hpp>
#include <athena/FileWriter.hpp>
#include <athena/IStreamWriter.hpp>

#include <hecl/hecl.hpp>

namespace DataSpec::DNADGRP {

template <class IDType>
bool ExtractDGRP(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    DGRP<IDType> dgrp;
    dgrp.read(rs);
    athena::io::ToYAMLStream(dgrp, writer);
    return true;
  }
  return false;
}
template bool ExtractDGRP<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractDGRP<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteDGRP(const DGRP<IDType>& dgrp, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  dgrp.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteDGRP<UniqueID32>(const DGRP<UniqueID32>& dgrp, const hecl::ProjectPath& outPath);
template bool WriteDGRP<UniqueID64>(const DGRP<UniqueID64>& dgrp, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNADGRP
