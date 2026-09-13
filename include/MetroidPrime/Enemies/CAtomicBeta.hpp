#ifndef _CATOMICBETA
#define _CATOMICBETA

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

class CElectricDescription;
class CWeaponDescription;

class CAtomicBeta : public CPatterned {
public:
  CAtomicBeta(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, CModelData mData, const CActorParameters& actParms,
              const CPatternedInfo& pInfo, CAssetId electricId, CAssetId weaponId,
              const CDamageInfo& dInfo, CAssetId particleId, float beamFadeSpeed, float beamRadius,
              float beamDamageInterval, const CDamageVulnerability& frozenDVuln, float moveSpeed,
              float minSpeed, float maxSpeed, ushort sId1, ushort sId2, ushort sId3,
              float speedStep);

  // CEntity
  ~CAtomicBeta() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  const CDamageVulnerability* GetDamageVulnerability() const override;
  void Touch(CActor& other, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

private:
  void CreateBeams(CStateManager& mgr);
  void UpdateBeams(CStateManager& mgr, bool fire);
  void DestroyBeams(CStateManager& mgr);
  void PlayLoopedSound(CSfxHandle& handle, const ushort sfxId, const CVector3f pos,
                       const uchar vol) const;
  void StopLoopedSound(CSfxHandle& handle) const;

  static bool IsCharging(const CStateManager& mgr);

  rstl::reserved_vector< TUniqueId, 3 > x568_projectileIds;
  bool x574_beamFired;
  float x578_minSpeed;
  float x57c_maxSpeed;
  float x580_speedStep;
  float x584_currentSpeed;
  CDamageVulnerability x588_frozenDamage;
  float x5f0_moveSpeed;
  CVector3f x5f4_direction;
  TToken< CElectricDescription > x600_electricWeapon;
  TToken< CWeaponDescription > x608_weaponDesc;
  CDamageInfo x610_projectileDamage;
  CAssetId x62c_beamParticle;
  float x630_beamFadeSpeed;
  float x634_beamRadius;
  float x638_beamDamageInterval;
  float x63c_;
  float x640_;
  ushort x644_sfxId1;
  ushort x646_sfxId2;
  ushort x648_sfxId3;
  CSfxHandle x64c_sfxHandle1;
  CSfxHandle x650_sfxHandle2;
  CSfxHandle x654_sfxHandle3;

  static const char* skBombLocators[];
};
CHECK_SIZEOF(CAtomicBeta, 0x658)

#endif // _CATOMICBETA
