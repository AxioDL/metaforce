#ifndef _CICEBEAM
#define _CICEBEAM

#include "types.h"

#include "MetroidPrime/Weapons/CGunWeapon.hpp"

class CIceBeam : public CGunWeapon {
public:
  CIceBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
           EMaterialTypes playerMaterial, const CVector3f& scale);
  ~CIceBeam() override;

  // CGunWeapon
  void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                   const CTransform4f& xf) override;
  void Update(float dt, CStateManager& mgr) override;
  void Fire(bool underwater, float dt, CPlayerState::EChargeStage chargeState,
            const CTransform4f& xf, CStateManager& mgr, TUniqueId homingTarget, float chargeFactor1,
            float chargeFactor2) override;

  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;

  void EnableFx(bool enable) override;
  void EnableSecondaryFx(ESecondaryFxType type) override;

private:
  TCachedToken< CGenDescription > mIceSmoke;
  TCachedToken< CGenDescription > mIce2nd1;
  TCachedToken< CGenDescription > mIce2nd2;
  rstl::single_ptr< CElementGen > mSmokeGen;
  rstl::single_ptr< CElementGen > mChargeFx;
  bool mLoaded : 1;
  bool mInEndFx : 1;

  void ReInitVariables();
};
CHECK_SIZEOF(CIceBeam, 0x24c)

#endif // _CICEBEAM
