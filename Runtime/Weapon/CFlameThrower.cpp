#include "Weapon/CFlameThrower.hpp"
#include "Weapon/CFlameInfo.hpp"
#include "World/CGameLight.hpp"
#include "Particle/CElementGen.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "CStateManager.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "TCastTo.hpp"
#include "CFlameThrower.hpp"

namespace urde {
const zeus::CVector3f CFlameThrower::kLightOffset(0, 3.f, 2.f);

CFlameThrower::CFlameThrower(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                             const CFlameInfo& flameInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                             const CDamageInfo& dInfo, TUniqueId uid, TAreaId aId, TUniqueId owner,
                             EProjectileAttrib attribs, CAssetId assetId1, s16 sId, CAssetId assetId2)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, uid, aId, owner, kInvalidUniqueId, attribs, false,
                  zeus::CVector3f(1.f), {}, -1, false)
, x2e8_(xf)
, x338_(flameInfo.x10_)
, x33c_flameDesc(g_SimplePool->GetObj({FOURCC('PART'), flameInfo.GetFlameFxId()}))
, x348_flameGen(new CElementGen(x33c_flameDesc))
, x34c_(176.f - float(flameInfo.GetLength()), xf.origin, bool(flameInfo.GetAttributes() & 0x4))
, x3f4_(assetId1)
, x3f8_(sId)
, x3fc_(assetId2)
, x400_24_(false)
, x400_25_(false)
, x400_26_(!(flameInfo.GetAttributes() & 1))
, x400_27_((flameInfo.GetAttributes() & 0x2) != 0) {

}

void CFlameThrower::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFlameThrower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {
    xe6_27_thermalVisorFlags |= 2;
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
  } else if (msg == EScriptObjectMessage::Deleted) {
    mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
    DeleteProjectileLight(mgr);
  }

  CGameProjectile::AcceptScriptMsg(msg, uid, mgr);
}

void CFlameThrower::SetTransform(const zeus::CTransform& xf, float) { x2e8_ = xf; }

void CFlameThrower::Reset(CStateManager& mgr, bool resetWarp) {
  SetFlameLightActive(mgr, false);
  if (resetWarp) {
    SetActive(false);
    x400_25_ = false;
    x3f0_flameState = 0;
    x330_ = 0.f;
    x334_ = 0.f;
    x318_ = zeus::skNullBox;
    x348_flameGen->SetParticleEmission(false);
    x34c_.ResetPosition(x2e8_.origin);
  } else {
    x348_flameGen->SetParticleEmission(false);
    x400_25_ = false;
    x3f0_flameState = 3;
  }
}

void CFlameThrower::Fire(const zeus::CTransform&, CStateManager& mgr, bool) {
  SetActive(true);
  x400_25_ = true;
  x400_24_ = true;
  x3f0_flameState = 1;
  CreateFlameParticles(mgr);
}

void CFlameThrower::CreateFlameParticles(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  x348_flameGen.reset(new CElementGen(x33c_flameDesc));
  x348_flameGen->SetParticleEmission(true);
  x348_flameGen->SetZTest(x400_27_);
  x348_flameGen->AddModifier(&x34c_);
  if (x348_flameGen->SystemHasLight() && x2c8_projectileLight == kInvalidUniqueId)
    CreateProjectileLight("FlameThrower_Light"sv, x348_flameGen->GetLight(), mgr);
}

void CFlameThrower::AddToRenderer(const zeus::CFrustum&, const CStateManager& mgr) const {
  g_Renderer->AddParticleGen(*x348_flameGen);
  EnsureRendered(mgr, x2e8_.origin, GetRenderBounds());
}

void CFlameThrower::SetFlameLightActive(CStateManager& mgr, bool active) {
  if (x2c8_projectileLight == kInvalidUniqueId)
    return;

  if (TCastToPtr<CGameLight> light = mgr.ObjectById(x2c8_projectileLight))
    light->SetActive(active);
}

void CFlameThrower::UpdateFlameState(float dt, CStateManager& mgr) {
  switch(x3f0_flameState) {
  case 1:
    x3f0_flameState = 2;
    break;
  case 3:
    x334_ += 4.f * dt;
    if (x334_ > 1.f) {
      x334_ = 1.f;
      x3f0_flameState = 4;
      x400_24_ = false;
    }
    break;
  case 4:
    x330_ += dt;
    if (x330_ > 0.1f && x348_flameGen && x348_flameGen->GetParticleCountAll() == 0) {
      x3f0_flameState = 0;
      Reset(mgr, true);
    }
    break;
  default:
    break;
  }
}

void CFlameThrower::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (!GetActive())
    return;

  UpdateFlameState(dt, mgr);
}
} // namespace urde
