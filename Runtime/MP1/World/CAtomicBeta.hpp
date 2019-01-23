#pragma once
#include "World/CPatterned.hpp"

namespace urde {
class CWeaponDescription;
}

namespace urde::MP1 {
class CAtomicBeta final : public CPatterned {
  static const std::string_view skBombLocators[3];
  static constexpr  u32 kBombCount = 3;
  rstl::reserved_vector<TUniqueId, kBombCount> x568_projectileIds;
  bool x574_beamFired = false;
  float x578_;
  float x57c_;
  float x580_;
  float x584_;
  CDamageVulnerability x588_frozenDamage;
  float x5f0_;
  zeus::CVector3f x5f4_;
  TToken<CElectricDescription> x600_electricWeapon;
  TToken<CWeaponDescription> x608_;
  CDamageInfo x610_projectileDamage;
  CAssetId x62c_beamParticle;
  float x630_;
  float x634_beamRadius;
  float x638_;
  float x63c_ = 1.f;
  float x640_ = 10.f;
  u16 x644_;
  u16 x646_;
  u16 x648_;
  CSfxHandle x64c_;
  CSfxHandle x650_;
  CSfxHandle x654_;

  void CreateBeams(CStateManager&);
  void UpdateBeams(CStateManager&, bool);
  void FreeBeams(CStateManager&);
  void UpdateOrCreateEmitter(CSfxHandle&, u16, const zeus::CVector3f&, float);
  void DestroyEmitter(CSfxHandle&);

public:
  DEFINE_PATTERNED(AtomicBeta)
  CAtomicBeta(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
              const CActorParameters&, const CPatternedInfo&, CAssetId, CAssetId, const CDamageInfo&, CAssetId, float,
              float, float, const CDamageVulnerability&, float, float, float, s16, s16, s16, float);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);

  void Think(float, CStateManager&);
  const CDamageVulnerability* GetDamageVulnerability() const;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode& mode) const {
    return GetDamageVulnerability()->WeaponHits(mode, false) ? EWeaponCollisionResponseTypes::AtomicBeta
                                                             : EWeaponCollisionResponseTypes::AtomicBetaReflect;
  }
  void Death(CStateManager&, const zeus::CVector3f&, EScriptObjectState);
  static bool IsPlayerBeamChargedEnough(const CStateManager& mgr);
};
} // namespace urde::MP1