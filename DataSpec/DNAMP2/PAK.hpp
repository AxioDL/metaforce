#pragma once

#include "../DNAMP1/PAK.hpp"

namespace DataSpec::DNAMP2 {

/* Same PAK format as MP1 */
struct PAK : DNAMP1::PAK {
  using DNAMP1::PAK::PAK;
  std::string bestEntryName(const nod::Node& pakNode, const Entry& entry, std::string& catalogueName) const;
};

} // namespace DataSpec::DNAMP2
