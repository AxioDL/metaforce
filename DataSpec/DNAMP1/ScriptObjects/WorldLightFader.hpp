#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct WorldLightFader : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<bool> active;
  Value<float> fadedLevel;
  Value<float> intialLevel;
};
} // namespace DataSpec::DNAMP1
