#include "Runtime/Weapon/CNewFlameThrower.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CGameLight.hpp"

namespace urde {

CNewFlameThrower::CNewFlameThrower(const TToken<CWeaponDescription>& desc, std::string_view name, EWeaponType wType,
                                   const std::array<CAssetId, 8>& resInfo, const zeus::CTransform& xf,
                                   EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid,
                                   TUniqueId owner, EProjectileAttrib attribs)
: CGameProjectile(false, desc, name, wType, xf, matType, dInfo, uid, aid, owner, kInvalidUniqueId, attribs, false,
                  zeus::skOne3f, {}, -1, false)
, x304_mainFire(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[0]}))
, x310_mainSmoke(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[1]}))
, x31c_secondarySmoke(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[4]}))
, x328_secondaryFire(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[5]}))
, x334_secondarySparks(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[6]}))
, x340_swooshCenter(g_SimplePool->GetObj(SObjectTag{FOURCC('SWHC'), resInfo[2]}))
, x34c_swooshFire(g_SimplePool->GetObj(SObjectTag{FOURCC('SWHC'), resInfo[3]})) {
  x304_mainFire.GetObj();
  x310_mainSmoke.GetObj();
  x31c_secondarySmoke.GetObj();
  x328_secondaryFire.GetObj();
  x334_secondarySparks.GetObj();
  x340_swooshCenter.GetObj();
  x34c_swooshFire.GetObj();
  x380_.resize(3);
}

void CNewFlameThrower::DeleteLightObjects(CStateManager& mgr) {
  for (TUniqueId id : x3b8_lightIds)
    mgr.FreeScriptObject(id);
  x3b8_lightIds.clear();
}

void CNewFlameThrower::CreateLightObjects(CStateManager& mgr) {
  DeleteLightObjects(mgr);
  for (int i = 0; i < 4; ++i) {
    TUniqueId uid = mgr.AllocateUniqueId();
    CLight lObj = x358_mainFireGen->GetLight();
    CGameLight* light = new CGameLight(uid, GetAreaId(), false, "FlamethrowerLight", zeus::CTransform(),
                                       x8_uid, lObj, u32(reinterpret_cast<uintptr_t>(this) + (i & 0x1)), 0, 0.f);
    mgr.AddObject(light);
    x3b8_lightIds.push_back(uid);
  }
}

void CNewFlameThrower::EnableFx(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  x358_mainFireGen = std::make_unique<CElementGen>(x304_mainFire);
  x35c_mainSmokeGen = std::make_unique<CElementGen>(x310_mainSmoke);
  x360_secondarySmokeGen = std::make_unique<CElementGen>(x31c_secondarySmoke);
  x364_secondaryFireGen = std::make_unique<CElementGen>(x328_secondaryFire);
  x368_secondarySparksGen = std::make_unique<CElementGen>(x334_secondarySparks);
  x36c_swooshCenterGen = std::make_unique<CParticleSwoosh>(x340_swooshCenter, 0);
  x36c_swooshCenterGen->SetRenderGaps(true);
  x370_swooshFireGen = std::make_unique<CParticleSwoosh>(x34c_swooshFire, 0);
  x370_swooshFireGen->SetRenderGaps(true);
  if (x358_mainFireGen && x358_mainFireGen->SystemHasLight() && x3b8_lightIds.empty())
    CreateLightObjects(mgr);
}

void CNewFlameThrower::StartFiring(const zeus::CTransform& xf, CStateManager& mgr) {
  SetActive(true);
  x37c_25_firing = true;
  x37c_24_renderAuxEffects = true;
  x374_ = 1;
  EnableFx(mgr);
}

bool CNewFlameThrower::AreEffectsFinished() const {
  if (x358_mainFireGen && x358_mainFireGen->GetParticleCount() != 0)
    return false;
  if (x35c_mainSmokeGen && x35c_mainSmokeGen->GetParticleCount() != 0)
    return false;
  if (x360_secondarySmokeGen && x360_secondarySmokeGen->GetParticleCount() != 0)
    return false;
  if (x364_secondaryFireGen && x364_secondaryFireGen->GetParticleCount() != 0)
    return false;
  return !(x368_secondarySparksGen && x368_secondarySparksGen->GetParticleCount() != 0);
}

void CNewFlameThrower::UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr) {}

void CNewFlameThrower::Reset(CStateManager& mgr, bool deactivate) {}

} // namespace urde
