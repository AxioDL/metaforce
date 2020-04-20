#pragma once

#include <memory>
#include <optional>

#include "Runtime/CRandom16.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Collision/CCollisionResponseData.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CDecalDescription;
class CGenDescription;
class CModel;

class CProjectileWeapon {
  static u16 g_GlobalSeed;
  TLockedToken<CWeaponDescription> x4_weaponDesc;
  CRandom16 x10_random;
  zeus::CTransform x14_localToWorldXf;
  zeus::CTransform x44_localXf;
  zeus::CVector3f x74_worldOffset;
  zeus::CVector3f x80_localOffset = zeus::skZero3f;
  zeus::CVector3f x8c_projOffset = zeus::skZero3f;
  zeus::CVector3f x98_scale = zeus::skOne3f;
  zeus::CVector3f xa4_localOffset2 = zeus::skZero3f;
  zeus::CVector3f xb0_velocity = zeus::skZero3f;
  zeus::CVector3f xbc_gravity = zeus::skZero3f;
  zeus::CColor xc8_ambientLightColor = zeus::skWhite;
  double xd0_curTime = 0.0;
  double xd8_remainderTime = 0.0;
  float xe0_maxTurnRate = 0.f;
  int xe4_flags;
  int xe8_lifetime = 0;
  int xec_childSystemUpdateRate = 0;
  int xf0_ = 0;
  int xf4_curFrame = 0;
  int xf8_lastParticleFrame = -1;
  std::unique_ptr<CElementGen> xfc_APSMGen;
  std::unique_ptr<CElementGen> x100_APS2Gen;
  std::unique_ptr<CElementGen> x104_;
  std::optional<TLockedToken<CModel>> x108_model;
  std::unique_ptr<CParticleSwoosh> x118_swoosh1;
  std::unique_ptr<CParticleSwoosh> x11c_swoosh2;
  std::unique_ptr<CParticleSwoosh> x120_swoosh3;
  bool x124_24_active : 1 = true;
  bool x124_25_APSO : 1 = false;
  bool x124_26_AP11 : 1 = false;
  bool x124_27_AP21 : 1 = false;
  bool x124_28_AS11 : 1 = false;
  bool x124_29_AS12 : 1 = false;
  bool x124_30_AS13 : 1 = false;
  bool x124_31_VMD2 : 1 = false;

public:
  CProjectileWeapon(const TToken<CWeaponDescription>& wDesc, const zeus::CVector3f& worldOffset,
                    const zeus::CTransform& orient, const zeus::CVector3f& scale, s32);
  virtual ~CProjectileWeapon() = default;
  bool IsProjectileActive() const { return x124_24_active; }
  std::optional<zeus::CAABox> GetBounds() const;
  const zeus::CVector3f& GetVelocity() const { return xb0_velocity; }
  void SetVelocity(const zeus::CVector3f& vel) { xb0_velocity = vel; }
  float GetMaxTurnRate() const { return xe0_maxTurnRate; }
  float GetAudibleFallOff() const;
  float GetAudibleRange() const;
  std::optional<TLockedToken<CDecalDescription>>
  GetDecalForCollision(EWeaponCollisionResponseTypes type) const;
  s32 GetSoundIdForCollision(EWeaponCollisionResponseTypes type) const;
  std::optional<TLockedToken<CGenDescription>> CollisionOccured(EWeaponCollisionResponseTypes type,
                                                                              bool deflected, bool useTarget,
                                                                              const zeus::CVector3f& pos,
                                                                              const zeus::CVector3f& normal,
                                                                              const zeus::CVector3f& target);
  TLockedToken<CWeaponDescription> GetWeaponDescription() const { return x4_weaponDesc; }
  void RenderParticles() const;
  virtual void AddToRenderer();
  virtual void Render();
  bool IsSystemDeletable() const;
  void UpdateChildParticleSystems(float);
  void UpdatePSTranslationAndOrientation();
  void SetWorldSpaceOrientation(const zeus::CTransform& xf);
  void SetRelativeOrientation(const zeus::CTransform& xf) { x44_localXf = xf; }
  virtual zeus::CVector3f GetTranslation() const;
  virtual zeus::CTransform GetTransform() const;
  void UpdateParticleFX();
  virtual void Update(float dt);
  void SetGravity(const zeus::CVector3f& grav) { xbc_gravity = grav; }
  zeus::CVector3f GetGravity() const { return xbc_gravity; }
  static void SetGlobalSeed(u16 seed) { g_GlobalSeed = seed; }
  CElementGen* GetAttachedPS1() const { return xfc_APSMGen.get(); }
  double GameTime() const { return xd0_curTime; }
  static constexpr float GetTickPeriod() { return 0.0166667f; }
};
} // namespace urde
