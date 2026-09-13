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
  u32 GetFlags() const { return x21c_flags; }
  void WasDeactivated(CStateManager& mgr);
  void SendArrivedMsg(TUniqueId reciever, CStateManager& mgr) const;
  float GetDuration() const { return x1e8_duration; }
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
  rstl::vector< CVector3f > x188_viewPoints;
  rstl::vector< CQuaternion > x198_viewOrientations;
  rstl::vector< TUniqueId > x1a8_viewPointArrivals;
  rstl::vector< CVector3f > x1b8_targets;
  rstl::vector< TUniqueId > x1c8_targetArrivals;
  rstl::vector< float > x1d8_viewHFovs;
  float x1e8_duration;
  float x1ec_t;
  float x1f0_origFovy;
  int x1f4_passedViewPoint;
  int x1f8_passedTarget;
  CQuaternion x1fc_origOrientation;
  TUniqueId x20c_lookAtId;
  CVector3f x210_moveIntoEyePos;
  u32 x21c_flags; // 0x1: look at player
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
CHECK_SIZEOF(CCinematicCamera, 0x228)

#endif // _CCINEMATICCAMERA
