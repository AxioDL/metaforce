#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct PathCamera : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> active;
  struct CameraParameters : BigDNA {
    AT_DECL_DNA
    Value<atUint32> propertyCount;
    Value<bool> closedLoop;
    Value<bool> noFilter;
    Value<bool> tangentOrientation;
    Value<bool> easeDist;
    Value<bool> useHintLookZ;
    Value<bool> clampToClosedDoor;
  } cameraParameters;

  Value<float> lengthExtent;
  Value<float> filterMag;
  Value<float> filterProportion;
  Value<atUint32> initPos;
  Value<float> minEaseDist;
  Value<float> maxEaseDist;
};
} // namespace DataSpec::DNAMP1
