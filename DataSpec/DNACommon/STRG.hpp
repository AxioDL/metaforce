#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "DataSpec/DNACommon/DNACommon.hpp"

#include <hecl/hecl.hpp>

namespace athena::io {
class IStreamReader;
}

namespace DataSpec {
struct ISTRG : BigDNAVYaml {
  ~ISTRG() override = default;

  virtual size_t count() const = 0;
  virtual std::string getUTF8(const FourCC& lang, size_t idx) const = 0;
  virtual std::u16string getUTF16(const FourCC& lang, size_t idx) const = 0;
  virtual hecl::SystemString getSystemString(const FourCC& lang, size_t idx) const = 0;
  virtual int32_t lookupIdx(std::string_view name) const = 0;

  virtual void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const;
};
std::unique_ptr<ISTRG> LoadSTRG(athena::io::IStreamReader& reader);

} // namespace DataSpec
