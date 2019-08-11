#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Relay : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<bool> active;
};
} // namespace DataSpec::DNAMP1
