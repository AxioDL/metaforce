#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct PlayerStateChange : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<bool> active;
  Value<atUint32> item;
  Value<atUint32> itemCount;
  Value<atUint32> itemCapacity;
  Value<atUint32> control;
  Value<atUint32> controlCommandOption;
};
} // namespace DataSpec::DNAMP1
