#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct RandomRelay : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atUint32> sendSetSize;
  Value<atUint32> sendSetVariance;
  Value<bool> percentSize;
  Value<bool> active;
};
} // namespace DataSpec::DNAMP1
