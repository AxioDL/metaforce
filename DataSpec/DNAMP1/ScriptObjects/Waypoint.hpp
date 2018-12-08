#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Waypoint : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> active;
  Value<float> speed;
  Value<float> pause;
  Value<atUint32> patternTranslate;
  Value<atUint32> patternOrient;
  Value<atUint32> patternFit;
  Value<atUint32> behaviour;
  Value<atUint32> behaviourOrient;
  Value<atUint32> behaviourModifiers; // 0x2: single, 0x4: double
  Value<atUint32> animation;
};
} // namespace DataSpec::DNAMP1
