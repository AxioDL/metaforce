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

class CCameraManager
{
    TUniqueId x0_curCameraId;
    std::vector<TUniqueId> x4_cineCameras;
    std::list<CCameraShakeData> x18_shakers;
    u32 x2c_lastShakeId = 0;
    zeus::CVector3f x30_shakeOffset;
    CGameArea::CAreaFog x3c_fog;
    int x74_fluidCounter = 0;
    TUniqueId x78_fluidId = kInvalidUniqueId;
    CFirstPersonCamera* x7c_fpCamera = nullptr;
    CBallCamera* x80_ballCamera = nullptr;
    s16 x84_rumbleId = -1;

    union
    {
        struct
        {
            bool x86_24_ : 1;
            bool x86_25_rumbling : 1;
            bool x86_26_inWater : 1;
        };
        u8 _dummy = 0;
    };

public:
    CCameraManager(TUniqueId curCameraId=kInvalidUniqueId);

    static float DefaultAspect() {return 1.42f;}
    static float DefaultFarPlane() {return 750.0f;}
    static float DefaultNearPlane() {return 0.2f;}
    static float DefaultFirstPersonFOV() {return 55.0f;}
    static float DefaultThirdPersonFOV() {return 60.0f;}

    void ResetCameras(CStateManager& mgr);
    void SetSpecialCameras(CFirstPersonCamera& fp, CBallCamera& ball)
    {
        x7c_fpCamera = &fp;
        x80_ballCamera = &ball;
    }
    bool IsInCinematicCamera() const {return x4_cineCameras.size() != 0;}
    zeus::CVector3f GetGlobalCameraTranslation(const CStateManager& stateMgr) const;
    zeus::CTransform GetCurrentCameraTransform(const CStateManager& stateMgr) const;
    void RemoveCameraShaker(int id);
    int AddCameraShaker(const CCameraShakeData& data);
    void AddCinemaCamera(TUniqueId, CStateManager& stateMgr);
    void SetInsideFluid(bool, TUniqueId);
    void Update(float dt, CStateManager& stateMgr);
    CGameCamera* GetCurrentCamera(CStateManager& stateMgr) const;
    const CGameCamera* GetCurrentCamera(const CStateManager& stateMgr) const;
    void SetCurrentCameraId(TUniqueId id, CStateManager& stateMgr) {x0_curCameraId = id;}
    TUniqueId GetCurrentCameraId() const
    {
        if (x4_cineCameras.size())
            return x4_cineCameras.back();
        return x0_curCameraId;
    }

    CBallCamera* GetBallCamera() { return x80_ballCamera; }
};

}

#endif // __URDE_CCAMERAMANAGER_HPP__
