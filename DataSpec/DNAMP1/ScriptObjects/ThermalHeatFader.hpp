#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct ThermalHeatFader : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<bool> active;
  Value<float> fadedLevel;
  Value<float> initialLevel;
};
} // namespace DataSpec::DNAMP1
