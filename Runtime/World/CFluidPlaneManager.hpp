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
    class CSplashRecord
    {
    public:
        CSplashRecord(TUniqueId id);
        void SetTime(float t);
        float GetTime() const;
        TUniqueId GetUniqueId() const;
    };

    CRippleManager x0_rippleManager;
    rstl::reserved_vector<CSplashRecord, 32> x18_splashes;
    float x11c_uvT = 0.f;
    bool x120_ = false;
    bool x121_ = false;

    class CFluidProfile
    {
    public:
        void Clear();
    };
    static CFluidProfile sProfile;
    static void SetupRippleMap();
public:
    static u8 RippleValues[64][64];
    static u8 RippleMins[64];
    static u8 RippleMaxs[64];

    CFluidPlaneManager();
    void StartFrame(bool);
    void EndFrame() { x121_ = false; }
    void Update(float dt);
    float GetUVT() const { return x11c_uvT; }
    float GetLastRippleDeltaTime(TUniqueId rippler) const;
    float GetLastSplashDeltaTime(TUniqueId splasher) const;
    void CreateSplash(TUniqueId splasher, CStateManager& mgr, const CScriptWater& water,
                      const zeus::CVector3f& pos, float factor, bool);
    rstl::reserved_vector<CSplashRecord, 32>& SplashRecords() { return x18_splashes; }
    const CRippleManager& GetRippleManager() const { return x0_rippleManager; }
    CRippleManager& RippleManager() { return x0_rippleManager; }
};

}

#endif // __URDE_CFLUIDPLANEMANAGER_HPP__
