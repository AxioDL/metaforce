#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Generator : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atUint32> spawnCount;
  Value<bool> noReuseFollowers;
  Value<bool> noInheritXf;
  Value<atVec3f> offset;
  Value<bool> active;
  Value<float> minScale;
  Value<float> maxScale;
};
} // namespace DataSpec::DNAMP1
