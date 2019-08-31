#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct CoverPoint : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> active;
  Value<atUint32> flags;
  Value<bool> crouch;
  Value<float> horizontalAngle;
  Value<float> verticleAngle;
  Value<float> coverTime;
};
} // namespace DataSpec::DNAMP1
