#ifndef _CPROJECTILEWEAPON_HPP
#define _CPROJECTILEWEAPON_HPP

#include "Weapons/CDecalDescription.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "Weapons/IWeaponProjectile.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/TToken.hpp"

#include "MetroidPrime/ActorCommon.hpp"

#include <rstl/optional_object.hpp>
#include <rstl/single_ptr.hpp>

class CProjectileWeapon : public IWeaponProjectile {
  static uint skGlobalSeed;

public:
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

  bool IsProjectileActive() const { return mActive; }
  TLockedToken< CWeaponDescription > GetWeaponDescription() const { return mWeaponDesc; }
  CElementGen* GetAttachedPS1() { return mAPSMGen; }
  const CElementGen* GetAttachedPS1() const { return mAPSMGen; }
  double GameTime() const { return mCurTime; }

private:
  TLockedToken< CWeaponDescription > mWeaponDesc;
  CRandom16 mRandom;
  CTransform4f mLocalToWorldXf;
  CTransform4f mLocalXf;
  CVector3f mWorldOffset;
#if VERSION >= VERSION_GM8P_00
  CVector3f mPreviousLocalOffset;
#endif
  CVector3f mLocalOffset;
#if VERSION >= VERSION_GM8P_00
  CVector3f mInterpolationOffset;
#endif
  CVector3f mProjOffset;
  CVector3f mScale;
  CVector3f mLocalOffset2;
  CVector3f mVelocity;
  CVector3f mGravity;
  CColor mAmbientLightColor;
  double mCurTime;
  double mRemainderTime;
  float mMaxTurnRate;
  int mFlags;
  int mLifetime;
  int mChildSystemUpdateRate;
  int xf0_;
  int mCurFrame;
  int mLastParticleFrame;
  CElementGen* mAPSMGen;
  CElementGen* mAPS2Gen;
  CElementGen* x104_;
  rstl::optional_object< TLockedToken< CModel > > mModel;
  CParticleSwoosh* mSwoosh1;
  CParticleSwoosh* mSwoosh2;
  CParticleSwoosh* mSwoosh3;
  bool mActive : 1;
  bool mAPSO : 1;
  bool mAP11 : 1;
  bool mAP21 : 1;
  bool mAS11 : 1;
  bool mAS12 : 1;
  bool mAS13 : 1;
  bool mVMD2 : 1;
};
CHECK_SIZEOF(CProjectileWeapon,
             (VERSION >= VERSION_GM8P_00 ? 0x140 : 0x128))

#endif // _CPROJECTILEWEAPON_HPP
