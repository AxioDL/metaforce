#pragma once

#include <memory>

#include "Runtime/Weapon/CBeamProjectile.hpp"

namespace metaforce {
struct SElectricBeamInfo {
  TToken<CElectricDescription> x0_electricDescription;
  float x8_maxLength;
  float xc_radius;
  float x10_travelSpeed;
  CAssetId x14_particleId;
  float x18_fadeSpeed;
  float x1c_damageInterval;
};

class CElectricBeamProjectile : public CBeamProjectile {
  std::unique_ptr<CParticleElectric> x468_electric;
  TCachedToken<CGenDescription> x46c_genDescription;
  std::unique_ptr<CElementGen> x478_elementGen;
  float x47c_fadeSpeed;
  float x480_intensity;
  float x484_damageTimer = 0.f;
  float x488_damageInterval;
  bool x48c_ = false;

public:
  DEFINE_ENTITY
  CElectricBeamProjectile(const TToken<CWeaponDescription>&, EWeaponType, const SElectricBeamInfo&,
                          const zeus::CTransform&, EMaterialTypes, const CDamageInfo&, TUniqueId, TAreaId, TUniqueId,
                          EProjectileAttrib);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void Touch(CActor&, CStateManager&) override {}
  void UpdateFx(const zeus::CTransform&, float, CStateManager&) override;
  void ResetBeam(CStateManager&, bool) override;
  void Fire(const zeus::CTransform&, CStateManager&, bool) override;
};
} // namespace metaforce
