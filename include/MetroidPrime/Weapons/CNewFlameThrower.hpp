#ifndef _CNEWFLAMETHROWER
#define _CNEWFLAMETHROWER

#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "rstl/single_ptr.hpp"

struct CWeaponAssetInfo;
class CElementGen;
class CParticleSwoosh;
class CSwooshDescription;
class CCollisionInfo;
class CCollisionInfoList;
class CCollisionPrimitive;

class CNewFlameThrower : public CGameProjectile {
public:
  // CEntity
  ~CNewFlameThrower() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  CNewFlameThrower(const TToken< CWeaponDescription >& desc, const rstl::string& name,
                   EWeaponType wType, const CWeaponAssetInfo& resInfo,
                   const CTransform4f& xf, EMaterialTypes matType, const CDamageInfo& dInfo,
                   TUniqueId uid, TAreaId aid, TUniqueId owner, uint attribs);

  void Fire(const CTransform4f& xf, CStateManager& mgr, bool unused);
  void Reset(CStateManager& mgr, bool deactivate);
  void UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr);
  const bool AreEffectsFinished() const;
  const bool CanRenderAuxEffects() const { return mRenderAuxEffects; }
  const bool IsFiring() const { return mFiring; }

private:
  struct SSortedListEntry {
    float mPosition;
    int mRemainingTime;
    SSortedListEntry(float position, int remainingTime)
    : mPosition(position), mRemainingTime(remainingTime) {}
    bool operator<(const SSortedListEntry& other) const { return mPosition < other.mPosition; }
  };
  enum EFlameState {
    kFS_Default,
    kFS_FireStart,
    kFS_FireActive,
    kFS_FireStopTimer,
    kFS_FireWaitForParticlesDone
  };

  float UpdateFlameState(float dt, CStateManager& mgr);
  void CreateFlameParticles(CStateManager& mgr);
  bool DoCollisionCheck(CStateManager& mgr, rstl::reserved_vector< CSphere, 32 >& collisions,
                        float dt);
  bool DynamicObjectCollision(CStateManager& mgr, const TEntityList& nearList,
                              const CCollisionPrimitive& primitive, TUniqueId& firstCollision,
                              CCollisionInfoList& collisions);
  void CreateLights(CStateManager& mgr);
  void UpdateLights(CStateManager& mgr);
  void DestroyLights(CStateManager& mgr);
  void SetLightsActive(CStateManager& mgr, bool active);
  void AddToSortedLists(const CCollisionInfo& info, int time);
  int GetApproxNumSortedListPointsInSphere(const CSphere& sphere);
  void RemoveDeadEntriesFromSortedLists();
  void SetWorldDarkening(CStateManager& mgr, TAreaId area, float speed, float target);
  bool CanDamage(CActor& actor, CStateManager& mgr);

  CRandom16 mRand;
  float mParticlesDoneTimer;
  float mFlamesDoneTimer;
  rstl::optional_object< CVector3f > mLastParticleCollisionLoc;
  TLockedToken< CGenDescription > mMainFire;
  TLockedToken< CGenDescription > mMainSmoke;
  TLockedToken< CGenDescription > mSecondarySmoke;
  TLockedToken< CGenDescription > mSecondaryFire;
  TLockedToken< CGenDescription > mSecondarySparks;
  TLockedToken< CSwooshDescription > mSwooshCenter;
  TLockedToken< CSwooshDescription > mSwooshFire;
  rstl::single_ptr< CElementGen > mMainFireGen;
  rstl::single_ptr< CElementGen > mMainSmokeGen;
  rstl::single_ptr< CElementGen > mSecondarySmokeGen;
  rstl::single_ptr< CElementGen > mSecondaryFireGen;
  rstl::single_ptr< CElementGen > mSecondarySparksGen;
  rstl::single_ptr< CParticleSwoosh > mSwooshCenterGen;
  rstl::single_ptr< CParticleSwoosh > mSwooshFireGen;
  EFlameState mFlameState;
  TAreaId mCurrentLitArea;
  bool mRenderAuxEffects : 1;
  bool mFiring : 1;
  bool mRunningSlowish : 1;
  bool mNewPointAdded : 1;
  bool mActiveLighting : 1;
  rstl::reserved_vector< rstl::vector< SSortedListEntry >, 3 > mFlameContactPoints;
  int mNumSmokeParticlesSpawned;
  rstl::reserved_vector< TUniqueId, 4 > mLightIds;
};
CHECK_SIZEOF(CNewFlameThrower, (VERSION >= VERSION_GM8P_00 ? 0x3f0
                                : VERSION >= VERSION_GM8E_02                             ? 0x3d8
                                                                                         : 0x3c8))

#endif // _CNEWFLAMETHROWER
