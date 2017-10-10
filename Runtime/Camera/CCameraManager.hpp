#ifndef __URDE_CCAMERAMANAGER_HPP__
#define __URDE_CCAMERAMANAGER_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "World/CGameArea.hpp"

namespace urde
{
class CFirstPersonCamera;
class CBallCamera;
class CStateManager;
class CGameCamera;
class CCameraShakeData;
class CScriptWater;
class CInterpolationCamera;
class CFinalInput;
class CScriptCameraHint;
class CCinematicCamera;

class CCameraManager
{
    static float sAspect;
    static float sFarPlane;
    static float sNearPlane;
    static float sFirstPersonFOV;
    static float sThirdPersonFOV;
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

    union
    {
        struct
        {
            bool xa0_24_pendingRumble : 1;
            bool xa0_25_rumbling : 1;
            bool xa0_26_inWater : 1;
        };
        u8 _dummy1 = 0;
    };

    TUniqueId xa2_spindleCamId = kInvalidUniqueId;
    TUniqueId xa4_pathCamId = kInvalidUniqueId;
    TUniqueId xa6_camHintId = kInvalidUniqueId;
    s32 xa8_hintPriority = 1000;
    rstl::reserved_vector<std::pair<s32, TUniqueId>, 64> xac_cameraHints;
    rstl::reserved_vector<TUniqueId, 64> x2b0_inactiveCameraHints;
    rstl::reserved_vector<TUniqueId, 64> x334_activeCameraHints;

    union
    {
        struct
        {
            bool x3b8_24_ : 1;
            bool x3b8_25_ : 1;
        };
        u8 _dummy2;
    };

    float x3bc_curFov = 60.f;

    void SetPathCamera(TUniqueId id, CStateManager& mgr);
    void SetSpindleCamera(TUniqueId id, CStateManager& mgr);
    void RestoreHintlessCamera(CStateManager& mgr);
    void InterpolateToBallCamera(const zeus::CTransform& xf, TUniqueId camId, const zeus::CVector3f& lookDir,
                                 float f1, float f2, float f3, bool b1, CStateManager& mgr);
    static constexpr bool ShouldBypassInterpolation() { return false; }
    void SkipBallCameraCinematic(CStateManager& mgr);
    void ApplyCameraHint(const CScriptCameraHint& hint, CStateManager& mgr);

public:
    CCameraManager(TUniqueId curCameraId=kInvalidUniqueId);

    static float Aspect() {return sAspect;}
    static float FarPlane() {return sFarPlane;}
    static float NearPlane() {return sNearPlane;}
    static float FirstPersonFOV() {return sFirstPersonFOV;}
    static float ThirdPersonFOV() {return sThirdPersonFOV;}

    void ResetCameras(CStateManager& mgr);
    void SetSpecialCameras(CFirstPersonCamera& fp, CBallCamera& ball);
    bool IsInCinematicCamera() const {return x4_cineCameras.size() != 0;}
    bool IsInFirstPersonCamera() const;
    zeus::CVector3f GetGlobalCameraTranslation(const CStateManager& stateMgr) const;
    zeus::CTransform GetCurrentCameraTransform(const CStateManager& stateMgr) const;
    void RemoveCameraShaker(int id);
    int AddCameraShaker(const CCameraShakeData& data, bool sfx);
    void AddCinemaCamera(TUniqueId, CStateManager& stateMgr);
    void RemoveCinemaCamera(TUniqueId, CStateManager&);
    void SetInsideFluid(bool, TUniqueId);
    void Update(float dt, CStateManager& stateMgr);
    CGameCamera* GetCurrentCamera(CStateManager& stateMgr) const;
    const CGameCamera* GetCurrentCamera(const CStateManager& stateMgr) const;
    void SetCurrentCameraId(TUniqueId id, CStateManager& stateMgr) {x0_curCameraId = id;}
    void CreateStandardCameras(CStateManager& stateMgr);
    TUniqueId GetCurrentCameraId() const
    {
        if (x4_cineCameras.size())
            return x4_cineCameras.back();
        return x0_curCameraId;
    }
    TUniqueId GetLastCameraId() const
    {
        if (x4_cineCameras.size())
            return x4_cineCameras.back();
        return kInvalidUniqueId;
    }

    void SkipCinematic(CStateManager& stateMgr);

    CFirstPersonCamera* GetFirstPersonCamera() { return x7c_fpCamera; }
    CBallCamera* GetBallCamera() { return x80_ballCamera; }
    CGameArea::CAreaFog& Fog() { return x3c_fog; }

    float GetCameraBobMagnitude() const;

    void UpdateCameraHints(float dt, CStateManager& mgr);
    void ThinkCameras(float dt, CStateManager& mgr);
    void UpdateFog(float dt, CStateManager& mgr);
    void UpdateRumble(float dt, CStateManager& mgr);
    void UpdateListener(CStateManager& mgr);

    float CalculateFogDensity(CStateManager&, const CScriptWater*);

    void ProcessInput(const CFinalInput& input, CStateManager& stateMgr);

    void RenderCameras(const CStateManager& mgr);
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
};

}

#endif // __URDE_CCAMERAMANAGER_HPP__
