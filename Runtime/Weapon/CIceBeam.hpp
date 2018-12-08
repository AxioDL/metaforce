#pragma once

#include "CGunWeapon.hpp"

namespace urde {

class CIceBeam final : public CGunWeapon {
  TCachedToken<CGenDescription> x21c_iceSmoke;
  TCachedToken<CGenDescription> x228_ice2nd1;
  TCachedToken<CGenDescription> x234_ice2nd2;
  std::unique_ptr<CElementGen> x240_smokeGen;
  std::unique_ptr<CElementGen> x244_chargeFx;
  bool x248_24_loaded : 1;
  bool x248_25_inEndFx : 1;
  void ReInitVariables();

public:
  CIceBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
           const zeus::CVector3f& scale);

  void PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
  void PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf);
  void Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf, CStateManager& mgr,
            TUniqueId homingTarget, float chargeFactor1, float chargeFactor2);
  void EnableFx(bool enable);
  void EnableSecondaryFx(ESecondaryFxType type);
  void Update(float dt, CStateManager& mgr);
  void Load(CStateManager& mgr, bool subtypeBasePose);
  void Unload(CStateManager& mgr);
  bool IsLoaded() const;
};

} // namespace urde
