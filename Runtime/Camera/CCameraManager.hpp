#pragma once

#include <list>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/World/CGameArea.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CBallCamera;
class CCameraShakeData;
class CCinematicCamera;
class CFirstPersonCamera;
class CGameCamera;
class CInterpolationCamera;
class CScriptCameraHint;
class CScriptWater;
class CStateManager;

struct CFinalInput;

class CCameraManager {
  static float sFirstPersonFOV;
  TUniqueId x0_curCameraId;
  std::vector<TUniqueId> x4_cineCameras;
  std::list<CCameraShakeData> x14_shakers;
  u32 x2c_lastShakeId = 0;
  zeus::CVector3f x30_shakeOffset;
  CGameArea::CAreaFog x3c_fog;
  int x74_fluidCounter = 0;
  TUniqueId x78_fluidId = kInvalidUniqueId;
  CFirstPersonCamera* x7c_fpCamera = nullptr;
  CBallCamera* x80_ballCamera = nullptr;
  s16 x84_rumbleId = -1;
  CInterpolationCamera* x88_interpCamera = nullptr;
  float x90_rumbleCooldown = 0.f;
  float x94_fogDensityFactor = 1.f;
  float x98_fogDensitySpeed = 0.f;
  float x9c_fogDensityFactorTarget = 1.f;
  bool xa0_24_pendingRumble : 1 = false;
  bool xa0_25_rumbling : 1 = false;
  bool xa0_26_inWater : 1 = false;
  TUniqueId xa2_spindleCamId = kInvalidUniqueId;
  TUniqueId xa4_pathCamId = kInvalidUniqueId;
  TUniqueId xa6_camHintId = kInvalidUniqueId;
  s32 xa8_hintPriority = 1000;
  rstl::reserved_vector<std::pair<s32, TUniqueId>, 64> xac_cameraHints;
  rstl::reserved_vector<TUniqueId, 64> x2b0_inactiveCameraHints;
  rstl::reserved_vector<TUniqueId, 64> x334_activeCameraHints;
  bool x3b8_24_ : 1 = false;
  bool x3b8_25_ : 1 = false;
  float x3bc_curFov = 60.f;

  void SetPathCamera(TUniqueId id, CStateManager& mgr);
  void SetSpindleCamera(TUniqueId id, CStateManager& mgr);
  void RestoreHintlessCamera(CStateManager& mgr);
  void InterpolateToBallCamera(const zeus::CTransform& xf, TUniqueId camId, const zeus::CVector3f& lookPos,
                               float maxTime, float positionSpeed, float rotationSpeed, bool sinusoidal,
                               CStateManager& mgr);
  void SkipBallCameraCinematic(CStateManager& mgr);
  void ApplyCameraHint(const CScriptCameraHint& hint, CStateManager& mgr);

  void EnterCinematic(CStateManager& mgr);

public:
  explicit CCameraManager(TUniqueId curCameraId = kInvalidUniqueId);

  static float Aspect() { return 1.42f; }
  static float FarPlane() { return 750.0f; }
  static float NearPlane() { return 0.2f; }
  static float FirstPersonFOV() { return sFirstPersonFOV; }
  static float ThirdPersonFOV() { return 60.0f; }

  void ResetCameras(CStateManager& mgr);
  void SetSpecialCameras(CFirstPersonCamera& fp, CBallCamera& ball);
  bool IsInCinematicCamera() const { return x4_cineCameras.size() != 0; }
  bool IsInFirstPersonCamera() const;
  zeus::CVector3f GetGlobalCameraTranslation(const CStateManager& stateMgr) const;
  zeus::CTransform GetCurrentCameraTransform(const CStateManager& stateMgr) const;
  void RemoveCameraShaker(u32 id);
  int AddCameraShaker(const CCameraShakeData& data, bool sfx);
  void AddCinemaCamera(TUniqueId id, CStateManager& stateMgr);
  void RemoveCinemaCamera(TUniqueId uid, CStateManager& mgr);
  void SetInsideFluid(bool isInside, TUniqueId fluidId);
  void Update(float dt, CStateManager& stateMgr);
  CGameCamera* GetCurrentCamera(CStateManager& stateMgr) const;
  const CGameCamera* GetCurrentCamera(const CStateManager& stateMgr) const;
  void SetCurrentCameraId(TUniqueId id, CStateManager& stateMgr) { x0_curCameraId = id; }
  void CreateStandardCameras(CStateManager& stateMgr);
  TUniqueId GetCurrentCameraId() const {
    if (x4_cineCameras.size())
      return x4_cineCameras.back();
    return x0_curCameraId;
  }
  TUniqueId GetLastCameraId() const {
    if (x4_cineCameras.size())
      return x4_cineCameras.back();
    return kInvalidUniqueId;
  }

  void SkipCinematic(CStateManager& stateMgr);

  CFirstPersonCamera* GetFirstPersonCamera() { return x7c_fpCamera; }
  const CFirstPersonCamera* GetFirstPersonCamera() const { return x7c_fpCamera; }

  CBallCamera* GetBallCamera() { return x80_ballCamera; }
  const CBallCamera* GetBallCamera() const { return x80_ballCamera; }

  CGameArea::CAreaFog& Fog() { return x3c_fog; }
  const CGameArea::CAreaFog& Fog() const { return x3c_fog; }

  float GetCameraBobMagnitude() const;

  void UpdateCameraHints(float dt, CStateManager& mgr);
  void ThinkCameras(float dt, CStateManager& mgr);
  void UpdateFog(float dt, CStateManager& mgr);
  void UpdateRumble(float dt, CStateManager& mgr);
  void UpdateListener(CStateManager& mgr);

  float CalculateFogDensity(CStateManager& mgr, const CScriptWater* water) const;
  void SetFogDensity(float fogDensityTarget, float fogDensitySpeed);

  void ProcessInput(const CFinalInput& input, CStateManager& stateMgr);

  void RenderCameras(CStateManager& mgr);
  void SetupBallCamera(CStateManager& mgr);
  void SetPlayerCamera(CStateManager& mgr, TUniqueId newCamId);
  int GetFluidCounter() const { return x74_fluidCounter; }
  bool HasBallCameraInitialPositionHint(CStateManager& mgr) const;

  void DeleteCameraHint(TUniqueId id, CStateManager& mgr);
  void AddInactiveCameraHint(TUniqueId id, CStateManager& mgr);
  void AddActiveCameraHint(TUniqueId id, CStateManager& mgr);

  TUniqueId GetLastCineCameraId() const;
  TUniqueId GetSpindleCameraId() const { return xa2_spindleCamId; }
  TUniqueId GetPathCameraId() const { return xa4_pathCamId; }
  const CCinematicCamera* GetLastCineCamera(CStateManager& mgr) const;
  const CScriptCameraHint* GetCameraHint(CStateManager& mgr) const;
  bool HasCameraHint(CStateManager& mgr) const;
  bool IsInterpolationCameraActive() const;

  bool ShouldBypassInterpolation() { return false; }
};

} // namespace urde
