#include "MetroidPrime/ScriptObjects/CScriptBeam.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"

CScriptBeam::CScriptBeam(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const bool active,
                         const TToken< CWeaponDescription >& weaponDesc, const CBeamInfo& bInfo,
                         const CDamageInfo& dInfo)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mWeaponDescription(weaponDesc)
, mBeamInfo(bInfo)
, mDamageInfo(dInfo)
, mProjectileId(kInvalidUniqueId) {}

void CScriptBeam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  switch (msg) {

  case kSM_Registered: {
    mProjectileId = mgr.AllocateUniqueId();
    mgr.AddObject(rs_new CPlasmaProjectile(
        mWeaponDescription, GetDebugName() + rstl::string_l("-Projectile"),
        mDamageInfo.GetWeaponMode().GetType(), mBeamInfo, GetTransform(), kMT_Projectile,
        mDamageInfo, mProjectileId, GetCurrentAreaId(), GetUniqueId(), CWeaponAssetInfo(),
        false,
        CWeapon::kPA_KeepInCinematic // TODO: wrong attrib definition?
        ));
  } break;

  case kSM_Deleted:
    mgr.DeleteObjectRequest(mProjectileId);
    break;

  case kSM_Increment:
    if (CPlasmaProjectile* proj =
            static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId))) {
      proj->ResetBeam(mgr, true);
      proj->Fire(GetTransform(), mgr, false);
    }
    break;
  case kSM_Decrement:
    if (CPlasmaProjectile* proj =
            static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId))) {
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
          static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId))) {
    if (proj->GetActive()) {
      proj->UpdateFx(GetTransform(), dt, mgr);
    }
  } else {
    mProjectileId = kInvalidUniqueId;
  }
}
