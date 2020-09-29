#pragma once

#include <memory>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Weapon/CGunWeapon.hpp"

namespace urde {

class CWaveBeam final : public CGunWeapon {
  TCachedToken<CWeaponDescription> x21c_waveBeam;
  TCachedToken<CElectricDescription> x228_wave2nd1;
  TCachedToken<CElectricDescription> x234_wave2nd2;
  TCachedToken<CGenDescription> x240_wave2nd3;
  float x24c_effectTimer = 0.f;
  std::unique_ptr<CParticleElectric> x250_chargeElec;
  std::unique_ptr<CElementGen> x254_chargeFx;
  bool x258_24_loaded : 1 = false;
  bool x258_25_effectTimerActive : 1 = false;
  void ReInitVariables();

public:
  CWaveBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
            const zeus::CVector3f& scale);

  void PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) override;
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf) override;
  void Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf, CStateManager& mgr,
            TUniqueId homingTarget, float chargeFactor1, float chargeFactor2) override;
  void EnableSecondaryFx(ESecondaryFxType type) override;
  void Update(float dt, CStateManager& mgr) override;
  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;
};

} // namespace urde
