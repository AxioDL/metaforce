#include "Runtime/Weapon/CWaveBuster.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace urde {

CWaveBuster::CWaveBuster(const TToken<CWeaponDescription>& desc, EWeaponType type, const zeus::CTransform& xf,
                         EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                         TUniqueId homingTarget, EProjectileAttrib attrib)
: CGameProjectile(true, desc, "WaveBuster", type, xf, matType, dInfo, uid, aid, owner, homingTarget, attrib, false,
                  zeus::skOne3f, {}, -1, false)
, x2e8_originalXf(xf)
, x348_targetPoint(x2e8_originalXf.basis[1].normalized() * 25.f + x2e8_originalXf.origin)
, x354_busterSwoosh1(g_SimplePool->GetObj("BusterSwoosh1"))
, x360_busterSwoosh2(g_SimplePool->GetObj("BusterSwoosh2"))
, x36c_busterSparks(g_SimplePool->GetObj("BusterSparks"))
, x378_busterLight(g_SimplePool->GetObj("BusterLight")) {
  x354_busterSwoosh1.GetObj();
  x360_busterSwoosh2.GetObj();
  x36c_busterSparks.GetObj();
  x378_busterLight.GetObj();
  x384_busterSwoosh1Gen = std::make_unique<CParticleSwoosh>(x354_busterSwoosh1, 0);
  x388_busterSwoosh2Gen = std::make_unique<CParticleSwoosh>(x360_busterSwoosh2, 0);
  x38c_busterSparksGen = std::make_unique<CElementGen>(x36c_busterSparks);
  x390_busterLightGen = std::make_unique<CElementGen>(x378_busterLight);
  x3d0_24_firing = true;
  x3d0_25_ = true;
  x3d0_26_ = false;
  x3d0_27_ = false;
  x3d0_28_ = true;
}

void CWaveBuster::UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr) {}

void CWaveBuster::ResetBeam(bool deactivate) {}

void CWaveBuster::SetNewTarget(TUniqueId id) {}

} // namespace urde
