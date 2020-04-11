#pragma once

#include <memory>

#include "Runtime/CRandom16.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"

#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {

class CWaveBuster : public CGameProjectile {
  zeus::CTransform x2e8_originalXf;
  zeus::CVector3f x318_;
  zeus::CVector3f x324_;
  zeus::CVector3f x330_;
  zeus::CVector3f x33c_;
  zeus::CVector3f x348_targetPoint;
  TCachedToken<CSwooshDescription> x354_busterSwoosh1;
  TCachedToken<CSwooshDescription> x360_busterSwoosh2;
  TCachedToken<CGenDescription> x36c_busterSparks;
  TCachedToken<CGenDescription> x378_busterLight;
  std::unique_ptr<CParticleSwoosh> x384_busterSwoosh1Gen;
  std::unique_ptr<CParticleSwoosh> x388_busterSwoosh2Gen;
  std::unique_ptr<CElementGen> x38c_busterSparksGen;
  std::unique_ptr<CElementGen> x390_busterLightGen;
  CRandom16 x394_rand{99};
  float x39c_ = 0.5f;
  float x3a0_ = 0.5f;
  float x3a4_ = 0.f;
  float x3a8_ = 0.f;
  float x3ac_ = 0.f;
  float x3b0_ = 0.f;
  float x3b4_ = 0.f;
  float x3b8_ = 0.f;
  float x3bc_ = 0.f;
  float x3c0_ = 0.f;
  float x3c4_ = 0.f;
  float x3c8_ = 0.f;
  u32 x3cc_ = 0;
  bool x3d0_24_firing : 1;
  bool x3d0_25_ : 1;
  bool x3d0_26_ : 1;
  bool x3d0_27_ : 1;
  bool x3d0_28_ : 1;

public:
  CWaveBuster(const TToken<CWeaponDescription>& desc, EWeaponType type, const zeus::CTransform& xf,
              EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
              TUniqueId homingTarget, EProjectileAttrib attrib);
  bool IsFiring() const { return x3d0_24_firing; }
  void UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr);
  void ResetBeam(bool deactivate);
  void SetNewTarget(TUniqueId id);

  std::optional<zeus::CAABox> GetTouchBounds() const override;
};

} // namespace urde
