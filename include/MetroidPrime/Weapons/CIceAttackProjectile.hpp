#ifndef _CICEATTACKPROJECTILE
#define _CICEATTACKPROJECTILE

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CElementGen;
class CGenDescription;
class CPhysicsActor;

class CIceAttackProjectile : public CActor {
public:
  // CEntity
  ~CIceAttackProjectile() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  CIceAttackProjectile(TToken< CGenDescription > trail, TToken< CGenDescription > explosion,
                       TToken< CGenDescription > moving, TUniqueId uid, TAreaId area,
                       TUniqueId owner, const bool active, const CTransform4f& xf,
                       const CDamageInfo& damage, const CAABox& bounds, float speed,
                       float turnSpeed, CAssetId steamTexture, ushort freezeSfx,
                       ushort explosionSfx, CAssetId iceTexture);

  void SetUseWorldRay(bool use) { x192_useWorldRay = use; }

private:
  class CTrailObject {
  public:
    CTrailObject(CElementGen* gen, TUniqueId collisionId, const CVector3f& position,
                 const CVector3f& normal, const CVector3f& step);
    void StartExplosion(CElementGen* gen, CStateManager& mgr);
    void DeleteCollisionObject(CStateManager& mgr);
    void Update(float dt, CStateManager& mgr, const CPhysicsActor* owner);
    void AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const;
    bool ExplosionStarted() const;
    bool ExplosionFinished() const;
    CActorLights& ActorLights() { return x18_actorLights; }

  private:
    rstl::auto_ptr< CElementGen > x0_trail;
    rstl::auto_ptr< CElementGen > x8_explosion;
    TUniqueId x10_collisionObj;
    float x14_elapsed;
    CActorLights x18_actorLights;
    CVector3f x2f8_position;
    CVector3f x304_normal;
    CVector3f x310_step;
    uint x31c_createdParticles;
    bool x320_collisionActive;
  };

  void CreateTrailObject(CStateManager& mgr, const CVector3f& normal, float dt);
  void UpdateTrailObjects(float dt, CStateManager& mgr);

  TToken< CGenDescription > xe8_trailDesc;
  TToken< CGenDescription > xf0_explosionDesc;
  TToken< CGenDescription > xf8_movingDesc;
  rstl::auto_ptr< CElementGen > x100_movingGen;
  rstl::vector< CTrailObject > x108_trailObjects;
  TUniqueId x118_owner;
  CDamageInfo x11c_damage;
  CDamageInfo x138_currentDamage;
  rstl::optional_object< CAABox > x154_bounds;
  float x170_speed;
  float x174_turnSpeed;
  float x178_moveTime;
  float x17c_explosionTimer;
  int x180_frameCount;
  CAssetId x184_steamTexture;
  ushort x188_freezeSfx;
  ushort x18a_explosionSfx;
  CAssetId x18c_iceTexture;
  bool x190_finishedMoving;
  bool x191_explosionSoundStarted;
  bool x192_useWorldRay;
  CSfxHandle x194_explosionSfxHandle;
};
CHECK_SIZEOF(CIceAttackProjectile, (VERSION >= VERSION_GM8P_00 ? 0x1a8 : 0x198))

#endif // _CICEATTACKPROJECTILE
