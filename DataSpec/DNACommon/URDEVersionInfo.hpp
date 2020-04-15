#pragma once

#include <string>

#include "DataSpec/DNACommon/DNACommon.hpp"

namespace DataSpec {
enum class EGame {
  Invalid = 0,
  MetroidPrime1 = 1,
  MetroidPrime2 = 2,
  MetroidPrime3 = 3,
};

enum class ERegion { Invalid = 0, NTSC_U = 'E', PAL = 'P', NTSC_J = 'J' };

struct URDEVersionInfo : BigDNA {
  AT_DECL_DNA_YAML

  String<-1> version;
  Value<ERegion> region;
  Value<EGame> game;
  Value<bool> isTrilogy;
};
} // namespace DataSpec
