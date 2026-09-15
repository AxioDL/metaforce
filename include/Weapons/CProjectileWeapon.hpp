#ifndef _CPROJECTILEWEAPON_HPP
#define _CPROJECTILEWEAPON_HPP

#include <Kyoto/CRandom16.hpp>
#include <Kyoto/Graphics/CColor.hpp>
#include <Kyoto/Math/CTransform4f.hpp>
#include <Kyoto/Particles/CElementGen.hpp>
#include <Kyoto/Particles/CParticleSwoosh.hpp>

#include <Kyoto/TToken.hpp>

#include <rstl/single_ptr.hpp>

#include "Kyoto/Math/CMatrix4f.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CDecalDescription.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "Weapons/IWeaponProjectile.hpp"
#include "rstl/optional_object.hpp"

class CProjectileWeapon : public IWeaponProjectile {
  static uint skGlobalSeed;
  CProjectileWeapon(const TToken< CWeaponDescription >& description, const CVector3f& worldOffset,
                    const CTransform4f& localToWorld, const CVector3f& scale, int flags);
  ~CProjectileWeapon();

  static float GetTickPeriod();

  bool Update(float dt);
  void UpdateParticleFX();
  const CTransform4f GetTransform() const;
  CTransform4f GetTransform();
  const CVector3f GetTranslation() const;
  void SetRelativeOrientation(const CTransform4f& orient);
  void SetWorldSpaceOrientation(const CTransform4f& orient);
  void UpdatePSTranslationAndOrientation();
  void UpdateChildParticleSystems(float dt);
  const bool IsSystemDeletable() const;
  void Render() const;
  void AddToRenderer() const;
  void RenderParticles() const;
  rstl::optional_object< TLockedToken< CGenDescription > >
  CollisionOccured(const EWeaponCollisionResponseTypes colType, const bool deflected,
                    const bool useTarget, const CVector3f& pos, const CVector3f& normal,
                    const CVector3f& target);

  uint GetSoundIdForCollision(EWeaponCollisionResponseTypes type) const;

  rstl::optional_object< TLockedToken< CDecalDescription > >
  GetDecalForCollision(EWeaponCollisionResponseTypes type) const;

  float GetAudibleRange() const;
  float GetAudibleFallOff() const;
  float GetMaxTurnRate() const;
  void SetVelocity(const CVector3f& velocity);
  const CVector3f& GetVelocity() const;
  void SetGravity(const CVector3f& gravity);
  const CVector3f& GetGravity() const;
  static void SetGlobalSeed(const uint seed);

  rstl::optional_object< CAABox > GetBounds() const;

private:
  TLockedToken< CWeaponDescription > x4_weaponDesc;
  CRandom16 x10_random;
  CTransform4f x14_localToWorldXf;
  CTransform4f x44_localXf;
  CVector3f x74_worldOffset;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  CVector3f x80_previousLocalOffset;
#endif
  CVector3f x80_localOffset;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  CVector3f x98_interpolationOffset;
#endif
  CVector3f x8c_projOffset;
  CVector3f x98_scale;
  CVector3f xa4_localOffset2;
  CVector3f xb0_velocity;
  CVector3f xbc_gravity;
  CColor xc8_ambientLightColor;
  double xd0_curTime;
  double xd8_remainderTime;
  float xe0_maxTurnRate;
  int xe4_flags;
  int xe8_lifetime;
  int xec_childSystemUpdateRate;
  int xf0_;
  int xf4_curFrame;
  int xf8_lastParticleFrame;
  CElementGen* xfc_APSMGen;
  CElementGen* x100_APS2Gen;
  CElementGen* x104_;
  rstl::optional_object< TLockedToken< CModel > > x108_model;
  CParticleSwoosh* x118_swoosh1;
  CParticleSwoosh* x11c_swoosh2;
  CParticleSwoosh* x120_swoosh3;
  bool x124_24_active : 1;
  bool x124_25_APSO : 1;
  bool x124_26_AP11 : 1;
  bool x124_27_AP21 : 1;
  bool x124_28_AS11 : 1;
  bool x124_29_AS12 : 1;
  bool x124_30_AS13 : 1;
  bool x124_31_VMD2 : 1;
};
CHECK_SIZEOF(CProjectileWeapon,
             (VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02 ? 0x140 : 0x128))

#endif // _CPROJECTILEWEAPON_HPP
