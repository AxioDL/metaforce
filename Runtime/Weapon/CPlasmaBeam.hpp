#pragma once

#include <memory>
#include "Runtime/Weapon/CGunWeapon.hpp"

namespace urde {

class CPlasmaBeam final : public CGunWeapon {
  TCachedToken<CGenDescription> x21c_plasma2nd1;
  std::unique_ptr<CElementGen> x228_chargeFx;
  bool x22c_24_loaded : 1 = false;
  bool x22c_25_worldLighingDim : 1 = false;
  float x230_fireShotDelayTimer = 0.f;
  float x234_fireShotDelay = 0.f;
  float x238_lightingResetDelayTimer = 0.f;
  TAreaId x23c_stateArea = kInvalidAreaId;
  void ReInitVariables();
  void SetWorldLighting(CStateManager& mgr, TAreaId aid, float speed, float target);

public:
  CPlasmaBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
              const zeus::CVector3f& scale);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
  bool IsFiring() const { return x234_fireShotDelay > 0.f; }
  void StopBeam(CStateManager& mgr, bool b1) { /* Empty */
  }
  void CreateBeam(CStateManager& mgr) { /* Empty */
  }
  void UpdateBeam(float dt, const zeus::CTransform& targetXf, const zeus::CVector3f& localBeamPos,
                  CStateManager& mgr) { /* Empty */
  }
  void DeleteBeam(CStateManager& mgr);

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
