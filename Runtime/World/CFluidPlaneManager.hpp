#ifndef __URDE_CFLUIDPLANEMANAGER_HPP__
#define __URDE_CFLUIDPLANEMANAGER_HPP__

#include "RetroTypes.hpp"
#include "CRippleManager.hpp"

namespace urde
{
class CStateManager;
class CScriptWater;

class CFluidPlaneManager
{
    CRippleManager x0_rippleManager;
    bool x121_;
    class CFluidProfile
    {
    public:
        void Clear();
    };
    static CFluidProfile sProfile;
public:
    CFluidPlaneManager();
    void StartFrame(bool);
    void EndFrame() { x121_ = false; }
    void Update(float dt);
    float GetLastRippleDeltaTime(TUniqueId rippler) const;
    float GetLastSplashDeltaTime(TUniqueId splasher) const;
    void CreateSplash(TUniqueId splasher, CStateManager& mgr, const CScriptWater& water,
                      const zeus::CVector3f& pos, float factor, bool);
};

}

#endif // __URDE_CFLUIDPLANEMANAGER_HPP__
