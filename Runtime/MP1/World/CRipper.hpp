#pragma once

#include "Runtime/World/CGrappleParameters.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CScriptPlatform.hpp"

namespace urde::MP1 {

class CRipper : public CPatterned {
  CGrappleParameters x568_grappleParams;
  TUniqueId x598_grapplePoint = kInvalidUniqueId;
  TUniqueId x59a_platformId = kInvalidUniqueId;
  bool x59c_24_muted : 1 = false;

  void ProcessGrapplePoint(CStateManager&);
  void AddGrapplePoint(CStateManager&);
  void RemoveGrapplePoint(CStateManager&);
  void AddPlatform(CStateManager&);
  void RemovePlatform(CStateManager&);
public:
  DEFINE_PATTERNED(Ripper)
  CRipper(TUniqueId uid, std::string_view name, EFlavorType type, const CEntityInfo& info, const zeus::CTransform& xf,
          CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
          const CGrappleParameters& grappleParms);

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode& wp, EProjectileAttrib) const override {
    if (!GetDamageVulnerability()->WeaponHits(wp, false))
      return EWeaponCollisionResponseTypes::Unknown82;

    return EWeaponCollisionResponseTypes::Unknown32;
  }

  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo&, EKnockBackType, bool, float) override;
  void Patrol(CStateManager&, EStateMsg, float) override;
  bool PathOver(CStateManager&, float) override { return false; } /* They never give you up, or let you down */
};

class CRipperControlledPlatform : public CScriptPlatform {
  TUniqueId x358_owner;
  float x35c_yaw;
public:
  CRipperControlledPlatform(TUniqueId, TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&,
                            const zeus::CAABox&, bool, const std::optional<TLockedToken<CCollidableOBBTreeGroup>>&);
};
}