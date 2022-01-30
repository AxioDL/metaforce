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
  Value<bool> active;
  Value<atUint32> col;
  Value<atUint32> row;
  Value<atUint32> side;
  Value<atVec3f> actorPos;
  Value<atVec3f> triggerPos;
  Value<atVec3f> effectPos;
};
} // namespace DataSpec::DNAMP1
