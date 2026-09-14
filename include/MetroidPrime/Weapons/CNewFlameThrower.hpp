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
  const bool CanRenderAuxEffects() const { return x37c_24_renderAuxEffects; }
  const bool IsFiring() const { return x37c_25_firing; }

private:
  struct SSortedListEntry {
    float x0_position;
    int x4_remainingTime;
    SSortedListEntry(float position, int remainingTime)
    : x0_position(position), x4_remainingTime(remainingTime) {}
    bool operator<(const SSortedListEntry& other) const { return x0_position < other.x0_position; }
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

  CRandom16 x2e8_rand;
  float x2ec_particlesDoneTimer;
  float x2f0_flamesDoneTimer;
  rstl::optional_object< CVector3f > x2f4_lastParticleCollisionLoc;
  TLockedToken< CGenDescription > x304_mainFire;
  TLockedToken< CGenDescription > x310_mainSmoke;
  TLockedToken< CGenDescription > x31c_secondarySmoke;
  TLockedToken< CGenDescription > x328_secondaryFire;
  TLockedToken< CGenDescription > x334_secondarySparks;
  TLockedToken< CSwooshDescription > x340_swooshCenter;
  TLockedToken< CSwooshDescription > x34c_swooshFire;
  rstl::single_ptr< CElementGen > x358_mainFireGen;
  rstl::single_ptr< CElementGen > x35c_mainSmokeGen;
  rstl::single_ptr< CElementGen > x360_secondarySmokeGen;
  rstl::single_ptr< CElementGen > x364_secondaryFireGen;
  rstl::single_ptr< CElementGen > x368_secondarySparksGen;
  rstl::single_ptr< CParticleSwoosh > x36c_swooshCenterGen;
  rstl::single_ptr< CParticleSwoosh > x370_swooshFireGen;
  EFlameState x374_flameState;
  TAreaId x378_currentLitArea;
  bool x37c_24_renderAuxEffects : 1;
  bool x37c_25_firing : 1;
  bool x37c_26_runningSlowish : 1;
  bool x37c_27_newPointAdded : 1;
  bool x37c_28_activeLighting : 1;
  rstl::reserved_vector< rstl::vector< SSortedListEntry >, 3 > x380_flameContactPoints;
  int x3b4_numSmokeParticlesSpawned;
  rstl::reserved_vector< TUniqueId, 4 > x3b8_lightIds;
};
CHECK_SIZEOF(CNewFlameThrower, (VERSION >= VERSION_GM8P_00 ? 0x3d8 : 0x3c8))

#endif // _CNEWFLAMETHROWER
