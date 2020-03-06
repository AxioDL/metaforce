#pragma once

#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Collision/CMaterialFilter.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/Weapon/WeaponCommon.hpp"

namespace urde {
class CWeapon : public CActor {
protected:
  EProjectileAttrib xe8_projectileAttribs;
  TUniqueId xec_ownerId;
  EWeaponType xf0_weaponType;
  CMaterialFilter xf8_filter;
  CDamageInfo x110_origDamageInfo;
  CDamageInfo x12c_curDamageInfo;
  float x148_curTime = 0.f;
  float x14c_damageFalloffSpeed = 0.f;
  float x150_damageDuration = 0.f;
  float x154_interferenceDuration = 0.f;

public:
  CWeapon(TUniqueId uid, TAreaId aid, bool active, TUniqueId owner, EWeaponType type, std::string_view name,
          const zeus::CTransform& xf, const CMaterialFilter& filter, const CMaterialList& mList, const CDamageInfo&,
          EProjectileAttrib attribs, CModelData&& mData);

  void Accept(IVisitor& visitor) override;
  bool HasAttrib(EProjectileAttrib attrib) const { return (int(xe8_projectileAttribs) & int(attrib)) == int(attrib); }
  EProjectileAttrib GetAttribField() const { return xe8_projectileAttribs; }
  void AddAttrib(EProjectileAttrib attrib) { xe8_projectileAttribs |= attrib; }
  const CMaterialFilter& GetFilter() const { return xf8_filter; }
  void SetFilter(const CMaterialFilter& filter) { xf8_filter = filter; }
  TUniqueId GetOwnerId() const { return xec_ownerId; }
  void SetOwnerId(TUniqueId oid) { xec_ownerId = oid; }
  EWeaponType GetType() const { return xf0_weaponType; }
  const CDamageInfo& GetDamageInfo() const { return x12c_curDamageInfo; }
  CDamageInfo& DamageInfo() { return x12c_curDamageInfo; }
  void SetDamageInfo(const CDamageInfo& dInfo) { x12c_curDamageInfo = dInfo; }
  float GetDamageDuration() const { return x150_damageDuration; }
  void SetDamageDuration(float dur) { x150_damageDuration = dur; }
  float GetInterferenceDuration() const { return x154_interferenceDuration; }
  void SetInterferenceDuration(float dur) { x154_interferenceDuration = dur; }

  void Think(float, CStateManager&) override;
  void Render(const CStateManager&) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;
};
} // namespace urde
