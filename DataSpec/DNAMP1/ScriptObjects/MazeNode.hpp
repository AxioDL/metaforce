#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct MazeNode : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> unknown1;
  Value<atUint32> unknown2;
  Value<atUint32> unknown3;
  Value<atUint32> unknown4;
  Value<atVec3f> unknown5;
  Value<atVec3f> unknown6;
  Value<atVec3f> unknown7;
};
} // namespace DataSpec::DNAMP1
