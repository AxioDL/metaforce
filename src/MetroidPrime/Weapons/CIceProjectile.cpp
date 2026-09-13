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
, xe8_trailDesc(trail)
, xf0_explosionDesc(explosion)
, xf8_movingDesc(moving)
, x118_owner(owner)
, x11c_damage(damage)
, x138_currentDamage(damage)
, x154_bounds(bounds)
, x170_speed(speed)
, x174_turnSpeed(turnSpeed)
, x178_moveTime(0.f)
, x17c_explosionTimer(0.f)
, x180_frameCount(0)
, x184_steamTexture(steamTexture)
, x188_freezeSfx(freezeSfx)
, x18a_explosionSfx(explosionSfx)
, x18c_iceTexture(iceTexture)
, x190_finishedMoving(false)
, x191_explosionSoundStarted(false)
, x192_useWorldRay(false) {
  const CVector3f up(0.f, 0.f, 1.f);
  const CVector3f right = CVector3f::Cross(up, xf.GetForward()).AsNormalized();
  const CVector3f forward = CVector3f::Cross(right, up).AsNormalized();
  SetTransform(CTransform4f::FromColumns(right, forward, up, GetTranslation()));
  x100_movingGen = rs_new CElementGen(xf8_movingDesc);
}

CIceAttackProjectile::~CIceAttackProjectile() {}

void CIceAttackProjectile::Touch(CActor& actor, CStateManager& mgr) {
  if (actor.GetUniqueId() == x118_owner && !mgr.GetPlayer()->GetFrozenState()) {
    const ushort sfx = x188_freezeSfx;
    mgr.Player()->SetFrozenState(mgr, x184_steamTexture, sfx, x18c_iceTexture);
    mgr.ApplyDamage(GetUniqueId(), actor.GetUniqueId(), GetUniqueId(), x11c_damage,
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                    CVector3f::Zero());
  }
}

rstl::optional_object< CAABox > CIceAttackProjectile::GetTouchBounds() const {
  if (GetActive()) {
    return x154_bounds->GetTransformedAABox(GetTransform());
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
  for (int i = 0; i < x108_trailObjects.size(); ++i) {
    x108_trailObjects[i].AddToRenderer(planes, mgr);
  }
  x100_movingGen->Render();
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

  CElementGen* const gen = rs_new CElementGen(xe8_trailDesc, CElementGen::kMOT_One);
  gen->SetLeaveLightsEnabledForModelRender(true);
  CTrailObject trail(gen, collisionId, GetTranslation(), normal,
                     GetTransform().Rotate(CVector3f(0.f, x170_speed * dt, 0.f)));
  trail.ActorLights().SetMaxAreaLights(2);
  trail.ActorLights().BuildAreaLightList(mgr, *mgr.GetWorld()->GetArea(GetCurrentAreaId()),
                                         CAABox(GetTranslation(), GetTranslation()));
  if (x108_trailObjects.capacity() < x108_trailObjects.size() + 1) {
    x108_trailObjects.reserve(x108_trailObjects.size() + 1);
  }
  x108_trailObjects.push_back(trail);
}

void CIceAttackProjectile::UpdateTrailObjects(float dt, CStateManager& mgr) {
  if (x190_finishedMoving) {
    x17c_explosionTimer += dt;
    if (x17c_explosionTimer > 0.05f) {
      if (!x191_explosionSoundStarted) {
        x194_explosionSfxHandle =
            CSfxManager::SfxStart(x18a_explosionSfx, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                                  true, CSfxManager::kAllAreas);
        x191_explosionSoundStarted = true;
      }
      x17c_explosionTimer -= 0.05f;
      for (int i = 0; i < x108_trailObjects.size(); ++i) {
        CTrailObject& trail = x108_trailObjects[i];
        if (!trail.ExplosionStarted()) {
          trail.StartExplosion(rs_new CElementGen(xf0_explosionDesc), mgr);
          break;
        }
      }
    }
  }
  const CPhysicsActor* owner = static_cast< const CPhysicsActor* >(mgr.GetObjectById(x118_owner));
  for (int i = 0; i < x108_trailObjects.size(); ++i) {
    x108_trailObjects[i].Update(dt, mgr, owner);
  }
}

void CIceAttackProjectile::Think(float dt, CStateManager& mgr) {
  bool hasFloor = false;
  bool hasWall = false;
  if (!GetActive()) {
    return;
  }
  if (!x190_finishedMoving) {
    if (const CActor* owner = TCastToConstPtr< CActor >(mgr.GetObjectById(x118_owner))) {
      CVector3f delta = owner->GetTranslation() - GetTranslation();
      const CVector3f flatDelta(delta.GetX(), delta.GetY(), 0.f);
      if (flatDelta.MagSquared() < 25.f) {
        x174_turnSpeed = 0.f;
      }
      if (delta.CanBeNormalized()) {
        delta.Normalize();
        delta = GetTransform().TransposeRotate(delta);
        const float angle =
            CVector3f::GetAngleDiff(CVector3f(0.f, 1.f, 0.f), delta) * CMath::Sign(delta.GetX());
        const float turn = -CMath::Clamp(-x174_turnSpeed * dt, angle, x174_turnSpeed * dt);
        CTransform4f xf = GetTransform();
        xf.RotateLocalZ(CRelAngle::FromRadians(turn));
        SetTransform(xf);
      }
    }
    x178_moveTime += dt;
    CVector3f position = GetTransform() * CVector3f(0.f, x170_speed * dt, 0.f);
    CRayCastResult floorResult;
    if (x192_useWorldRay) {
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
    if (x178_moveTime > 3.f || hasWall || !hasFloor) {
      x190_finishedMoving = true;
    }
    ++x180_frameCount;
    if (x180_frameCount % 4 == 0 && hasFloor) {
      CreateTrailObject(mgr, floorResult.GetPlane().GetNormal(), dt);
    }
  }
  UpdateTrailObjects(dt, mgr);
  x100_movingGen->SetOrientation(GetTransform().GetRotation());
  x100_movingGen->SetGlobalTranslation(GetTranslation() + 1.f * GetTransform().GetForward() -
                                       1.f * GetTransform().GetUp());
  x100_movingGen->Update(dt);
  if (x190_finishedMoving) {
    x100_movingGen->SetParticleEmission(false);
    bool finished = x100_movingGen->GetParticleCount() == 0;
    for (int i = 0; i < x108_trailObjects.size(); ++i) {
      const CTrailObject& trail = x108_trailObjects[i];
      if (!trail.ExplosionFinished() || !trail.ExplosionStarted()) {
        finished = false;
        break;
      }
    }
    if (finished) {
      mgr.DeleteObjectRequest(GetUniqueId());
      CSfxManager::SfxStop(x194_explosionSfxHandle);
    }
  }
  x138_currentDamage = x11c_damage.MakeScaledForTime(dt);
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
      for (int i = 0; i < x108_trailObjects.size(); ++i) {
        x108_trailObjects[i].DeleteCollisionObject(mgr);
      }
    }
    break;
  }
}

CIceAttackProjectile::CTrailObject::CTrailObject(CElementGen* gen, TUniqueId collisionId,
                                                 const CVector3f& position, const CVector3f& normal,
                                                 const CVector3f& step)
: x0_trail(gen)
, x10_collisionObj(collisionId)
, x14_elapsed(0.f)
, x18_actorLights(1, CVector3f(0.f, 0.f, 1.f), 4, 4)
, x2f8_position(position)
, x304_normal(normal)
, x310_step(step)
, x31c_createdParticles(0)
, x320_collisionActive(false) {
  CHECK_SIZEOF(CTrailObject, 0x324)
}

void CIceAttackProjectile::CTrailObject::StartExplosion(CElementGen* gen, CStateManager& mgr) {
  x8_explosion = gen;
  x8_explosion->SetOrientation(x0_trail->GetOrientation());
  x8_explosion->SetTranslation(x0_trail->GetTranslation() + CVector3f(0.f, 0.f, 0.5f));
  x8_explosion->SetParticleEmission(true);
  DeleteCollisionObject(mgr);
}

void CIceAttackProjectile::CTrailObject::DeleteCollisionObject(CStateManager& mgr) {
  if (x10_collisionObj != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(x10_collisionObj);
    x10_collisionObj = kInvalidUniqueId;
  }
}

void CIceAttackProjectile::CTrailObject::Update(float dt, CStateManager& mgr,
                                                const CPhysicsActor* owner) {
  if (!x320_collisionActive) {
    if (CPhysicsActor* collision =
            static_cast< CPhysicsActor* >(mgr.ObjectById(x10_collisionObj))) {
      if (!CCollisionPrimitive::CollideBoolean(
              CInternalCollisionStructure::CPrimDesc(*collision->GetCollisionPrimitive(),
                                                     CMaterialFilter::GetPassEverything(),
                                                     collision->GetPrimitiveTransform()),
              CInternalCollisionStructure::CPrimDesc(*owner->GetCollisionPrimitive(),
                                                     CMaterialFilter::GetPassEverything(),
                                                     owner->GetPrimitiveTransform()))) {
        collision->SetActive(true);
        x320_collisionActive = true;
      }
    }
  }
  if (!x0_trail.null() && x31c_createdParticles < 3) {
    CVector3f normal = x304_normal;
    normal[kDX] += mgr.Random()->Range(-0.4f, 0.4f);
    normal[kDY] += mgr.Random()->Range(-0.4f, 0.4f);
    normal[kDZ] += mgr.Random()->Range(-0.4f, 0.4f);
    const CVector3f up =
        CTransform4f::RotateZ(CRelAngle::FromDegrees(mgr.Random()->Range(0.f, 360.f))) *
        CVector3f(0.f, 1.f, 0.f);
    const CTransform4f orientation =
        CTransform4f::LookAt(CVector3f::Zero(), normal.AsNormalized(), up);
    const float radius = x310_step.Magnitude();
    x0_trail->SetTranslation(x2f8_position + CVector3f(mgr.Random()->Range(-radius, radius),
                                                       mgr.Random()->Range(-radius, radius), -1.f));
    x0_trail->SetOrientation(orientation);
    x0_trail->ForceParticleCreation(1);
    x0_trail->SetOrientation(CTransform4f::Identity());
    ++x31c_createdParticles;
    x2f8_position += x310_step;
  }
  if (!x0_trail.null() && (x14_elapsed < 0.3f || !x8_explosion.null())) {
    x0_trail->Update(dt);
  }
  x14_elapsed += dt;
  if (!x8_explosion.null()) {
    x8_explosion->Update(dt);
    if (!x0_trail.null() && x0_trail->IsSystemDeletable()) {
      delete x0_trail.release();
      x0_trail = rstl::auto_ptr< CElementGen >();
    }
  }
}

void CIceAttackProjectile::CTrailObject::AddToRenderer(const CFrustumPlanes& planes,
                                                       const CStateManager& mgr) const {
  if (!x0_trail.null()) {
    x18_actorLights.ActivateLights();
    x0_trail->Render();
  }
  if (!x8_explosion.null()) {
    x8_explosion->Render();
  }
}

bool CIceAttackProjectile::CTrailObject::ExplosionStarted() const { return !x8_explosion.null(); }

bool CIceAttackProjectile::CTrailObject::ExplosionFinished() const {
  if (!x8_explosion.null() && x8_explosion->IsSystemDeletable()) {
    return true;
  }
  return false;
}
