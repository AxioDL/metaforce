#ifndef _CPROJECTILEWEAPON
#define _CPROJECTILEWEAPON

#include "MetroidPrime/ActorCommon.hpp"
#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CWeaponDescription;
class CElementGen;
class CModel;
class CParticleSwoosh;
class CGenDescription;
class CDecalDescription;

class CProjectileWeapon {
public:
  // TODO ctor
  CProjectileWeapon(const TToken< CWeaponDescription >& wDesc, const CVector3f& origin,
                    const CTransform4f& orient, const CVector3f& scale, const int);
  virtual ~CProjectileWeapon();
  virtual void Update(float dt);
  virtual void AddToRenderer() const;
  virtual void Render() const;
  virtual CVector3f GetTranslation() const;
  virtual CTransform4f GetTransform() const;
  CTransform4f GetTransform();
  const CVector3f& GetVelocity() const; // { return xb0_velocity; }
  void SetWorldSpaceOrientation(const CTransform4f& xf);
  void SetVelocity(const CVector3f& velocity); // { xb0_velocity = velocity; }
  CVector3f GetGravity() const;                // { return xbc_gravity; }
  void SetGravity(const CVector3f& gravity);
  static float GetTickPeriod(); // { return 0.0166667f; }

  static void SetGlobalSeed(uint seed);
  float GetMaxTurnRate() const;
  bool IsProjectileActive() const { return x124_24_active; }
  TLockedToken< CWeaponDescription > GetWeaponDescription() const { return x4_weaponDesc; }
  CElementGen* GetAttachedPS1() { return xfc_APSMGen.get(); }
  const CElementGen* GetAttachedPS1() const { return xfc_APSMGen.get(); }
  double GameTime() const { return xd0_curTime; }
  void UpdateParticleFX();
  const bool IsSystemDeletable() const;
  void RenderParticles() const;
  uint GetSoundIdForCollision(EWeaponCollisionResponseTypes type) const;
  float GetAudibleRange() const;
  float GetAudibleFallOff() const;
  rstl::optional_object< TLockedToken< CGenDescription > >
  CollisionOccured(const EWeaponCollisionResponseTypes type, const bool deflected,
                   const bool useTarget, const CVector3f& pos, const CVector3f& normal,
                   const CVector3f& target);
  rstl::optional_object< TLockedToken< CDecalDescription > >
  GetDecalForCollision(EWeaponCollisionResponseTypes type) const;

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
  rstl::single_ptr< CElementGen > xfc_APSMGen;
  rstl::single_ptr< CElementGen > x100_APS2Gen;
  rstl::single_ptr< CElementGen > x104_;
  rstl::optional_object< TLockedToken< CModel > > x108_model;
  rstl::single_ptr< CParticleSwoosh > x118_swoosh1;
  rstl::single_ptr< CParticleSwoosh > x11c_swoosh2;
  rstl::single_ptr< CParticleSwoosh > x120_swoosh3;
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

#endif // _CPROJECTILEWEAPON
