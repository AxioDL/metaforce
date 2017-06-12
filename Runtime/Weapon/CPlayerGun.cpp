#include "CPlayerGun.hpp"
#include "Particle/CGenDescription.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

CPlayerGun::CPlayerGun(TUniqueId id)
: x0_lights(8, zeus::CVector3f{-30.f, 0.f, 30.f}, 4, 4, 0, 0, 0, 0.1f), x538_thisId(id),
  x550_camBob(CPlayerCameraBob::ECameraBobType::One,
              zeus::CVector2f(0.071f, 0.141f), 0.47f),
  x678_morph(g_tweakPlayerGun->GetX38(), g_tweakPlayerGun->GetX34())
{
    x354_ = g_tweakPlayerGun->GetX2c();
    x358_ = g_tweakPlayerGun->GetX30();
    x668_ = g_tweakPlayerGun->GetX24();
    x66c_ = g_tweakPlayerGun->GetX28();

    x832_31_ = true;
    x833_24_ = true;
    x833_30_ = true;
    x6e0_.SetSortThermal(true);

    /* TODO: Finish */
}

void CPlayerGun::AcceptScriptMessage(EScriptObjectMessage, TUniqueId, CStateManager&)
{

}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr)
{
    x72c_currentBeam->AsyncLoadSuitArm(mgr);
    x740_grappleArm->AsyncLoadSuit(mgr);
}

void CPlayerGun::TouchModel(CStateManager& stateMgr)
{

}

}
