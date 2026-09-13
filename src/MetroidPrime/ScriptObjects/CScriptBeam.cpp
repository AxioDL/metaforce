#include "MetroidPrime/ScriptObjects/CScriptBeam.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"

CScriptBeam::CScriptBeam(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const bool active,
                         const TToken< CWeaponDescription >& weaponDesc, const CBeamInfo& bInfo,
                         const CDamageInfo& dInfo)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_weaponDescription(weaponDesc)
, xf4_beamInfo(bInfo)
, x138_damageInfo(dInfo)
, x154_projectileId(kInvalidUniqueId) {}

void CScriptBeam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  switch (msg) {

  case kSM_Registered: {
    x154_projectileId = mgr.AllocateUniqueId();
    mgr.AddObject(rs_new CPlasmaProjectile(
        xe8_weaponDescription, GetDebugName() + rstl::string_l("-Projectile"),
        x138_damageInfo.GetWeaponMode().GetType(), xf4_beamInfo, GetTransform(), kMT_Projectile,
        x138_damageInfo, x154_projectileId, GetCurrentAreaId(), GetUniqueId(), CWeaponAssetInfo(),
        false,
        CWeapon::kPA_KeepInCinematic // TODO: wrong attrib definition?
        ));
  } break;

  case kSM_Deleted:
    mgr.DeleteObjectRequest(x154_projectileId);
    break;

  case kSM_Increment:
    if (CPlasmaProjectile* proj =
            static_cast< CPlasmaProjectile* >(mgr.ObjectById(x154_projectileId))) {
      proj->ResetBeam(mgr, true);
      proj->Fire(GetTransform(), mgr, false);
    }
    break;
  case kSM_Decrement:
    if (CPlasmaProjectile* proj =
            static_cast< CPlasmaProjectile* >(mgr.ObjectById(x154_projectileId))) {
      if (proj->GetActive()) {
        proj->ResetBeam(mgr, false);
      }
    }
    break;
  }
  CActor::AcceptScriptMsg(msg, objId, mgr);
}

ENTITY_ACCEPT_IMPL(CScriptBeam)

void CScriptBeam::Think(float dt, CStateManager& mgr) {
  if (CPlasmaProjectile* proj =
          static_cast< CPlasmaProjectile* >(mgr.ObjectById(x154_projectileId))) {
    if (proj->GetActive()) {
      proj->UpdateFx(GetTransform(), dt, mgr);
    }
  } else {
    x154_projectileId = kInvalidUniqueId;
  }
}
