#ifndef __URDE_CCAMERAMANAGER_HPP__
#define __URDE_CCAMERAMANAGER_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CFirstPersonCamera;
class CBallCamera;
class CStateManager;

class CCameraManager
{
    TUniqueId x0_id;
    std::vector<TUniqueId> x4_cineCameras;
    CFirstPersonCamera* x7c_fpCamera;
    CBallCamera* x80_ballCamera;
public:
    CCameraManager(TUniqueId id);

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

};

}

#endif // __URDE_CCAMERAMANAGER_HPP__
