#pragma once

#include "Camera/CGameCamera.hpp"

namespace urde {

enum class ESpindleInput {
  Constant,
  HintToBallDist,
  HintToBallVOff,
  HintBallAngle,
  HintBallRightAngle,
  HintBallLeftAngle,
  HintDeltaDist,
  HintDeltaVOff
};

struct SSpindleProperty {
  u32 x0_flags;
  ESpindleInput x4_input;
  float x8_lowOut;
  float xc_highOut;
  float x10_lowIn;
  float x14_highIn;

  SSpindleProperty(CInputStream& in);
  void FixupAngles() {
    x8_lowOut = zeus::degToRad(x8_lowOut);
    xc_highOut = zeus::degToRad(xc_highOut);
  }
  float GetValue(float inVar) const;
};

class CScriptSpindleCamera : public CGameCamera {
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
  u32 x188_flags;
  rstl::reserved_vector<float, 8> x18c_inVars;
  float x1b0_hintToCamDistMin;
  float x1b4_hintToCamDistMax;
  float x1b8_hintToCamVOffMin;
  float x1bc_hintToCamVOffMax;
  SSpindleProperty x1c0_targetHintToCamDeltaAngleVel;
  SSpindleProperty x1d8_deltaAngleScaleWithCamDist;
  SSpindleProperty x1f0_hintToCamDist;
  SSpindleProperty x208_distOffsetFromBallDist;
  SSpindleProperty x220_hintBallToCamAzimuth;
  SSpindleProperty x238_unused;
  SSpindleProperty x250_maxHintBallToCamAzimuth;
  SSpindleProperty x268_camLookRelAzimuth;
  SSpindleProperty x280_lookPosZOffset;
  SSpindleProperty x298_camPosZOffset;
  SSpindleProperty x2b0_clampedAzimuthFromHintDir;
  SSpindleProperty x2c8_dampingAzimuthSpeed;
  SSpindleProperty x2e0_targetHintToCamDeltaAngleVelRange;
  SSpindleProperty x2f8_deleteHintBallDist;
  SSpindleProperty x310_recoverClampedAzimuthFromHintDir;
  float x328_maxAzimuthInterpTimer = 0.f;
  bool x32c_outsideClampedAzimuth = false;
  zeus::CVector3f x330_lookDir;
  bool x33c_24_inResetThink;

  float GetInVar(const SSpindleProperty& seg) const { return x18c_inVars[int(seg.x4_input)]; }

public:
  CScriptSpindleCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                       const zeus::CTransform& xf, bool active, u32 flags, float hintToCamDistMin,
                       float hintToCamDistMax, float hintToCamVOffMin, float hintToCamVOffMax,
                       const SSpindleProperty& targetHintToCamDeltaAngleVel,
                       const SSpindleProperty& deltaAngleScaleWithCamDist,
                       const SSpindleProperty& hintToCamDist,
                       const SSpindleProperty& distOffsetFromBallDist,
                       const SSpindleProperty& hintBallToCamAzimuth,
                       const SSpindleProperty& unused,
                       const SSpindleProperty& maxHintBallToCamAzimuth,
                       const SSpindleProperty& camLookRelAzimuth,
                       const SSpindleProperty& lookPosZOffset,
                       const SSpindleProperty& camPosZOffset,
                       const SSpindleProperty& clampedAzimuthFromHintDir,
                       const SSpindleProperty& dampingAzimuthSpeed,
                       const SSpindleProperty& targetHintToCamDeltaAngleVelRange,
                       const SSpindleProperty& deleteHintBallDist,
                       const SSpindleProperty& recoverClampedAzimuthFromHintDir);

  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void Render(const CStateManager&) const;
  void Reset(const zeus::CTransform& xf, CStateManager& mgr);
  void ProcessInput(const CFinalInput& input, CStateManager& mgr);
};

} // namespace urde
