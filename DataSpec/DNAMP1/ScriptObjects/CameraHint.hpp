#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct CameraHint : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> active;
  Value<atUint32> priority;
  Value<atUint32> behaviour;
  struct CameraHintParameters : BigDNA {
    AT_DECL_DNA
    Value<atUint32> propertyCount;
    Value<bool> calculateCamPos;                 // 0x1
    Value<bool> chaseAllowed;                    // 0x2
    Value<bool> boostAllowed;                    // 0x4
    Value<bool> obscureAvoidance;                // 0x8
    Value<bool> volumeCollider;                  // 0x10
    Value<bool> applyImmediately;                // 0x20
    Value<bool> lookAtBall;                      // 0x40
    Value<bool> hintDistanceSelection;           // 0x80
    Value<bool> hintDistanceSelfPos;             // 0x100
    Value<bool> controlInterpolation;            // 0x200
    Value<bool> sinusoidalInterpolation;         // 0x400
    Value<bool> sinusoidalInterpolationHintless; // 0x800
    Value<bool> clampVelocity;                   // 0x1000
    Value<bool> skipCinematic;                   // 0x2000
    Value<bool> noElevationInterp;               // 0x4000
    Value<bool> directElevation;                 // 0x8000
    Value<bool> overrideLookDir;                 // 0x10000
    Value<bool> noElevationVelClamp;             // 0x20000
    Value<bool> calculateTransformFromPrevCam;   // 0x40000
    Value<bool> noSpline;                        // 0x80000
    Value<bool> unknown21;                       // 0x100000
    Value<bool> unknown22;                       // 0x200000
  } cameraHintParameters;

  struct BoolFloat : BigDNA {
    AT_DECL_DNA
    Value<bool> active;
    Value<float> value;
  } minDist, maxDist, backwardsDist; // 0x400000, 0x800000, 0x1000000
  struct BoolVec3f : BigDNA {
    AT_DECL_DNA
    Value<bool> active;
    Value<atVec3f> value;
  } lookAtOffset, chaseLookAtOffset; // 0x2000000, 0x4000000
  Value<atVec3f> ballToCam;
  BoolFloat fov, attitudeRange, azimuthRange, anglePerSecond; // 0x8000000, 0x10000000, 0x20000000, 0x40000000
  Value<float> clampVelRange;
  Value<float> clampRotRange;
  BoolFloat elevation; // 0x80000000
  Value<float> interpolateTime;
  Value<float> clampVelTime;
  Value<float> controlInterpDur;
};
} // namespace DataSpec::DNAMP1
