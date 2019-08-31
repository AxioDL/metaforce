#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct ActorRotate : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> rotationOffset;
  Value<float> timeScale;
  Value<bool> updateActors;
  Value<bool> updateOnCreation;
  Value<bool> active;
};
} // namespace DataSpec::DNAMP1
