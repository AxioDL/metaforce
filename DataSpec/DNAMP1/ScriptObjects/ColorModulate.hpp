#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct ColorModulate : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec4f> colorA;
  Value<atVec4f> colorB;
  Value<atUint32> blendMode;
  Value<float> timeA2B;
  Value<float> timeB2A;
  Value<bool> doReverse;
  Value<bool> resetTargetWhenDone;
  Value<bool> depthCompare;
  Value<bool> depthUpdate;
  Value<bool> depthBackwards;
  Value<bool> active;
};
} // namespace DataSpec::DNAMP1
