#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct FogVolume : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> volume;
  Value<float> flickerSpeed;
  Value<float> unknown2;
  Value<atVec4f> fogColor;
  Value<bool> active;
};
} // namespace DataSpec::DNAMP1
