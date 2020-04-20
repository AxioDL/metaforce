#pragma once

#include <memory>

#include "Runtime/Weapon/CGunWeapon.hpp"

namespace urde {

class CIceBeam final : public CGunWeapon {
  TCachedToken<CGenDescription> x21c_iceSmoke;
  TCachedToken<CGenDescription> x228_ice2nd1;
  TCachedToken<CGenDescription> x234_ice2nd2;
  std::unique_ptr<CElementGen> x240_smokeGen;
  std::unique_ptr<CElementGen> x244_chargeFx;
  bool x248_24_loaded : 1 = false;
  bool x248_25_inEndFx : 1 = false;
  void ReInitVariables();

public:
  CIceBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
           const zeus::CVector3f& scale);

  void PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) override;
  void PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) override;
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf) override;
  void Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf, CStateManager& mgr,
            TUniqueId homingTarget, float chargeFactor1, float chargeFactor2) override;
  void EnableFx(bool enable) override;
  void EnableSecondaryFx(ESecondaryFxType type) override;
  void Update(float dt, CStateManager& mgr) override;
  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;
};

} // namespace urde
