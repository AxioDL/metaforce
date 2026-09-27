#ifndef _CCINEMATICCAMERA
#define _CCINEMATICCAMERA

#include "types.h"

#include "MetroidPrime/Cameras/CGameCamera.hpp"

class CCinematicCamera : public CGameCamera {
public:
  CCinematicCamera(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, bool active, const float shotDuration, const float fovy,
                   const float znear, const float zfar, const float aspect, const uint flags);
  ~CCinematicCamera();
  // CActor
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CGameCamera

  void ProcessInput(const CFinalInput&, CStateManager& mgr) override;
  void Reset(const CTransform4f&, CStateManager& mgr) override;

  // CCinematicCamera
  u32 GetFlags() const { return mFlags; }
  void WasDeactivated(CStateManager& mgr);
  void SendArrivedMsg(TUniqueId reciever, CStateManager& mgr) const;
  float GetDuration() const { return mDuration; }
  CVector3f GetInterpolatedSplinePoint(const rstl::vector< CVector3f >& points, int& idxOut,
                                       float t) const;
  CQuaternion GetInterpolatedOrientation(const rstl::vector< CQuaternion >& rotations,
                                         float t) const;
  float GetInterpolatedHFov(const rstl::vector< float >& fovs, float t) const;
  float GetMoveOutofIntoAlpha() const;
  void DeactivateSelf(CStateManager& mgr);
  void CalculateMoveOutofIntoEyePosition(bool outOfEye, CStateManager& mgr);
  void GenerateMoveOutofIntoPoints(bool outOfEye, CStateManager& mgr);
  static bool PickRandomActiveConnection(const rstl::vector< SConnection >& conns,
                                         SConnection& randConn, CStateManager& mgr);
  void CalculateWaypoints(CStateManager& mgr);

private:
  rstl::vector< CVector3f > mViewPoints;
  rstl::vector< CQuaternion > mViewOrientations;
  rstl::vector< TUniqueId > mViewPointArrivals;
  rstl::vector< CVector3f > mTargets;
  rstl::vector< TUniqueId > mTargetArrivals;
  rstl::vector< float > mViewHFovs;
  float mDuration;
  float mT;
  float mOrigFovy;
  int mPassedViewPoint;
  int mPassedTarget;
  CQuaternion mOrigOrientation;
  TUniqueId mLookAtId;
  CVector3f mMoveIntoEyePos;
  u32 mFlags; // 0x1: look at player
                  // 0x2: out of player eye
                  // 0x4: into player eye
                  // 0x10: finish cine skip
                  // 0x20: disable input
                  // 0x40: draw player
                  // 0x80: check failsafe
                  // 0x100: cinematic pause
                  // 0x200: disable out of into
  bool x220_24_ : 1;
};
CHECK_SIZEOF(CCinematicCamera, (VERSION >= VERSION_GM8E_02 ? 0x238 : 0x228))

#endif // _CCINEMATICCAMERA
