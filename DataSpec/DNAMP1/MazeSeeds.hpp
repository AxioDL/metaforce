#pragma once

#include <vector>

#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1 {
struct MazeSeeds : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> seeds[300];
};
} // namespace DataSpec::DNAMP1
