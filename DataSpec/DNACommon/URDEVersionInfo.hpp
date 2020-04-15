#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"

namespace DataSpec {
enum class ERegion {
  Invalid = -1,
  NTSC_U = 'E',
  PAL = 'P',
  NTSC_J = 'J'
};
enum class EGame {
  Invalid=0,
  MetroidPrime1,
  MetroidPrime2,
  MetroidPrime3,
};

struct URDEVersionInfo : public BigDNA {
  AT_DECL_DNA_YAML
  String<-1> version;
  Value<ERegion> region;
  Value<EGame> game;
  Value<bool> isTrilogy;
};
}