#ifndef _CSCRIPTSPINDLECAMERA
#define _CSCRIPTSPINDLECAMERA

#include "types.h"

#include "MetroidPrime/Cameras/CGameCamera.hpp"

#include "rstl/reserved_vector.hpp"

class CInputStream;

enum ESpindleInput {
  kSI_Constant,
  kSI_HintToBallDist,
  kSI_HintToBallVOff,
  kSI_HintBallAngle,
  kSI_HintBallRightAngle,
  kSI_HintBallLeftAngle,
  kSI_HintDeltaDist,
  kSI_HintDeltaVOff,
};

struct CSpindleCameraInterpolant {
  uint x0_flags;
  ESpindleInput x4_input;
  float x8_lowOut;
  float xc_highOut;
  float x10_lowIn;
  float x14_highIn;

  CSpindleCameraInterpolant() {}
  CSpindleCameraInterpolant(ESpindleInput input, uint flags, float lowOut, float highOut,
                            float lowIn, float highIn);

  void ConvertToRadians();
  float InterpolateValue(float inVar) const;
};
CHECK_SIZEOF(CSpindleCameraInterpolant, 0x18)

CSpindleCameraInterpolant LoadSpindleSegment(CInputStream& in);

class CScriptSpindleCamera : public CGameCamera {
public:
  CScriptSpindleCamera(
      TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
      bool active, int flags, float hintToCamDistMin, float hintToCamDistMax,
      float hintToCamVOffMin, float hintToCamVOffMax,
      CSpindleCameraInterpolant targetHintToCamDeltaAngleVel,
      CSpindleCameraInterpolant deltaAngleScaleWithCamDist, CSpindleCameraInterpolant hintToCamDist,
      CSpindleCameraInterpolant distOffsetFromBallDist,
      CSpindleCameraInterpolant hintBallToCamAzimuth, CSpindleCameraInterpolant unused,
      CSpindleCameraInterpolant maxHintBallToCamAzimuth,
      CSpindleCameraInterpolant camLookRelAzimuth, CSpindleCameraInterpolant lookPosZOffset,
      CSpindleCameraInterpolant camPosZOffset, CSpindleCameraInterpolant clampedAzimuthFromHintDir,
      CSpindleCameraInterpolant dampingAzimuthSpeed,
      CSpindleCameraInterpolant targetHintToCamDeltaAngleVelRange,
      CSpindleCameraInterpolant deleteHintBallDist,
      CSpindleCameraInterpolant recoverClampedAzimuthFromHintDir);

  // CEntity
  ~CScriptSpindleCamera() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

private:
  float GetInterpolant(const CSpindleCameraInterpolant& seg) const;

  uint x188_flags;
  rstl::reserved_vector< float, 8 > x18c_inVars;
  float x1b0_hintToCamDistMin;
  float x1b4_hintToCamDistMax;
  float x1b8_hintToCamVOffMin;
  float x1bc_hintToCamVOffMax;
  CSpindleCameraInterpolant x1c0_targetHintToCamDeltaAngleVel;
  CSpindleCameraInterpolant x1d8_deltaAngleScaleWithCamDist;
  CSpindleCameraInterpolant x1f0_hintToCamDist;
  CSpindleCameraInterpolant x208_distOffsetFromBallDist;
  CSpindleCameraInterpolant x220_hintBallToCamAzimuth;
  CSpindleCameraInterpolant x238_unused;
  CSpindleCameraInterpolant x250_maxHintBallToCamAzimuth;
  CSpindleCameraInterpolant x268_camLookRelAzimuth;
  CSpindleCameraInterpolant x280_lookPosZOffset;
  CSpindleCameraInterpolant x298_camPosZOffset;
  CSpindleCameraInterpolant x2b0_clampedAzimuthFromHintDir;
  CSpindleCameraInterpolant x2c8_dampingAzimuthSpeed;
  CSpindleCameraInterpolant x2e0_targetHintToCamDeltaAngleVelRange;
  CSpindleCameraInterpolant x2f8_deleteHintBallDist;
  CSpindleCameraInterpolant x310_recoverClampedAzimuthFromHintDir;
  float x328_maxAzimuthInterpTimer;
  bool x32c_24_outsideClampedAzimuth : 1;
  CVector3f x330_lookDir;
  bool x33c_24_inResetThink : 1;
};
CHECK_SIZEOF(CScriptSpindleCamera, 0x340)

#endif // _CSCRIPTSPINDLECAMERA
