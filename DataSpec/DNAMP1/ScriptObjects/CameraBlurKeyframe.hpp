#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct CameraBlurKeyframe : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<bool> active;
  Value<atUint32> btype;
  Value<float> amount;
  Value<atUint32> unk;
  Value<float> timeIn;
  Value<float> timeOut;
};
} // namespace DataSpec::DNAMP1
