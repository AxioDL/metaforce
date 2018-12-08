#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct RandomRelay : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atUint32> unknown1;
  Value<atUint32> unknown2;
  Value<bool> unknown3;
  Value<bool> unknown4;
};
} // namespace DataSpec::DNAMP1
