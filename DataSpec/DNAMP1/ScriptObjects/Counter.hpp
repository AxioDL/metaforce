#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Counter : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atUint32> initial;
  Value<atUint32> maxValue;
  Value<bool> autoReset;
  Value<bool> active;
};
} // namespace DataSpec::DNAMP1
