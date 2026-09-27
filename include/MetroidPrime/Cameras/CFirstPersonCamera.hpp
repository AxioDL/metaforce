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

  void SetScriptPitchId(TUniqueId uid) { mPitchId = uid; }
  void UpdateElevation(CStateManager& mgr);
  void UpdateTransform(CStateManager& mgr, float dt);
  const CTransform4f& GetGunFollowTransform() const;
  void SetLockCamera(bool lock) { mLockCamera = lock; }
  void CancelCinematicOffset();
  void DeferBallTransitionProcessing() { mDeferBallTransitionProcessing = true; }

private:
  float mOrbitCameraSpeed;
  bool mLockCamera;
  CTransform4f mGunFollowXf;
  float mPitch;
  TUniqueId mPitchId;
  bool mDeferBallTransitionProcessing : 1;
  CVector3f mCloseInVec;
  float mCloseInTimer;
};
CHECK_SIZEOF(CFirstPersonCamera, (VERSION >= VERSION_GM8E_02 ? 0x1e8 : 0x1d8))

#endif // _CFIRSTPERSONCAMERA
