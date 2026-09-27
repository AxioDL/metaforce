#ifndef _CWAVEBEAM
#define _CWAVEBEAM

#include "Kyoto/Math/CRelAngle.hpp"
#include "types.h"

#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include "rstl/single_ptr.hpp"

class CElectricDescription;
class CParticleElectric;

class CWaveBeam : public CGunWeapon {
public:
  CWaveBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
            EMaterialTypes playerMaterial, const CVector3f& scale);
  ~CWaveBeam() override;

  void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                   const CTransform4f& xf) override;
  void Fire(const bool underwater, const float dt, CPlayerState::EChargeStage chargeState,
            const CTransform4f& xf, CStateManager& mgr, const TUniqueId homingTarget,
            const float chargeFactor1, const float chargeFactor2) override;
  void EnableSecondaryFx(const ESecondaryFxType type) override;
  void Update(float dt, CStateManager& mgr) override;
  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;

private:
  TCachedToken< CWeaponDescription > mWaveBeam;
  TCachedToken< CElectricDescription > mWave2nd1;
  TCachedToken< CElectricDescription > mWave2nd2;
  TCachedToken< CGenDescription > mWave2nd3;
  float mEffectTimer;
  rstl::single_ptr< CParticleElectric > mChargeElec;
  rstl::single_ptr< CElementGen > mChargeFx;
  bool mLoaded : 1;
  bool mEffectTimerActive : 1;

  void ReInitVariables();
};
CHECK_SIZEOF(CWaveBeam, 0x25c)

#endif // _CWAVEBEAM
