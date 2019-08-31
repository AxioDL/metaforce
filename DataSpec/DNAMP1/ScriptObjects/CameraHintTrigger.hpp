#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct CameraHintTrigger : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> volume;
  Value<bool> active;
  Value<bool> deactivateOnEntered;
  Value<bool> deactivateOnExited;
};
} // namespace DataSpec::DNAMP1
