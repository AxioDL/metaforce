#ifndef _CFIRSTPERSONCAMERA
#define _CFIRSTPERSONCAMERA

#include "MetroidPrime/Cameras/CGameCamera.hpp"

class CFirstPersonCamera : public CGameCamera {
public:
  CFirstPersonCamera(const TUniqueId& uid, const CTransform4f& xf, TUniqueId watchedObj,
                     float orbitCameraSpeed, float fov, float nearz, float farz, float aspect);

  // CEntity
  ~CFirstPersonCamera() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager&) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

  void SetScriptPitchId(TUniqueId uid) { x1c4_pitchId = uid; }
  void UpdateElevation(CStateManager& mgr);
  void UpdateTransform(CStateManager& mgr, float dt);
  const CTransform4f& GetGunFollowTransform() const;
  void SetLockCamera(bool lock) { x18c_lockCamera = lock; }
  void CancelCinematicOffset();
  void DeferBallTransitionProcessing() { x1c6_24_deferBallTransitionProcessing = true; }

private:
  float x188_orbitCameraSpeed;
  bool x18c_lockCamera;
  CTransform4f x190_gunFollowXf;
  float x1c0_pitch;
  TUniqueId x1c4_pitchId;
  bool x1c6_24_deferBallTransitionProcessing : 1;
  CVector3f x1c8_closeInVec;
  float x1d4_closeInTimer;
};
CHECK_SIZEOF(CFirstPersonCamera, (VERSION >= VERSION_GM8P_00 ? 0x1e8 : 0x1d8))

#endif // _CFIRSTPERSONCAMERA
