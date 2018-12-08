#pragma once

#include "CGunWeapon.hpp"

namespace urde {

class CPlasmaBeam final : public CGunWeapon {
  TCachedToken<CGenDescription> x21c_plasma2nd1;
  std::unique_ptr<CElementGen> x228_chargeFx;
  bool x22c_24_loaded : 1;
  bool x22c_25_worldLighingDim : 1;
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

  void PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf);
  void Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf, CStateManager& mgr,
            TUniqueId homingTarget, float chargeFactor1, float chargeFactor2);
  void EnableSecondaryFx(ESecondaryFxType type);
  void Update(float dt, CStateManager& mgr);
  void Load(CStateManager& mgr, bool subtypeBasePose);
  void Unload(CStateManager& mgr);
  bool IsLoaded() const;
};

} // namespace urde
