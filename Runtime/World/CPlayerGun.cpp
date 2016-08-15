#include "CPlayerGun.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

CPlayerGun::CPlayerGun(TUniqueId id)
: x0_lights(8, zeus::CVector3f{-30.f, 0.f, 30.f}, 4, 4, 0), x538_thisId(id),
  x550_camBob(CPlayerCameraBob::ECameraBobType::One,
              zeus::CVector2f(0.071f, 0.141f), 0.47f),
  x678_morph(g_tweakPlayerGun->GetSomething6(), g_tweakPlayerGun->GetSomething5())
{
    x354_ = g_tweakPlayerGun->GetSomething3();
    x358_ = g_tweakPlayerGun->GetSomething4();
    x668_ = g_tweakPlayerGun->GetSomething1();
    x66c_ = g_tweakPlayerGun->GetSomething2();

    x832_31_ = true;
    x833_24_ = true;
    x833_30_ = true;
    x6e0_.SetSortThermal(true);

    /* TODO: Finish */
}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr)
{
    x72c_currentBeam->AsyncLoadSuitArm(mgr);
    x740_grappleArm->AsyncLoadSuit(mgr);
}

}
