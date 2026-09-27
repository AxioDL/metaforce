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
  uint mFlags;
  ESpindleInput mInput;
  float mLowOut;
  float mHighOut;
  float mLowIn;
  float mHighIn;

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

  uint mFlags;
  rstl::reserved_vector< float, 8 > mInVars;
  float mHintToCamDistMin;
  float mHintToCamDistMax;
  float mHintToCamVOffMin;
  float mHintToCamVOffMax;
  CSpindleCameraInterpolant mTargetHintToCamDeltaAngleVel;
  CSpindleCameraInterpolant mDeltaAngleScaleWithCamDist;
  CSpindleCameraInterpolant mHintToCamDist;
  CSpindleCameraInterpolant mDistOffsetFromBallDist;
  CSpindleCameraInterpolant mHintBallToCamAzimuth;
  CSpindleCameraInterpolant mUnused;
  CSpindleCameraInterpolant mMaxHintBallToCamAzimuth;
  CSpindleCameraInterpolant mCamLookRelAzimuth;
  CSpindleCameraInterpolant mLookPosZOffset;
  CSpindleCameraInterpolant mCamPosZOffset;
  CSpindleCameraInterpolant mClampedAzimuthFromHintDir;
  CSpindleCameraInterpolant mDampingAzimuthSpeed;
  CSpindleCameraInterpolant mTargetHintToCamDeltaAngleVelRange;
  CSpindleCameraInterpolant mDeleteHintBallDist;
  CSpindleCameraInterpolant mRecoverClampedAzimuthFromHintDir;
  float mMaxAzimuthInterpTimer;
  bool mOutsideClampedAzimuth : 1;
  CVector3f mLookDir;
  bool mInResetThink : 1;
};
CHECK_SIZEOF(CScriptSpindleCamera, (VERSION >= VERSION_GM8E_02 ? 0x350 : 0x340))

#endif // _CSCRIPTSPINDLECAMERA
