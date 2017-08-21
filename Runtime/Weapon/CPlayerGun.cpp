#include "CPlayerGun.hpp"
#include "Particle/CGenDescription.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const zeus::CVector3f sGunScale(2.f);

CPlayerGun::CPlayerGun(TUniqueId id)
: x0_lights(8, zeus::CVector3f{-30.f, 0.f, 30.f}, 4, 4, 0, 0, 0, 0.1f), x538_thisId(id),
  x550_camBob(CPlayerCameraBob::ECameraBobType::One,
              zeus::CVector2f(CPlayerCameraBob::kCameraBobExtentX, CPlayerCameraBob::kCameraBobExtentY),
              CPlayerCameraBob::kCameraBobPeriod),
  x678_morph(g_tweakPlayerGun->GetGunTransformTime(), g_tweakPlayerGun->GetHoloHoldTime()),
  x6c8_(zeus::CVector3f(-0.29329199f, 0.f, -0.2481945f),
        zeus::CVector3f(0.29329199f, 1.292392f, 0.2481945f)),
  x6e0_rightHandModel(CAnimRes(g_tweakGunRes->xc_rightHand, 0, zeus::CVector3f(3.f), 0, true))
{
    x354_bombFuseTime = g_tweakPlayerGun->GetBombFuseTime();
    x358_bombDropDelayTime = g_tweakPlayerGun->GetBombDropDelayTime();
    x668_aimVerticalSpeed = g_tweakPlayerGun->GetAimVerticalSpeed();
    x66c_aimHorizontalSpeed = g_tweakPlayerGun->GetAimHorizontalSpeed();

    x73c_gunMotion = std::make_unique<CGunMotion>(g_tweakGunRes->x4_gunMotion, sGunScale);
    x740_grappleArm = std::make_unique<CGrappleArm>(sGunScale);
    x744_auxWeapon = std::make_unique<CAuxWeapon>(id);


    x832_31_ = true;
    x833_24_isFidgeting = true;
    x833_30_ = true;
    x6e0_rightHandModel.SetSortThermal(true);

    /* TODO: Finish */
}

void CPlayerGun::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&)
{

}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr)
{
    x72c_currentBeam->AsyncLoadSuitArm(mgr);
    x740_grappleArm->AsyncLoadSuit(mgr);
}

void CPlayerGun::TouchModel(const CStateManager& stateMgr)
{

}

void CPlayerGun::DamageRumble(const zeus::CVector3f& location, float damage, const CStateManager& mgr)
{
    x398_damageAmt = damage;
    x3dc_damageLocation = location;
}

void CPlayerGun::ProcessInput(const CFinalInput& input, CStateManager& mgr)
{

}

void CPlayerGun::ResetIdle(CStateManager& mgr)
{

}

void CPlayerGun::CancelFiring(CStateManager& mgr)
{

}

float CPlayerGun::GetBeamVelocity() const
{
    if (x72c_currentBeam->IsLoaded())
        return x72c_currentBeam->GetVelocityInfo().GetVelocity(x330_chargeWeaponIdx).y;
    return 10.f;
}

void CPlayerGun::Update(float grappleSwingT, float cameraBobT, float dt, CStateManager& mgr)
{

}

void CPlayerGun::PreRender(const CStateManager& mgr, const zeus::CFrustum& frustum, const zeus::CVector3f& camPos)
{

}

void CPlayerGun::Render(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags) const
{

}

void CPlayerGun::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const
{

}

TUniqueId CPlayerGun::DropPowerBomb(CStateManager& mgr)
{
    return {};
}

}
