#include "Runtime/World/CScriptBeam.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/Weapon/CPlasmaProjectile.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptBeam::CScriptBeam(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         bool active, const TToken<CWeaponDescription>& weaponDesc, const CBeamInfo& bInfo,
                         const CDamageInfo& dInfo)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_weaponDescription(weaponDesc)
, xf4_beamInfo(bInfo)
, x138_damageInfo(dInfo) {}

void CScriptBeam::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptBeam::Think(float dt, CStateManager& mgr) {
  if (CPlasmaProjectile* proj = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x154_projectileId))) {
    if (proj->GetActive())
      proj->UpdateFx(x34_transform, dt, mgr);
  } else
    x154_projectileId = kInvalidUniqueId;
}

void CScriptBeam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Increment) {
    if (CPlasmaProjectile* proj = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x154_projectileId))) {
      proj->ResetBeam(mgr, true);
      proj->Fire(GetTransform(), mgr, false);
    }
  } else if (msg == EScriptObjectMessage::Decrement) {
    if (CPlasmaProjectile* proj = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x154_projectileId))) {
      if (proj->GetActive()) {
        proj->ResetBeam(mgr, false);
      }
    }
  } else if (msg == EScriptObjectMessage::Registered) {
    x154_projectileId = mgr.AllocateUniqueId();
    mgr.AddObject(new CPlasmaProjectile(xe8_weaponDescription, x10_name + "-Projectile",
                                        x138_damageInfo.GetWeaponMode().GetType(), xf4_beamInfo, x34_transform,
                                        EMaterialTypes::Projectile, x138_damageInfo, x8_uid, x4_areaId,
                                        x154_projectileId, {}, false, EProjectileAttrib::PlasmaProjectile));
  } else if (msg == EScriptObjectMessage::Deleted) {
    mgr.FreeScriptObject(x154_projectileId);
  }

  CActor::AcceptScriptMsg(msg, objId, mgr);
}
} // namespace urde
