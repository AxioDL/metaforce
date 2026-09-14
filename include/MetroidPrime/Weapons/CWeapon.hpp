#ifndef _CWEAPON
#define _CWEAPON

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CWeapon : public CActor {
public:
  enum EProjectileAttrib {
    kPA_None = 0,
    kPA_PartialCharge = (1 << 0),
    kPA_PlasmaProjectile = (1 << 1),
    kPA_Charged = (1 << 2),
    kPA_Ice = (1 << 3),
    kPA_Wave = (1 << 4),
    kPA_Plasma = (1 << 5),
    kPA_Phazon = (1 << 6),
    kPA_ComboShot = (1 << 7),
    kPA_Bombs = (1 << 8),
    kPA_PowerBombs = (1 << 9),
    kPA_BigProjectile = (1 << 10),
    kPA_ArmCannon = (1 << 11),
    kPA_BigStrike = (1 << 12),
    kPA_DamageFalloff = (1 << 13),
    kPA_StaticInterference = (1 << 14),
    kPA_PlayerUnFreeze = (1 << 15),
    kPA_ParticleOPTS = (1 << 16),
    kPA_KeepInCinematic = (1 << 17),
  };

  CWeapon(const TUniqueId uid, const TAreaId areaId, const bool active, const TUniqueId owner,
          const EWeaponType type, const rstl::string& name, const CTransform4f& xf,
          const CMaterialFilter& filter, const CMaterialList& mList, const CDamageInfo& dInfo,
          const int attribs, const CModelData& mData);

  // CEntity
  ~CWeapon() override;
  DECLARE_TYPES_MATCH;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager&) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void FluidFXThink(EFluidState, CScriptWater&, CStateManager&) override;

  void SetDamageFalloffSpeed(float d);

  void SetDamageDuration(float duration) {
    xe8_projectileAttribs |= kPA_BigStrike;
    x150_damageDuration = duration;
  }
  void SetInterferenceDuration(float duration) {
    xe8_projectileAttribs |= kPA_StaticInterference;
    x154_interferenceDuration = duration;
  }

  int GetAttribField() const { return xe8_projectileAttribs; }
  bool HasAttrib(EProjectileAttrib attrib) const {
    return (xe8_projectileAttribs & attrib) == attrib; // maybe wrong
  }
  TUniqueId GetOwnerId() const { return xec_ownerId; }
  EWeaponType GetType() const { return xf0_weaponType; }
  CMaterialFilter GetFilter() const { return xf8_filter; }

  // TODO: names?
  CDamageInfo& OrigDamageInfo() { return x110_origDamageInfo; }
  CDamageInfo& CurrentDamageInfo() { return x12c_curDamageInfo; }
  const CDamageInfo& GetCurrentDamageInfo() const { return x12c_curDamageInfo; }
  float GetDamageDuration() const { return x150_damageDuration; }
  float GetInterferenceDuration() const { return x154_interferenceDuration; }

  void SetOwnerId(TUniqueId id) { xec_ownerId = id; }
  void SetDamageInfo(const CDamageInfo& dInfo) { x12c_curDamageInfo = dInfo; }
  void SetFilter(const CMaterialFilter& filter) { xf8_filter = filter; }

protected:
  int xe8_projectileAttribs;
  TUniqueId xec_ownerId;
  EWeaponType xf0_weaponType;
  CMaterialFilter xf8_filter;
  CDamageInfo x110_origDamageInfo;
  CDamageInfo x12c_curDamageInfo;
  float x148_curTime;
  float x14c_damageFalloffSpeed;
  float x150_damageDuration;
  float x154_interferenceDuration;
};
CHECK_SIZEOF(CWeapon, (VERSION >= VERSION_GM8P_00 ? 0x168 : 0x158))

#endif // _CWEAPON
