#include "CGunController.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "Character/CPASAnimParmData.hpp"
#include "CStateManager.hpp"

namespace urde
{

bool CGunController::Update(float dt, CStateManager& mgr)
{
    CAnimData& animData = *x0_modelData.AnimationData();
    switch (x50_gunState)
    {
    case EGunState::FreeLook:
    {
        x58_24_ = x4_freeLook.Update(animData, dt, mgr);
        if (!x58_24_ || !x58_25_)
            break;

        EnterComboFire(mgr, x4_freeLook.xc_);
        x58_24_ = false;
        break;
    }
    case EGunState::ComboFire:
        x58_24_ = x1c_comboFire.Update(animData, dt, mgr);
        break;
    case EGunState::Fidget:
        x58_24_ = x30_fidget.Update(animData, dt, mgr);
        break;
    case EGunState::Six:
    {
        if (animData.IsAnimTimeRemaining(0.001f, "Whole Body"))
            break;
        x54_ = x4_freeLook.SetAnim(animData, x4_freeLook.xc_, x4_freeLook.x10_, 0, mgr, 0.f);
        x50_gunState = EGunState::FreeLook;
        break;
    }
    case EGunState::Seven:
        x58_24_ = !animData.IsAnimTimeRemaining(0.001f, "Whole Body");
        break;
    default:
        break;
    }

    if (!x58_24_)
        return false;

    x50_gunState = EGunState::Zero;
    x58_25_ = false;

    return true;
}

void CGunController::ReturnToBasePosition(CStateManager& mgr, float)
{
    const CPASDatabase& pasDatabase = x0_modelData.AnimationData()->GetCharacterInfo().GetPASDatabase();
    std::pair<float, s32> anim =
        pasDatabase.FindBestAnimation(CPASAnimParmData::NoParameters(6), *mgr.GetActiveRandom(), -1);
}

void CGunController::Reset()
{
    x58_24_ = true;
    x58_25_ = false;
    x50_gunState = EGunState::Zero;
}
}
