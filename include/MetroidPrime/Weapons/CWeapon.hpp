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
    mProjectileAttribs |= kPA_BigStrike;
    mDamageDuration = duration;
  }
  void SetInterferenceDuration(float duration) {
    mProjectileAttribs |= kPA_StaticInterference;
    mInterferenceDuration = duration;
  }

  int GetAttribField() const { return mProjectileAttribs; }
  bool HasAttrib(EProjectileAttrib attrib) const {
    return (mProjectileAttribs & attrib) == attrib; // maybe wrong
  }
  TUniqueId GetOwnerId() const { return mOwnerId; }
  EWeaponType GetType() const { return mWeaponType; }
  CMaterialFilter GetFilter() const { return mFilter; }

  // TODO: names?
  CDamageInfo& OrigDamageInfo() { return mOrigDamageInfo; }
  CDamageInfo& CurrentDamageInfo() { return mCurDamageInfo; }
  const CDamageInfo& GetCurrentDamageInfo() const { return mCurDamageInfo; }
  float GetDamageDuration() const { return mDamageDuration; }
  float GetInterferenceDuration() const { return mInterferenceDuration; }

  void SetOwnerId(TUniqueId id) { mOwnerId = id; }
  void SetDamageInfo(const CDamageInfo& dInfo) { mCurDamageInfo = dInfo; }
  void SetFilter(const CMaterialFilter& filter) { mFilter = filter; }

protected:
  int mProjectileAttribs;
  TUniqueId mOwnerId;
  EWeaponType mWeaponType;
  CMaterialFilter mFilter;
  CDamageInfo mOrigDamageInfo;
  CDamageInfo mCurDamageInfo;
  float mCurTime;
  float mDamageFalloffSpeed;
  float mDamageDuration;
  float mInterferenceDuration;
};
CHECK_SIZEOF(CWeapon, (VERSION >= VERSION_GM8E_02 ? 0x168 : 0x158))

#endif // _CWEAPON
