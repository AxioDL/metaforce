#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "Runtime/rstl.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"

namespace urde {

class CNewFlameThrower : public CGameProjectile {
  CRandom16 x2e8_rand{99};
  float x2ec_ = 0.f;
  float x2f0_ = 0.f;
  bool x300_ = false;
  TCachedToken<CGenDescription> x304_mainFire;
  TCachedToken<CGenDescription> x310_mainSmoke;
  TCachedToken<CGenDescription> x31c_secondarySmoke;
  TCachedToken<CGenDescription> x328_secondaryFire;
  TCachedToken<CGenDescription> x334_secondarySparks;
  TCachedToken<CSwooshDescription> x340_swooshCenter;
  TCachedToken<CSwooshDescription> x34c_swooshFire;
  std::unique_ptr<CElementGen> x358_mainFireGen;
  std::unique_ptr<CElementGen> x35c_mainSmokeGen;
  std::unique_ptr<CElementGen> x360_secondarySmokeGen;
  std::unique_ptr<CElementGen> x364_secondaryFireGen;
  std::unique_ptr<CElementGen> x368_secondarySparksGen;
  std::unique_ptr<CParticleSwoosh> x36c_swooshCenterGen;
  std::unique_ptr<CParticleSwoosh> x370_swooshFireGen;
  u32 x374_ = 0;
  TAreaId x378_ = kInvalidAreaId;
  bool x37c_24_renderAuxEffects : 1;
  bool x37c_25_firing : 1;
  bool x37c_26_ : 1;
  bool x37c_27_ : 1;
  bool x37c_28_ : 1;
  rstl::reserved_vector<std::vector<std::pair<float, u32>>, 3> x380_;
  u32 x3b4_ = 0;
  rstl::reserved_vector<TUniqueId, 4> x3b8_lightIds;
  void DeleteLightObjects(CStateManager& mgr);
  void CreateLightObjects(CStateManager& mgr);
  void EnableFx(CStateManager& mgr);

public:
  // Resinfo:
  //  NFTMainFire
  //  NFTMainSmoke
  //  NFTSwooshCenter
  //  NFTSwooshFire
  //  NFTSecondarySmoke
  //  NFTSecondaryFire
  //  NFTSecondarySparks
  //  <invalid>
  CNewFlameThrower(const TToken<CWeaponDescription>& desc, std::string_view name, EWeaponType wType,
                   const CAssetId resInfo[8], const zeus::CTransform& xf, EMaterialTypes matType,
                   const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner, EProjectileAttrib attribs);
  void StartFiring(const zeus::CTransform& xf, CStateManager& mgr);
  bool CanRenderAuxEffects() const { return x37c_24_renderAuxEffects; }
  bool IsFiring() const { return x37c_25_firing; }
  bool AreEffectsFinished() const;
  void UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr);
  void Reset(CStateManager& mgr, bool deactivate);
};

} // namespace urde
