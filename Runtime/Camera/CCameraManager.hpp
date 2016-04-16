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
    zeus::CVector3f x30_;
    CFirstPersonCamera* x7c_fpCamera;
    CBallCamera* x80_ballCamera;
public:
    CCameraManager(TUniqueId curCameraId);

    void SetSpecialCameras(CFirstPersonCamera& fp, CBallCamera& ball)
    {
        x7c_fpCamera = &fp;
        x80_ballCamera = &ball;
    }
    bool IsInCinematicCamera() const {return x4_cineCameras.size() != 0;}
    zeus::CVector3f GetGlobalCameraTranslation(const CStateManager& stateMgr) const;
    zeus::CTransform GetGlobalCameraTransform(const CStateManager& stateMgr) const;
    void RemoveCameraShaker(int);
    void AddCinemaCamera(TUniqueId, CStateManager& stateMgr);
    void SetInsideFluid(bool, TUniqueId);
    void Update(float dt, CStateManager& stateMgr);
    CGameCamera* GetCurrentCamera(CStateManager& stateMgr) const;
    const CGameCamera* GetCurrentCamera(const CStateManager& stateMgr) const;
    void SetCurrentCameraId(TUniqueId id, CStateManager& stateMgr);
    TUniqueId GetCurrentCameraId() const
    {
        if (x4_cineCameras.size())
            return x4_cineCameras.back();
        return x0_curCameraId;
    }
};

}

#endif // __URDE_CCAMERAMANAGER_HPP__
