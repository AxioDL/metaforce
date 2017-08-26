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
    Inactive,
    Default,
    FreeLook,
    ComboFire,
    Idle,
    Fidget,
    Strike,
    BigStrike
};

class CGunController
{
    CModelData& x0_modelData;
    CGSFreeLook x4_freeLook;
    CGSComboFire x1c_comboFire;
    CGSFidget x30_fidget;
    EGunState x50_gunState = EGunState::Inactive;
    s32 x54_curAnimId = -1;
    bool x58_24_animDone : 1;
    bool x58_25_enteredComboFire : 1;

public:
    CGunController(CModelData& modelData)
    : x0_modelData(modelData)
    {
        x58_24_animDone = true;
        x58_25_enteredComboFire = false;
    }

    void UnLoadFidget() { x30_fidget.UnLoadAnim(); }
    void LoadFidgetAnimAsync(CStateManager& mgr, s32 type, s32 parm1, s32 parm2);
    void EnterFidget(CStateManager& mgr, s32 type, s32 parm1, s32 parm2);
    bool IsFidgetLoaded() const { return x30_fidget.IsAnimLoaded(); }
    s32 GetFreeLookSetId() const { return x4_freeLook.GetSetId(); }
    bool IsComboOver() const { return x1c_comboFire.IsComboOver(); }
    void EnterFreeLook(CStateManager& mgr, s32 gunId, s32 setId);
    void EnterComboFire(CStateManager& mgr, s32 gunId);
    void EnterStruck(CStateManager& mgr, float angle, bool bigStrike, bool b2);
    void EnterIdle(CStateManager& mgr);
    bool Update(float dt, CStateManager& mgr);
    void ReturnToDefault(CStateManager& mgr, float dt, bool setState);
    void ReturnToBasePosition(CStateManager&, float);
    void Reset();
};
}

#endif // __URDE_CGUNCONTROLLER_HPP__
