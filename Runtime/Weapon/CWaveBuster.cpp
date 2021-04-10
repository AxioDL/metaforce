#include "Runtime/Weapon/CWaveBuster.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

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
}

void CWaveBuster::UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr) {}

void CWaveBuster::ResetBeam(bool deactivate) {}

void CWaveBuster::SetNewTarget(TUniqueId id) {}

void CWaveBuster::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CWaveBuster::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId senderId, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Deleted) {
    DeleteProjectileLight(mgr);
  } else if (msg == EScriptObjectMessage::Registered) {
    if (x390_busterLightGen != nullptr && x390_busterLightGen->SystemHasLight()) {
      const CLight light = x390_busterLightGen->GetLight();
      CreateProjectileLight("WaveBuster_Light", light, mgr);
    }

    // Thermal hot
    xe6_27_thermalVisorFlags = 2;

    x318_ = x2e8_originalXf.origin;
    x324_ = x34_transform.origin;
    x330_ = x34_transform.origin;
  }

  CGameProjectile::AcceptScriptMsg(msg, senderId, mgr);
}

void CWaveBuster::AddToRenderer([[maybe_unused]] const zeus::CFrustum& frustum, CStateManager& mgr) {
  const auto bounds = GetSortingBounds(mgr);
  EnsureRendered(mgr, x2e8_originalXf.origin, bounds);
}

std::optional<zeus::CAABox> CWaveBuster::GetTouchBounds() const {
  if (x3d0_28_) {
    return std::nullopt;
  }

  return GetProjectileBounds();
}

} // namespace metaforce
