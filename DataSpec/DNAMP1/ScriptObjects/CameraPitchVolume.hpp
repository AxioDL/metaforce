#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct CameraPitchVolume : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> volume;
  Value<bool> active;
  Value<float> upPitch;
  Value<float> downPitch;
  Value<float> scale;
};
} // namespace DataSpec::DNAMP1
