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
    float x90_ = 0.f;
    float x94_ = 1.f;
    float x98_ = 0.f;
    float x9c_ = 1.f;

    union
    {
        struct
        {
            bool xa0_24_ : 1;
            bool xa0_25_rumbling : 1;
            bool xa0_26_inWater : 1;
        };
        u8 _dummy1 = 0;
    };

    TUniqueId xa2_ = kInvalidUniqueId;
    TUniqueId xa4_ = kInvalidUniqueId;
    TUniqueId xa6_ = kInvalidUniqueId;
    u32 xa8_ = 1000;
    u32 xac_ = 0;

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
    zeus::CVector3f GetGlobalCameraTranslation(const CStateManager& stateMgr) const;
    zeus::CTransform GetCurrentCameraTransform(const CStateManager& stateMgr) const;
    void RemoveCameraShaker(int id);
    int AddCameraShaker(const CCameraShakeData& data, bool sfx);
    void AddCinemaCamera(TUniqueId, CStateManager& stateMgr);
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
    CBallCamera* BallCamera(CStateManager&) const;
    CGameArea::CAreaFog& Fog() { return x3c_fog; }

    float sub80009148() const;

    void sub800097AC(float, CStateManager&);
    void ThinkCameras(float, CStateManager&);
    void UpdateFog(float, CStateManager&);
    void UpdateRumble(float, CStateManager&);
    void UpdateListener(CStateManager&);

    float CalculateFogDensity(CStateManager&, const CScriptWater*);

    void ProcessInput(const CFinalInput& input, CStateManager& stateMgr);
};

}

#endif // __URDE_CCAMERAMANAGER_HPP__
