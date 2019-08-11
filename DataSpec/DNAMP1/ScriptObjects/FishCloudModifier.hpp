#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct FishCloudModifier : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> position;
  Value<bool> active;
  Value<bool> repulsor;
  Value<bool> swirl;
  Value<float> radius;
  Value<float> priority;
};
} // namespace DataSpec::DNAMP1
