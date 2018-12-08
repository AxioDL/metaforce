#pragma once

#include "ITweak.hpp"

namespace DataSpec {

struct ITweakPlayerControl : ITweak {
  virtual atUint32 GetMapping(atUint32) const = 0;
};

} // namespace DataSpec
