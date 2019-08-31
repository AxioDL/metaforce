#pragma once

#include "CGameCamera.hpp"

namespace urde {

class CCinematicCamera : public CGameCamera {
  std::vector<zeus::CVector3f> x188_viewPoints;
  std::vector<zeus::CQuaternion> x198_viewOrientations;
  std::vector<TUniqueId> x1a8_viewPointArrivals;
  std::vector<zeus::CVector3f> x1b8_targets;
  std::vector<TUniqueId> x1c8_targetArrivals;
  std::vector<float> x1d8_viewHFovs;
  float x1e8_duration;
  float x1ec_t = 0.f;
  float x1f0_origFovy;
  int x1f4_passedViewPoint = 0;
  int x1f8_passedTarget = 0;
  zeus::CQuaternion x1fc_origOrientation;
  TUniqueId x20c_lookAtId = kInvalidUniqueId;
  zeus::CVector3f x210_moveIntoEyePos;
  u32 x21c_flags; // 0x1: look at player, 0x2: out of player eye, 0x4: into player eye, 0x10: finish cine skip,
                  // 0x20: disable input, 0x40: draw player, 0x80: check failsafe, 0x100: cinematic pause,
                  // 0x200: disable out of into
  bool x220_24_;
  zeus::CVector3f GetInterpolatedSplinePoint(const std::vector<zeus::CVector3f>& points, int& idxOut, float t) const;
  zeus::CQuaternion GetInterpolatedOrientation(const std::vector<zeus::CQuaternion>& rotations, float t) const;
  float GetInterpolatedHFov(const std::vector<float>& fovs, float t) const;
  float GetMoveOutofIntoAlpha() const;
  void DeactivateSelf(CStateManager& mgr);
  void CalculateMoveOutofIntoEyePosition(bool outOfEye, CStateManager& mgr);
  void GenerateMoveOutofIntoPoints(bool outOfEye, CStateManager& mgr);
  static bool PickRandomActiveConnection(const std::vector<SConnection>& conns, SConnection& randConn,
                                         CStateManager& mgr);
  void CalculateWaypoints(CStateManager& mgr);

public:
  CCinematicCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   bool active, float shotDuration, float fovy, float znear, float zfar, float aspect, u32 flags);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void ProcessInput(const CFinalInput&, CStateManager& mgr) override;
  void Reset(const zeus::CTransform&, CStateManager& mgr) override;
  u32 GetFlags() const { return x21c_flags; }
  void WasDeactivated(CStateManager& mgr);
  void SendArrivedMsg(TUniqueId reciever, CStateManager& mgr);
  float GetDuration() const { return x1e8_duration; }
};

} // namespace urde
