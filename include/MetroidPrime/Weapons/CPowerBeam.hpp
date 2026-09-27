#ifndef _CPOWERBEAM
#define _CPOWERBEAM

#include "types.h"

#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include "rstl/single_ptr.hpp"

class CPowerBeam : public CGunWeapon {
public:
  CPowerBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
             EMaterialTypes playerMaterial, const CVector3f& scale);
  ~CPowerBeam();

  void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                   const CTransform4f& xf) override;
  void Fire(const bool underwater, const float dt, CPlayerState::EChargeStage chargeState,
            const CTransform4f& xf, CStateManager& mgr, const TUniqueId homingTarget,
            const float chargeFactor1, const float chargeFactor2) override;
  void EnableSecondaryFx(ESecondaryFxType type) override;
  void Update(const float dt, CStateManager& mgr) override;
  void Load(CStateManager& mgr, const bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;

private:
  enum ESmokeState { kSS_Inactive, kSS_Active, kSS_Done };
  TCachedToken< CGenDescription > mShotSmoke;
  TCachedToken< CGenDescription > mPower2nd1;
  rstl::single_ptr< CElementGen > mShotSmokeGen;
  rstl::single_ptr< CElementGen > mPower2ndGen;
  float mSmokeTimer;
  ESmokeState mSmokeState;
  bool x244_24 : 1;
  bool mLoaded : 1;

  void ReInitVariables();
};
CHECK_SIZEOF(CPowerBeam, 0x248)

#endif // _CPOWERBEAM
