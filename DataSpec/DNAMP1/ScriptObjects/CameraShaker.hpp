#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct CameraShaker : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<float> xMag;
  Value<float> xB;
  Value<float> yMag;
  Value<float> yB;
  Value<float> zMag;
  Value<float> zB;
  Value<float> duration;
  Value<bool> active;
};
} // namespace DataSpec::DNAMP1
