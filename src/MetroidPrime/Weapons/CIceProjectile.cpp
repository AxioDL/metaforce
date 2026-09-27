#include "MetroidPrime/Weapons/CIceAttackProjectile.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CInternalCollisionStructure.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"

CIceAttackProjectile::CIceAttackProjectile(TToken< CGenDescription > trail,
                                           TToken< CGenDescription > explosion,
                                           TToken< CGenDescription > moving, TUniqueId uid,
                                           TAreaId area, TUniqueId owner, const bool active,
                                           const CTransform4f& xf, const CDamageInfo& damage,
                                           const CAABox& bounds, float speed, float turnSpeed,
                                           CAssetId steamTexture, ushort freezeSfx,
                                           ushort explosionSfx, CAssetId iceTexture)
: CActor(uid, active, rstl::string_l("IceAttackProjectile"),
         CEntityInfo(area, CEntity::NullConnectionList), xf, CModelData::CModelDataNull(),
         CMaterialList(kMT_Projectile, kMT_CameraPassthrough), CActorParameters::None(),
         kInvalidUniqueId)
, mTrailDesc(trail)
, mExplosionDesc(explosion)
, mMovingDesc(moving)
, mOwner(owner)
, mDamage(damage)
, mCurrentDamage(damage)
, mBounds(bounds)
, mSpeed(speed)
, mTurnSpeed(turnSpeed)
, mMoveTime(0.f)
, mExplosionTimer(0.f)
, mFrameCount(0)
, mSteamTexture(steamTexture)
, mFreezeSfx(freezeSfx)
, mExplosionSfx(explosionSfx)
, mIceTexture(iceTexture)
, mFinishedMoving(false)
, mExplosionSoundStarted(false)
, mUseWorldRay(false) {
  const CVector3f up(0.f, 0.f, 1.f);
  const CVector3f right = CVector3f::Cross(up, xf.GetForward()).AsNormalized();
  const CVector3f forward = CVector3f::Cross(right, up).AsNormalized();
  SetTransform(CTransform4f::FromColumns(right, forward, up, GetTranslation()));
  mMovingGen = rs_new CElementGen(mMovingDesc);
}

CIceAttackProjectile::~CIceAttackProjectile() {}

void CIceAttackProjectile::Touch(CActor& actor, CStateManager& mgr) {
  if (actor.GetUniqueId() == mOwner && !mgr.GetPlayer()->GetFrozenState()) {
    const ushort sfx = mFreezeSfx;
    mgr.Player()->SetFrozenState(mgr, mSteamTexture, sfx, mIceTexture);
    mgr.ApplyDamage(GetUniqueId(), actor.GetUniqueId(), GetUniqueId(), mDamage,
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                    CVector3f::Zero());
  }
}

rstl::optional_object< CAABox > CIceAttackProjectile::GetTouchBounds() const {
  if (GetActive()) {
    return mBounds->GetTransformedAABox(GetTransform());
  }
  return rstl::optional_object_null();
}

void CIceAttackProjectile::AddToRenderer(const CFrustumPlanes& planes,
                                         const CStateManager& mgr) const {
  mgr.AddDrawableActor(*this, -1000.f * CGraphics::GetViewMatrix().GetForward(),
                       GetRenderBoundsCached());
}

void CIceAttackProjectile::Render(const CStateManager& mgr) const {
  CFrustumPlanes planes;
  for (int i = 0; i < mTrailObjects.size(); ++i) {
    mTrailObjects[i].AddToRenderer(planes, mgr);
  }
  mMovingGen->Render();
}

ENTITY_ACCEPT_IMPL(CIceAttackProjectile)

void CIceAttackProjectile::CreateTrailObject(CStateManager& mgr, const CVector3f& normal,
                                             float dt) {
  const TUniqueId collisionId = mgr.AllocateUniqueId();
  CCollisionActor* collision =
      rs_new CCollisionActor(collisionId, GetCurrentAreaId(), GetUniqueId(), false, 3.f, 0.001f);
  collision->SetTransform(GetTransform());
  collision->SetTranslation(collision->GetTranslation() - CVector3f(0.f, 0.f, 1.f));
  collision->MaterialList().Add(kMT_Immovable);
  collision->MaterialList().Add(kMT_CameraPassthrough);
  CMaterialFilter filter = collision->GetMaterialFilter();
  filter.ExcludeList().Add(CMaterialList(kMT_Projectile, kMT_Character));
  collision->SetMaterialFilter(filter);
  mgr.AddObject(collision);

  CElementGen* const gen = rs_new CElementGen(mTrailDesc, CElementGen::kMOT_One);
  gen->SetLeaveLightsEnabledForModelRender(true);
  CTrailObject trail(gen, collisionId, GetTranslation(), normal,
                     GetTransform().Rotate(CVector3f(0.f, mSpeed * dt, 0.f)));
  trail.ActorLights().SetMaxAreaLights(2);
  trail.ActorLights().BuildAreaLightList(mgr, *mgr.GetWorld()->GetArea(GetCurrentAreaId()),
                                         CAABox(GetTranslation(), GetTranslation()));
  if (mTrailObjects.capacity() < mTrailObjects.size() + 1) {
    mTrailObjects.reserve(mTrailObjects.size() + 1);
  }
  mTrailObjects.push_back(trail);
}

void CIceAttackProjectile::UpdateTrailObjects(float dt, CStateManager& mgr) {
  if (mFinishedMoving) {
    mExplosionTimer += dt;
    if (mExplosionTimer > 0.05f) {
      if (!mExplosionSoundStarted) {
        mExplosionSfxHandle =
            CSfxManager::SfxStart(mExplosionSfx, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                                  true, CSfxManager::kAllAreas);
        mExplosionSoundStarted = true;
      }
      mExplosionTimer -= 0.05f;
      for (int i = 0; i < mTrailObjects.size(); ++i) {
        CTrailObject& trail = mTrailObjects[i];
        if (!trail.ExplosionStarted()) {
          trail.StartExplosion(rs_new CElementGen(mExplosionDesc), mgr);
          break;
        }
      }
    }
  }
  const CPhysicsActor* owner = static_cast< const CPhysicsActor* >(mgr.GetObjectById(mOwner));
  for (int i = 0; i < mTrailObjects.size(); ++i) {
    mTrailObjects[i].Update(dt, mgr, owner);
  }
}

void CIceAttackProjectile::Think(float dt, CStateManager& mgr) {
  bool hasFloor = false;
  bool hasWall = false;
  if (!GetActive()) {
    return;
  }
  if (!mFinishedMoving) {
    if (const CActor* owner = TCastToConstPtr< CActor >(mgr.GetObjectById(mOwner))) {
      CVector3f delta = owner->GetTranslation() - GetTranslation();
      const CVector3f flatDelta(delta.GetX(), delta.GetY(), 0.f);
      if (flatDelta.MagSquared() < 25.f) {
        mTurnSpeed = 0.f;
      }
      if (delta.CanBeNormalized()) {
        delta.Normalize();
        delta = GetTransform().TransposeRotate(delta);
        const float angle =
            CVector3f::GetAngleDiff(CVector3f(0.f, 1.f, 0.f), delta) * CMath::Sign(delta.GetX());
        const float turn = -CMath::Clamp(-mTurnSpeed * dt, angle, mTurnSpeed * dt);
        CTransform4f xf = GetTransform();
        xf.RotateLocalZ(CRelAngle::FromRadians(turn));
        SetTransform(xf);
      }
    }
    mMoveTime += dt;
    CVector3f position = GetTransform() * CVector3f(0.f, mSpeed * dt, 0.f);
    CRayCastResult floorResult;
    if (mUseWorldRay) {
      TUniqueId uid = kInvalidUniqueId;
      TEntityList nearList;
      const CMaterialFilter filter =
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor, kMT_Platform));
      const CVector3f direction(0.f, 0.f, -1.f);
      const CVector3f origin = position + CVector3f(0.f, 0.f, 0.5f);
      mgr.BuildNearList(nearList, origin, direction, 4.f, filter, nullptr);
      floorResult = mgr.RayWorldIntersection(uid, origin, direction, 4.f, filter, nearList);
    } else {
      floorResult =
          mgr.RayStaticIntersection(position, CVector3f(0.f, 0.f, -1.f), 4.f,
                                    CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor)));
    }
    if (floorResult.IsValid()) {
      hasFloor = true;
      position = floorResult.GetPoint() + CVector3f(0.f, 0.f, 1.f);
    }
    const CRayCastResult wallResult =
        mgr.RayStaticIntersection(position, GetTransform().Rotate(CVector3f(0.f, 1.f, 0.f)), 3.f,
                                  CMaterialFilter::MakeInclude(CMaterialList(kMT_Wall)));
    if (wallResult.IsValid()) {
      hasWall = true;
    }
    SetTranslation(position);
    if (mMoveTime > 3.f || hasWall || !hasFloor) {
      mFinishedMoving = true;
    }
    ++mFrameCount;
    if (mFrameCount % 4 == 0 && hasFloor) {
      CreateTrailObject(mgr, floorResult.GetPlane().GetNormal(), dt);
    }
  }
  UpdateTrailObjects(dt, mgr);
  mMovingGen->SetOrientation(GetTransform().GetRotation());
  mMovingGen->SetGlobalTranslation(GetTranslation() + 1.f * GetTransform().GetForward() -
                                       1.f * GetTransform().GetUp());
  mMovingGen->Update(dt);
  if (mFinishedMoving) {
    mMovingGen->SetParticleEmission(false);
    bool finished = mMovingGen->GetParticleCount() == 0;
    for (int i = 0; i < mTrailObjects.size(); ++i) {
      const CTrailObject& trail = mTrailObjects[i];
      if (!trail.ExplosionFinished() || !trail.ExplosionStarted()) {
        finished = false;
        break;
      }
    }
    if (finished) {
      mgr.DeleteObjectRequest(GetUniqueId());
      CSfxManager::SfxStop(mExplosionSfxHandle);
    }
  }
  mCurrentDamage = mDamage.MakeScaledForTime(dt);
}

void CIceAttackProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                           CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case kSM_Registered:
    SetActive(true);
    break;
  case kSM_Deleted:
    if (sender == kInvalidUniqueId) {
      for (int i = 0; i < mTrailObjects.size(); ++i) {
        mTrailObjects[i].DeleteCollisionObject(mgr);
      }
    }
    break;
  }
}

CIceAttackProjectile::CTrailObject::CTrailObject(CElementGen* gen, TUniqueId collisionId,
                                                 const CVector3f& position, const CVector3f& normal,
                                                 const CVector3f& step)
: mTrail(gen)
, mCollisionObj(collisionId)
, mElapsed(0.f)
, mActorLights(1, CVector3f(0.f, 0.f, 1.f), 4, 4)
, mPosition(position)
, mNormal(normal)
, mStep(step)
, mCreatedParticles(0)
, mCollisionActive(false) {
  CHECK_SIZEOF(CTrailObject, 0x324)
}

void CIceAttackProjectile::CTrailObject::StartExplosion(CElementGen* gen, CStateManager& mgr) {
  mExplosion = gen;
  mExplosion->SetOrientation(mTrail->GetOrientation());
  mExplosion->SetTranslation(mTrail->GetTranslation() + CVector3f(0.f, 0.f, 0.5f));
  mExplosion->SetParticleEmission(true);
  DeleteCollisionObject(mgr);
}

void CIceAttackProjectile::CTrailObject::DeleteCollisionObject(CStateManager& mgr) {
  if (mCollisionObj != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mCollisionObj);
    mCollisionObj = kInvalidUniqueId;
  }
}

void CIceAttackProjectile::CTrailObject::Update(float dt, CStateManager& mgr,
                                                const CPhysicsActor* owner) {
  if (!mCollisionActive) {
    if (CPhysicsActor* collision =
            static_cast< CPhysicsActor* >(mgr.ObjectById(mCollisionObj))) {
      if (!CCollisionPrimitive::CollideBoolean(
              CInternalCollisionStructure::CPrimDesc(*collision->GetCollisionPrimitive(),
                                                     CMaterialFilter::GetPassEverything(),
                                                     collision->GetPrimitiveTransform()),
              CInternalCollisionStructure::CPrimDesc(*owner->GetCollisionPrimitive(),
                                                     CMaterialFilter::GetPassEverything(),
                                                     owner->GetPrimitiveTransform()))) {
        collision->SetActive(true);
        mCollisionActive = true;
      }
    }
  }
  if (!mTrail.null() && mCreatedParticles < 3) {
    CVector3f normal = mNormal;
    normal[kDX] += mgr.Random()->Range(-0.4f, 0.4f);
    normal[kDY] += mgr.Random()->Range(-0.4f, 0.4f);
    normal[kDZ] += mgr.Random()->Range(-0.4f, 0.4f);
    const CVector3f up =
        CTransform4f::RotateZ(CRelAngle::FromDegrees(mgr.Random()->Range(0.f, 360.f))) *
        CVector3f(0.f, 1.f, 0.f);
    const CTransform4f orientation =
        CTransform4f::LookAt(CVector3f::Zero(), normal.AsNormalized(), up);
    const float radius = mStep.Magnitude();
    mTrail->SetTranslation(mPosition + CVector3f(mgr.Random()->Range(-radius, radius),
                                                       mgr.Random()->Range(-radius, radius), -1.f));
    mTrail->SetOrientation(orientation);
    mTrail->ForceParticleCreation(1);
    mTrail->SetOrientation(CTransform4f::Identity());
    ++mCreatedParticles;
    mPosition += mStep;
  }
  if (!mTrail.null() && (mElapsed < 0.3f || !mExplosion.null())) {
    mTrail->Update(dt);
  }
  mElapsed += dt;
  if (!mExplosion.null()) {
    mExplosion->Update(dt);
    if (!mTrail.null() && mTrail->IsSystemDeletable()) {
      delete mTrail.release();
      mTrail = rstl::auto_ptr< CElementGen >();
    }
  }
}

void CIceAttackProjectile::CTrailObject::AddToRenderer(const CFrustumPlanes& planes,
                                                       const CStateManager& mgr) const {
  if (!mTrail.null()) {
    mActorLights.ActivateLights();
    mTrail->Render();
  }
  if (!mExplosion.null()) {
    mExplosion->Render();
  }
}

bool CIceAttackProjectile::CTrailObject::ExplosionStarted() const { return !mExplosion.null(); }

bool CIceAttackProjectile::CTrailObject::ExplosionFinished() const {
  if (!mExplosion.null() && mExplosion->IsSystemDeletable()) {
    return true;
  }
  return false;
}
