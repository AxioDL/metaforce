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

  void SetUseWorldRay(bool use) { mUseWorldRay = use; }

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
    CActorLights& ActorLights() { return mActorLights; }

  private:
    rstl::auto_ptr< CElementGen > mTrail;
    rstl::auto_ptr< CElementGen > mExplosion;
    TUniqueId mCollisionObj;
    float mElapsed;
    CActorLights mActorLights;
    CVector3f mPosition;
    CVector3f mNormal;
    CVector3f mStep;
    uint mCreatedParticles;
    bool mCollisionActive;
  };

  void CreateTrailObject(CStateManager& mgr, const CVector3f& normal, float dt);
  void UpdateTrailObjects(float dt, CStateManager& mgr);

  TToken< CGenDescription > mTrailDesc;
  TToken< CGenDescription > mExplosionDesc;
  TToken< CGenDescription > mMovingDesc;
  rstl::auto_ptr< CElementGen > mMovingGen;
  rstl::vector< CTrailObject > mTrailObjects;
  TUniqueId mOwner;
  CDamageInfo mDamage;
  CDamageInfo mCurrentDamage;
  rstl::optional_object< CAABox > mBounds;
  float mSpeed;
  float mTurnSpeed;
  float mMoveTime;
  float mExplosionTimer;
  int mFrameCount;
  CAssetId mSteamTexture;
  ushort mFreezeSfx;
  ushort mExplosionSfx;
  CAssetId mIceTexture;
  bool mFinishedMoving;
  bool mExplosionSoundStarted;
  bool mUseWorldRay;
  CSfxHandle mExplosionSfxHandle;
};
CHECK_SIZEOF(CIceAttackProjectile, (VERSION >= VERSION_GM8E_02 ? 0x1a8 : 0x198))

#endif // _CICEATTACKPROJECTILE
