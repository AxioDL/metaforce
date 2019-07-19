#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct BallTrigger : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> volume;
  Value<bool> active;
  Value<float> force;
  Value<float> minAngle;
  Value<float> maxDistance;
  Value<atVec3f> forceAngle;
  Value<bool> stopPlayer;
};
} // namespace DataSpec::DNAMP1
