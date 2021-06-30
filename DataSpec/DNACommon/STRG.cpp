#include "DataSpec/DNACommon/STRG.hpp"

#include "DataSpec/DNAMP1/STRG.hpp"
#include "DataSpec/DNAMP2/STRG.hpp"
#include "DataSpec/DNAMP3/STRG.hpp"

#include <logvisor/logvisor.hpp>

namespace DataSpec {

void ISTRG::gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const { /* TODO: parse out resource tokens */
}

std::unique_ptr<ISTRG> LoadSTRG(athena::io::IStreamReader& reader) {
  uint32_t magic = reader.readUint32Big();
  if (magic != 0x87654321) {
    LogDNACommon.report(logvisor::Error, FMT_STRING("invalid STRG magic"));
    return {};
  }

  uint32_t version = reader.readUint32Big();
  switch (version) {
  case 0: {
    auto* newStrg = new DNAMP1::STRG;
    newStrg->_read(reader);
    return std::unique_ptr<ISTRG>(newStrg);
  }
  case 1: {
    auto* newStrg = new DNAMP2::STRG;
    newStrg->_read(reader);
    return std::unique_ptr<ISTRG>(newStrg);
  }
  case 3: {
    auto* newStrg = new DNAMP3::STRG;
    newStrg->_read(reader);
    return std::unique_ptr<ISTRG>(newStrg);
  }
  default:
    break;
  }
  return {};
}
} // namespace DataSpec
