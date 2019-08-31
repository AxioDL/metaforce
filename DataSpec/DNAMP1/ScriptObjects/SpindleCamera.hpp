#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {

struct SpindleCamera : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> active;
  /*
   * 0x1: Look toward hint
   * 0x2: Flat look delta
   * 0x8: force minimum-clamp ball-to-cam azimuth
   * 0x10: minimum-clamp ball-to-cam azimuth
   * 0x20: Enable clampedAzimuthFromHintDir
   * 0x40: Enable distOffsetFromBallDist
   * 0x80: Use ball pos for cam pos Z (vs. hint pos)
   * 0x100: Enable deltaAngleScaleWithCamDist
   * 0x200: Use ball pos for look pos Z (vs. hint pos)
   * 0x400: unused
   * 0x800: Variable hint-to-ball direction
   * 0x1000: Damp look azimuth with hint ball-to-cam azimuth < 10-degrees
   * 0x2000: Enable deleteHintBallDist
   * 0x4000: Ignore ball-to-cam azimuth sign
   */
  PropertyFlags flags;
  Value<float> hintToCamDistMin;
  Value<float> hintToCamDistMax;
  Value<float> hintToCamVOffMin;
  Value<float> hintToCamVOffMax;
  struct SpindleCameraParameters : BigDNA {
    AT_DECL_DNA_YAML
    Value<atUint32> input;
    PropertyFlags flags; // high reflect, low reflect
    Value<float> lowOut;
    Value<float> highOut;
    Value<float> lowIn;
    Value<float> highIn;
  };
  SpindleCameraParameters targetHintToCamDeltaAngleVel;
  SpindleCameraParameters deltaAngleScaleWithCamDist;
  SpindleCameraParameters hintToCamDist;
  SpindleCameraParameters distOffsetFromBallDist;
  SpindleCameraParameters hintBallToCamAzimuth;
  SpindleCameraParameters unused;
  SpindleCameraParameters maxHintBallToCamAzimuth;
  SpindleCameraParameters camLookRelAzimuth;
  SpindleCameraParameters lookPosZOffset;
  SpindleCameraParameters camPosZOffset;
  SpindleCameraParameters clampedAzimuthFromHintDir;
  SpindleCameraParameters dampingAzimuthSpeed;
  SpindleCameraParameters targetHintToCamDeltaAngleVelRange;
  SpindleCameraParameters deleteHintBallDist;
  SpindleCameraParameters recoverClampedAzimuthFromHintDir;
};

} // namespace DataSpec::DNAMP1
