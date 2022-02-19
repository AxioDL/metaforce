#pragma once

#include "Runtime/Tweaks/ITweak.hpp"
#include "Runtime/Input/ControlMapper.hpp"

namespace metaforce::Tweaks {

struct ITweakPlayerControl : ITweak {
  [[nodiscard]] virtual ControlMapper::EFunctionList GetMapping(atUint32) const = 0;
};

} // namespace DataSpec
