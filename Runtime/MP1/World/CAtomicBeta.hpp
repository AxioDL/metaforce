#pragma once

#include <string_view>

#include "Runtime/rstl.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CWeaponDescription;
}

namespace metaforce::MP1 {
class CAtomicBeta final : public CPatterned {
  static constexpr u32 kBombCount = 3;
  rstl::reserved_vector<TUniqueId, kBombCount> x568_projectileIds;
  bool x574_beamFired = false;
  float x578_minSpeed;
  float x57c_maxSpeed;
  float x580_speedStep;
  float x584_currentSpeed;
  CDamageVulnerability x588_frozenDamage;
  float x5f0_moveSpeed;
  zeus::CVector3f x5f4_;
  TToken<CElectricDescription> x600_electricWeapon;
  TToken<CWeaponDescription> x608_;
  CDamageInfo x610_projectileDamage;
  CAssetId x62c_beamParticle;
  float x630_beamFadeSpeed;
  float x634_beamRadius;
  float x638_beamDamageInterval;
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
  DEFINE_PATTERNED(AtomicBeta);
  CAtomicBeta(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
              const CActorParameters&, const CPatternedInfo&, CAssetId, CAssetId, const CDamageInfo&, CAssetId, float,
              float, float, const CDamageVulnerability&, float, float, float, s16, s16, s16, float);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  void Think(float, CStateManager&) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode& mode, EProjectileAttrib) const override {
    return GetDamageVulnerability()->WeaponHits(mode, false) ? EWeaponCollisionResponseTypes::AtomicBeta
                                                             : EWeaponCollisionResponseTypes::AtomicBetaReflect;
  }
  void Death(CStateManager&, const zeus::CVector3f&, EScriptObjectState) override;
  static bool IsPlayerBeamChargedEnough(const CStateManager& mgr);
};
} // namespace metaforce::MP1
