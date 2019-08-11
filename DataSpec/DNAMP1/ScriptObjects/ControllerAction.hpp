#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct ControllerAction : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<bool> active;
  Value<atUint32> command;
  Value<bool> deactivateOnClose;
};
} // namespace DataSpec::DNAMP1
