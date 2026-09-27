#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include <rstl/math.hpp>

CWeapon::CWeapon(TUniqueId uid, TAreaId areaId, const bool active, TUniqueId owner,
                 EWeaponType type, const rstl::string& name, const CTransform4f& xf,
                 const CMaterialFilter& filter, const CMaterialList& mList,
                 const CDamageInfo& dInfo, int attribs, const CModelData& mData)
: CActor(uid, active, name, CEntityInfo(areaId, CEntity::NullConnectionList), xf, mData, mList,
         CActorParameters::None().HotInThermal(true), kInvalidUniqueId)
, mProjectileAttribs(attribs)
, mOwnerId(owner)
, mWeaponType(type)
, mFilter(filter)
, mOrigDamageInfo(dInfo)
, mCurDamageInfo(dInfo)
, mCurTime(0.f)
, mDamageFalloffSpeed(0.f)
, mDamageDuration(0.f)
, mInterferenceDuration(0.f) {}

CWeapon::~CWeapon() {}

void CWeapon::SetDamageFalloffSpeed(float speed) {
  if (speed > 0.f) {
    mDamageFalloffSpeed = 1.f / speed;
  }
}

void CWeapon::Think(float dt, CStateManager& mgr) {
  mCurTime += dt;
  if ((mProjectileAttribs & CWeapon::kPA_DamageFalloff) == CWeapon::kPA_DamageFalloff) {
    float max = 1.f - mCurTime * mDamageFalloffSpeed;
    float scale = CMath::Max(0.f, max);
    float damage = scale * mOrigDamageInfo.GetDamage();
    float radius = scale * mOrigDamageInfo.GetRadius();
    float knockback = scale * mOrigDamageInfo.GetKnockBackPower();
    mCurDamageInfo =
        CDamageInfo(mOrigDamageInfo.GetWeaponMode(), damage,
                    (double)(scale * mOrigDamageInfo.GetDamage()), radius, knockback);
  } else {
    mCurDamageInfo = mOrigDamageInfo;
  }
  CEntity::Think(dt, mgr);
}

void CWeapon::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  bool doRipple = true;
  float mag = 0.f;
  switch (mWeaponType) {
  case kWT_Power:
    mag = 0.1f;
    break;
  case kWT_Ice:
    mag = 0.3f;
    break;
  case kWT_Wave:
    mag = 0.1f;
    break;
  case kWT_Plasma:
    mag = 0.f;
    break;
  case kWT_Missile:
    mag = 0.5f;
    break;
  case kWT_Phazon:
    mag = 0.1f;
    break;
  default:
    doRipple = false;
    break;
  }

  if ((mProjectileAttribs & CWeapon::kPA_ComboShot) != 0 && state != kFS_InFluid) {
    mag += 0.5f;
  }

  if ((mProjectileAttribs & CWeapon::kPA_Charged) != 0) {
    mag += 0.25f;
  }

  if (mag > 1.f) {
    mag = 1.f;
  }

  if (doRipple) {
    CVector3f pos(GetTranslation().GetX(), GetTranslation().GetY(), water.GetSurfaceZ());
    if ((mProjectileAttribs & CWeapon::kPA_ComboShot) != 0) {
      if (!water.CanRippleAtPoint(pos)) {
        doRipple = false;
      }
    } else if (state == kFS_InFluid) {
      doRipple = false;
    }

    if (doRipple) {
      bool sfx = state == kFS_EnteredFluid || state == kFS_LeftFluid;
      water.FluidPlane().AddRipple(mag, GetUniqueId(), pos, water, mgr);
      mgr.FluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, pos, mag, sfx);
    }
  }
}

void CWeapon::Render(const CStateManager& mgr) const {}

EWeaponCollisionResponseTypes CWeapon::GetCollisionResponseType(const CVector3f& pos,
                                                                const CVector3f& dir,
                                                                const CWeaponMode& wMode,
                                                                int) const {
  return kWCR_Projectile;
}
