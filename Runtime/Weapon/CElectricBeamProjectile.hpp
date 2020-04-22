#pragma once

#include <memory>

#include "Runtime/Weapon/CBeamProjectile.hpp"

namespace urde {
struct SElectricBeamInfo {
  TToken<CElectricDescription> x0_electricDescription;
  float x8_maxLength;
  float xc_radius;
  float x10_travelSpeed;
  CAssetId x14_particleId;
  float x18_;
  float x1c_;
};

class CElectricBeamProjectile : public CBeamProjectile {
  std::unique_ptr<CParticleElectric> x468_electric;
  TCachedToken<CGenDescription> x46c_genDescription;
  std::unique_ptr<CElementGen> x478_elementGen;
  float x47c_;
  float x480_;
  float x484_ = 0.f;
  float x488_;
  bool x48c_ = false;

public:
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
} // namespace urde
