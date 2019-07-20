#pragma once

#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {

struct AIKeyframe : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atUint32> animationId;
  Value<bool> looping;
  Value<float> lifetime;
  Value<bool> active;
  Value<atUint32> fadeOut;
  Value<float> totalPlayback;
};
} // namespace DataSpec::DNAMP1
