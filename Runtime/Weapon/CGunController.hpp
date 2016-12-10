#ifndef __URDE_CGUNCONTROLLER_HPP__
#define __URDE_CGUNCONTROLLER_HPP__

#include "Character/CModelData.hpp"
#include "CGSFreeLook.hpp"
#include "CGSComboFire.hpp"
#include "CGSFidget.hpp"

namespace urde
{
enum class EGunState
{
    Zero,
    One,
    FreeLook,
    ComboFire,
    Four,
    Fidget,
    Six,
    Seven
};

class CGunController
{
    CModelData& x0_modelData;
    CGSFreeLook x4_freeLook;
    CGSComboFire x1c_comboFire;
    CGSFidget x30_fidget;
    EGunState x50_gunState = EGunState::Zero;
    u32 x54_ = -1;

    union {
        struct
        {
            bool x58_24_ : 1;
            bool x58_25_ : 1;
        };
        u8 _dummy = 0;
    };

public:
    CGunController(CModelData& modelData) : x0_modelData(modelData), x58_24_(true) {}
    void UnLoadFidget();
    void LoadFidgetAnimAsync(CStateManager&, s32, s32, s32);
    void GetFreeLookSetId() const;
    bool IsFidgetLoaded() const;
    bool IsComboOver() const;
    void EnterFreeLook(CStateManager&, s32, s32);
    void EnterComboFire(CStateManager&, s32);
    void EnterFidget(CStateManager&, s32, s32, s32);
    void EnterStruck(CStateManager&, float);
    void EnterIdle(CStateManager&);
    bool Update(float, CStateManager&);
    void ReturnToDefault(CStateManager&, float);
    void ReturnToBasePosition(CStateManager&, float);
    void Reset();
};
}

#endif // __URDE_CGUNCONTROLLER_HPP__
