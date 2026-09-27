#ifndef _CCAMERAMANAGER
#define _CCAMERAMANAGER

#include "types.h"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CBallCamera;
class CCameraShakeData;
class CFinalInput;
class CFirstPersonCamera;
class CGameCamera;
class CInterpolationCamera;
class CScriptCameraHint;
class CScriptWater;
class CStateManager;

class CCameraManager {
  static float sFirstPersonFOV;
  static float sThirdPersonFOV;
  static float sNearPlane;
  static float sFarPlane;
  static float sAspectRatio;
  static float sMinShakeVolume;
  static float sMaxShakeVolume;

public:
  CCameraManager(TUniqueId curCamera = kInvalidUniqueId);

  void CreateCameras(CStateManager& mgr);
  void UpdateCameras(float dt, CStateManager& mgr);
  void ResetCameras(CStateManager& mgr);
  void Render(const CStateManager& mgr);
  TUniqueId GetCurrentCameraId() const;
  void SetCurrentCameraId(TUniqueId camId);

  CGameCamera& CurrentCamera(CStateManager& mgr);
  const CGameCamera& GetCurrentCamera(const CStateManager& mgr) const;
  const CGameCamera& GetCurrentCinematicCamera(CStateManager& mgr) const;
  TUniqueId GetCurrentCinematicCameraId() const;

  void Update(float dt, CStateManager& mgr);
  void UpdateAudioListener(CStateManager& mgr);
  void UpdateScreenShake(float dt, CStateManager& mgr);
  void UpdateFilters(float dt, CStateManager& mgr);
  void UpdateCameraHints(float dt, CStateManager& mgr);
  float GetWaterFarDistance(CStateManager& mgr, const CScriptWater* water);
  void SetWaterFogScale(float fogDensityTarget, float fogDensitySpeed);

  CFirstPersonCamera* FirstPersonCamera() const /* map */ { return mFpCamera; }
  const CFirstPersonCamera* GetFirstPersonCamera() const { return mFpCamera; }

  void SetPlayerCamera(CStateManager& mgr, TUniqueId newCamId);
  bool IsInCinematicCamera() const;
  float GetCameraBobMagnitude() const;
  void AddCinemaCamera(TUniqueId uid, CStateManager& mgr);
  void RemoveCinemaCamera(TUniqueId uid, CStateManager& mgr);
  void EnterCinematic(CStateManager& mgr);
  void StopCinematics(CStateManager& mgr);

  int AddCameraShaker(const CCameraShakeData& data, bool sfx);
  void RemoveCameraShaker(int id);
  CTransform4f GetCurrentCameraTransform(const CStateManager& mgr) const;
  CVector3f GetGlobalCameraTranslation(const CStateManager& mgr) const;
  bool IsInFPCamera() const;
  bool IsInterpolationCameraActive() const;
  bool ShouldBypassInterpolationCamera() const;
  void SetupInterpolation(const CTransform4f& xf, TUniqueId camId, CVector3f lookPos,
                               float maxTime, float positionSpeed, float rotationSpeed,
                               bool sinusoidal, CStateManager& mgr);
  void CinematicCut(CStateManager& mgr);
  void UseCameraHint(const CScriptCameraHint& hint, CStateManager& mgr);
  const CScriptCameraHint* GetCameraHint(CStateManager& mgr) const;
  void NoCameraHintsLeft(CStateManager& mgr);
  void AddCameraHint(TUniqueId uid, CStateManager& mgr);
  void DeleteCameraHint(TUniqueId uid, CStateManager& mgr);
  void ReallyRemoveCameraHint(TUniqueId uid, CStateManager& mgr);
  // SetSpecialCameras__14CCameraManagerFR18CFirstPersonCameraR11CBallCamera
  void ResetCameraHint(CStateManager& mgr);
  bool HasCameraHint(CStateManager& mgr) const;
  bool HasBallCameraInitialPositionHint(CStateManager& mgr) const;

  void SetCurrentFov(float fov) { mCurFov = fov; }

  CGameArea::CAreaFog& Fog() { return mFog; }
  const CGameArea::CAreaFog& GetFog() const { return mFog; }

  int GetFluidCounter() const { return mFluidCounter; }
  TUniqueId GetFluidId() const { return mFluidId; }
  bool GetInsideFluid() const { return mFluidCounter == 0; }
  bool IsInsideFluid() const { return mInWater; }
  // WasInsideFluid__14CCameraManagerCFv
  // SetWasInsideFluid__14CCameraManagerFb
  void SetInsideFluid(bool isInside, TUniqueId fluidId);

  void ProcessInput(const CFinalInput& input, CStateManager& mgr);

  void SetPathCamera(TUniqueId id, CStateManager& mgr);
  TUniqueId GetPathCameraId() const;
  void SetSpindleCamera(TUniqueId id, CStateManager& mgr);
  TUniqueId GetSpindleCameraId() const;

  CBallCamera* BallCamera() const { return mBallCamera; }
  const CBallCamera* GetBallCamera() const { return mBallCamera; }

  static float GetDefaultThirdPersonVerticalFOV();
  static float GetDefaultFirstPersonVerticalFOV();
  static float GetDefaultFirstPersonNearClipDistance();
  static float GetDefaultFirstPersonFarClipDistance();
  static float GetDefaultAspectRatio();

private:
  TUniqueId mCurCameraId;
  rstl::vector< TUniqueId > mCineCameras;
  rstl::list< CCameraShakeData > mShakers;
  uint mLastShakeId;
  CVector3f mShakeOffset;
  CGameArea::CAreaFog mFog;
  int mFluidCounter;
  TUniqueId mFluidId;
  CFirstPersonCamera* mFpCamera;
  CBallCamera* mBallCamera;
  int mRumbleId;
  CInterpolationCamera* mInterpCamera;
  short x8c_;
  float mRumbleCooldown;
  float mFogDensityFactor;
  float mFogDensitySpeed;
  float mFogDensityFactorTarget;
  bool mPendingRumble : 1;
  bool mRumbling : 1;
  bool mInWater : 1;

  TUniqueId mSpindleCamId;
  TUniqueId mPathCamId;
  TUniqueId mCamHintId;
  int mHintPriority;
  rstl::reserved_vector< rstl::pair< int, TUniqueId >, 64 > mCameraHints;
  rstl::reserved_vector< TUniqueId, 64 > mInactiveCameraHints;
  rstl::reserved_vector< TUniqueId, 64 > mActiveCameraHints;
  bool x3b8_24_ : 1;
  bool x3b8_25_ : 1;
  float mCurFov;
};
CHECK_SIZEOF(CCameraManager, 0x3c0)

#endif // _CCAMERAMANAGER
