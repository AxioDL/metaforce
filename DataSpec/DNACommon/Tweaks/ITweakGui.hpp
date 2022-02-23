#pragma once

#include "ITweak.hpp"
#include "zeus/CVector2f.hpp"

namespace DataSpec {

struct ITweakGui : ITweak {
  enum class EHudVisMode : atUint32 { Zero, One, Two, Three };
  enum class EHelmetVisMode : atUint32 { ReducedUpdate, NotVisible, Deco, HelmetDeco, GlowHelmetDeco, HelmetOnly };
};

} // namespace DataSpec
