#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Camera : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> active;
  Value<float> shotDuration;
  Value<bool> lookAtPlayer;
  Value<bool> outOfPlayerEye;
  Value<bool> intoPlayerEye;
  Value<bool> drawPlayer;
  Value<bool> disableInput;
  Value<bool> unknown;
  Value<bool> finishCineSkip;
  Value<float> fov;
  Value<bool> checkFailsafe;
  Value<bool> disableOutOfInto;
};
} // namespace DataSpec::DNAMP1
