#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Dock : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<bool> active;
  Value<atVec3f> location;
  Value<atVec3f> volume;
  Value<atUint32> dock;
  Value<atUint32> room;
  Value<bool> loadConnected;
};
} // namespace DataSpec::DNAMP1
