#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct TeamAIMgr : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atUint32> aiCount;
  Value<atUint32> meleeCount;
  Value<atUint32> projectileCount;
  Value<atUint32> unknownCount;
  Value<atUint32> maxMeleeAttackerCount;
  Value<atUint32> maxProjectileAttackerCount;
  Value<atUint32> positionMode;
  Value<float> meleeTimeInterval;
  Value<float> projectileTimeInterval;
};
} // namespace DataSpec::DNAMP1
