#include "MetroidPrime/Weapons/CNewFlameThrower.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CInternalCollisionStructure.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPuddleToadGamma.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CSnakeWeedSwarm.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "rstl/algorithm.hpp"

static const CMaterialFilter skExcludeProjectilePassthrough =
    CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough));

CNewFlameThrower::CNewFlameThrower(const TToken< CWeaponDescription >& desc,
                                   const rstl::string& name, EWeaponType wType,
                                   const CWeaponAssetInfo& resInfo, const CTransform4f& xf,
                                   EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid,
                                   TAreaId aid, TUniqueId owner, uint attribs)
: CGameProjectile(false, desc, name, wType, xf, matType, dInfo, uid, aid, owner, kInvalidUniqueId,
                  attribs, false, CVector3f(1.f, 1.f, 1.f), rstl::optional_object_null(),
                  CSfxManager::kInternalInvalidSfxId, false)
, mRand(99)
, mParticlesDoneTimer(0.f)
, mFlamesDoneTimer(0.f)
, mLastParticleCollisionLoc()
, mMainFire(gpSimplePool->GetObj(SObjectTag('PART', resInfo.data[0])))
, mMainSmoke(gpSimplePool->GetObj(SObjectTag('PART', resInfo.data[1])))
, mSecondarySmoke(gpSimplePool->GetObj(SObjectTag('PART', resInfo.data[4])))
, mSecondaryFire(gpSimplePool->GetObj(SObjectTag('PART', resInfo.data[5])))
, mSecondarySparks(gpSimplePool->GetObj(SObjectTag('PART', resInfo.data[6])))
, mSwooshCenter(gpSimplePool->GetObj(SObjectTag('SWHC', resInfo.data[2])))
, mSwooshFire(gpSimplePool->GetObj(SObjectTag('SWHC', resInfo.data[3])))
, mMainFireGen(nullptr)
, mMainSmokeGen(nullptr)
, mSecondarySmokeGen(nullptr)
, mSecondaryFireGen(nullptr)
, mSecondarySparksGen(nullptr)
, mSwooshCenterGen(nullptr)
, mSwooshFireGen(nullptr)
, mFlameState(kFS_Default)
, mCurrentLitArea(kInvalidAreaId)
, mRenderAuxEffects(false)
, mFiring(false)
, mRunningSlowish(false)
, mNewPointAdded(true)
, mActiveLighting(false)
, mFlameContactPoints(3, rstl::vector< SSortedListEntry >())
, mNumSmokeParticlesSpawned(0)
, mLightIds() {}

rstl::optional_object< CAABox > CNewFlameThrower::GetTouchBounds() const {
  return rstl::optional_object_null();
}

ENTITY_ACCEPT_IMPL(CNewFlameThrower)

void CNewFlameThrower::Touch(CActor& actor, CStateManager& mgr) {}

void CNewFlameThrower::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  const TAreaId area = mgr.GetWorld()->GetCurrentAreaId();
  mgr.SetActorAreaId(*this, area);
  for (int i = 0; i < mLightIds.size(); ++i) {
    CEntity* light = mgr.ObjectById(mLightIds[i]);
    if (light != nullptr) {
      mgr.SetActorAreaId(static_cast< CActor& >(*light), area);
    } else {
      mLightIds[i] = kInvalidUniqueId;
    }
  }
}

float CNewFlameThrower::UpdateFlameState(float dt, CStateManager& mgr) {
  bool activeLighting = false;
  switch (mFlameState) {
  case kFS_FireStart:
    mFlameState = kFS_FireActive;
    break;
  case kFS_FireActive:
    activeLighting = true;
    break;
  case kFS_FireStopTimer:
    activeLighting = true;
    mFlamesDoneTimer = 4.f * dt + mFlamesDoneTimer;
    if (mFlamesDoneTimer > 1.f) {
      mFlamesDoneTimer = 1.f;
      mFlameState = kFS_FireWaitForParticlesDone;
      mRenderAuxEffects = false;
    }
    break;
  case kFS_FireWaitForParticlesDone:
    mParticlesDoneTimer += dt;
    if (mParticlesDoneTimer > 0.1f && AreEffectsFinished()) {
      mFlameState = kFS_Default;
      Reset(mgr, true);
    }
    break;
  }
  if (activeLighting) {
    SetWorldDarkening(mgr, mgr.GetPlayer()->GetCurrentAreaId(), 4.f, 0.7f);
  } else {
    SetWorldDarkening(mgr, mgr.GetPlayer()->GetCurrentAreaId(), 1.f, 1.f);
  }
  return 0.f;
}

void CNewFlameThrower::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (GetActive()) {
    mRunningSlowish = gpMain->GetAverageDrawTime() + gpMain->GetAverageTickTime() > 0.65f;
    UpdateFlameState(dt, mgr);
    const CVector3f pos = xf.GetTranslation();
    const CTransform4f rotation = xf.GetRotation();
    mMainFireGen->SetTranslation(pos);
    mMainFireGen->SetOrientation(rotation);
    mSwooshCenterGen->SetTranslation(pos);
    mSwooshCenterGen->SetOrientation(rotation);
    mSwooshFireGen->SetTranslation(pos);
    mSwooshFireGen->SetOrientation(rotation);
    mMainFireGen->SetGeneratorRate(mRunningSlowish ? 0.5f : 1.f);
    mMainFireGen->Update(dt);
    mMainSmokeGen->Update(dt);
    mSecondarySmokeGen->Update(dt);
    mSecondaryFireGen->Update(dt);
    mSecondarySparksGen->Update(dt);
    mSwooshCenterGen->Update(dt);
    mSwooshFireGen->Update(dt);
    rstl::reserved_vector< CSphere, 32 > collisions;
    DoCollisionCheck(mgr, collisions, dt);
    if (collisions.size() > 0) {
      rstl::vector< CParticleSwoosh::SSwooshData >& swooshes = mSwooshCenterGen->Swooshes();
      for (int i = 0; i < swooshes.size(); ++i) {
        CParticleSwoosh::SSwooshData& swoosh = swooshes[i];
        if (swoosh.mActive) {
          for (int j = 0; j < collisions.size(); ++j) {
            const CSphere& sphere = collisions[j];
            const float radiusSquared = sphere.GetRadius() * sphere.GetRadius();
            const float distance = (sphere.GetCenter() - swoosh.mTranslation).MagSquared();
            if (distance < radiusSquared) {
              swoosh.mActive = false;
            }
          }
        }
      }
      for (int i = 0; i < mMainFireGen->GetParticleCount(); ++i) {
        const int index = i;
        CElementGen::CParticle& particle = mMainFireGen->Particles()[index];
        for (int j = 0; j < collisions.size(); ++j) {
          const CSphere& sphere = collisions[j];
          const float radiusSquared = sphere.GetRadius() * sphere.GetRadius();
          const float distance = (sphere.GetCenter() - particle.mPos).MagSquared();
          if (distance < radiusSquared) {
            particle.mEndFrame = -1;
          }
        }
      }
      for (int i = 0; i < mMainSmokeGen->GetParticleCount(); ++i) {
        const int index = i;
        CElementGen::CParticle& particle = mMainSmokeGen->Particles()[index];
        for (int j = 0; j < collisions.size(); ++j) {
          const CSphere& sphere = collisions[j];
          const float radiusSquared = sphere.GetRadius() * sphere.GetRadius();
          const float distance = (sphere.GetCenter() - particle.mPos).MagSquared();
          if (distance < radiusSquared) {
            particle.mEndFrame = -1;
          }
        }
      }
    }
    if (mFlameState == kFS_FireActive) {
      const rstl::vector< CParticleSwoosh::SSwooshData >& swooshes =
          mSwooshCenterGen->GetSwooshes();
      if (swooshes.capacity() - swooshes.size() < 4) {
        const int step = (swooshes.size() / 2 * 3) / 2;
        const int index = (step + mSwooshCenterGen->GetCurParticle()) % swooshes.size();
        const CParticleSwoosh::SSwooshData& swoosh = swooshes[index];
        if (swoosh.mActive) {
          const CVector3f& position = swoosh.mTranslation;
          const CTransform4f& orientation = swoosh.mOrientation;
          CVector3f delta = position - swooshes[(index + 1) % swooshes.size()].mTranslation;
          delta -= CVector3f::Dot(delta, orientation.GetForward()) * orientation.GetForward();
          const float magnitude = delta.Magnitude();
          const float maxRate = mRunningSlowish ? 2.f : 4.f;
          const float rate = rstl::min_val(rstl::max_val(1.f, magnitude * 30.f), maxRate);
          const int particles = int(rate);
          mNumSmokeParticlesSpawned =
              rstl::max_val(mNumSmokeParticlesSpawned - 1, particles);
          mMainSmokeGen->SetTranslation(position);
          mMainSmokeGen->SetOrientation(orientation);
          mMainSmokeGen->ForceParticleCreation(mNumSmokeParticlesSpawned);
        }
      }
    }
    UpdateLights(mgr);
  }
}

void CNewFlameThrower::AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const {
  EnsureRendered(mgr, GetTranslation(), GetSortingBounds(mgr));
}

void CNewFlameThrower::Render(const CStateManager& mgr) const {
  if (GetActive()) {
    mSwooshCenterGen->Render();
    mSwooshFireGen->Render();
    CElementGen* gens[5];
    gens[0] = mMainFireGen.get();
    gens[1] = mMainSmokeGen.get();
    gens[2] = mSecondarySmokeGen.get();
    gens[3] = mSecondaryFireGen.get();
    gens[4] = mSecondarySparksGen.get();
    CElementGen::RenderParticlesFlameThrower(gens, 5);
  }
}

void CNewFlameThrower::Fire(const CTransform4f& xf, CStateManager& mgr, bool) {
  SetActive(true);
  mFiring = true;
  mRenderAuxEffects = true;
  mFlameState = kFS_FireStart;
  CreateFlameParticles(mgr);
}

void CNewFlameThrower::Reset(CStateManager& mgr, bool deactivate) {
  if (deactivate) {
    SetLightsActive(mgr, false);
    SetActive(false);
    mFlameState = kFS_Default;
    mParticlesDoneTimer = 0.f;
    mFlamesDoneTimer = 0.f;
  } else {
    mFlameState = kFS_FireStopTimer;
  }
  mFiring = false;
  mMainFireGen->SetParticleEmission(false);
  mMainSmokeGen->SetParticleEmission(false);
  mSwooshCenterGen->SetParticleEmission(false);
  mSwooshFireGen->SetParticleEmission(false);
}

void CNewFlameThrower::CreateFlameParticles(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  mMainFireGen = rs_new CElementGen(mMainFire);
  mMainSmokeGen = rs_new CElementGen(mMainSmoke);
  mSecondarySmokeGen = rs_new CElementGen(mSecondarySmoke);
  mSecondaryFireGen = rs_new CElementGen(mSecondaryFire);
  mSecondarySparksGen = rs_new CElementGen(mSecondarySparks);
  mSwooshCenterGen = rs_new CParticleSwoosh(mSwooshCenter, 0);
  mSwooshCenterGen->SetRenderGaps(true);
  mSwooshFireGen = rs_new CParticleSwoosh(mSwooshFire, 0);
  mSwooshFireGen->SetRenderGaps(true);
  if (!mMainFireGen.null() && mMainFireGen->SystemHasLight() && mLightIds.empty()) {
    CreateLights(mgr);
  }
}

void CNewFlameThrower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                       CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    SetThermalFlags(kTF_Hot);
    Think(1.f / 60.f, mgr);
    mgr.AddWeaponId(GetOwnerId(), GetType());
    break;
  case kSM_Deleted:
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    DestroyLights(mgr);
    SetWorldDarkening(mgr, mgr.GetPlayer()->GetCurrentAreaId(), 4.f, 1.f);
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, uid, mgr);
}

bool CNewFlameThrower::DoCollisionCheck(CStateManager& mgr,
                                        rstl::reserved_vector< CSphere, 32 >& collisions,
                                        float dt) {
  bool collided = false;
  mLastParticleCollisionLoc = rstl::optional_object_null();
  rstl::vector< CParticleSwoosh::SSwooshData >& swooshes = mSwooshFireGen->Swooshes();
  TEntityList nearList;
  const int batchSize = rstl::max_val(swooshes.size() / 4, 6);
  const int count = mSwooshFireGen->GetSwooshCount();
  int current = count;
  current += mSwooshFireGen->GetCurParticle();
  --current;
  const int previous = current % count;
  for (int start = 0; start < swooshes.size(); start += batchSize) {
    rstl::optional_object< CAABox > bounds;
    const int end = rstl::min_val(start + batchSize, swooshes.size());
    float radius = 0.f;
    for (int i = start; i < end; ++i) {
      const CParticleSwoosh::SSwooshData& swoosh = swooshes[i];
      if (swoosh.mActive) {
        if (!bounds) {
          bounds = CAABox::MakeMaxInvertedBox();
        }
        bounds->AccumulateBounds(swoosh.mTranslation);
        radius = rstl::max_val(radius, swoosh.mVelocity.MagSquared());
      }
    }
    const float expandedRadius = CMath::SqrtF(radius) + 0.1f;
    radius = expandedRadius;
    if (bounds) {
      bounds->AccumulateBounds(bounds->GetMinPoint() -
                               CVector3f(expandedRadius, expandedRadius, expandedRadius));
      bounds->AccumulateBounds(bounds->GetMaxPoint() +
                               CVector3f(expandedRadius, expandedRadius, expandedRadius));
      nearList.clear();
      mgr.BuildNearList(nearList, *bounds,
                        CMaterialFilter::MakeIncludeExclude(
                            CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough)),
                        mgr.GetPlayer());
      CAreaCollisionCache cache(*bounds);
      CGameCollision::BuildAreaCollisionCache(mgr, cache);
      for (int i = start; i < end; ++i) {
        CParticleSwoosh::SSwooshData& swoosh = swooshes[i];
        if (previous != i && swoosh.mActive) {
          const float sphereRadius = radius;
          const CSphere sphere(swoosh.mTranslation, sphereRadius);
          CCollidableSphere primitive(sphere, CMaterialList(kMT_Solid));
          TUniqueId firstCollision = kInvalidUniqueId;
          CCollisionInfoList contacts;
          bool hit = CGameCollision::DetectStaticCollision_Cached(
              mgr, cache, primitive, CTransform4f::Identity(), skExcludeProjectilePassthrough,
              contacts);
          hit |= DynamicObjectCollision(mgr, nearList, primitive, firstCollision, contacts);
          if (hit && contacts.GetCount() != 0) {
            swoosh.mActive = false;
            collided = true;
            CCollisionInfoList filteredContacts;
            CollisionUtil::AccumulateCollisionInfo(contacts, filteredContacts);
            CVector3f lastPoint = CVector3f::Zero();
            for (int j = 0; j < filteredContacts.GetCount(); ++j) {
              if (j > 3) {
                break;
              }
              const CCollisionInfo& info = filteredContacts[j];
              const CSphere contactSphere(info.GetPoint(), mRunningSlowish ? 1.f : 0.75f);
              const int nearbyPoints = GetApproxNumSortedListPointsInSphere(contactSphere);
              if (nearbyPoints < (mRunningSlowish ? 2 : 3)) {
                AddToSortedLists(info, 10);
                const CTransform4f rotation =
                    CTransform4f::LookAt(CVector3f::Zero(), info.GetNormalLeft(), CVector3f::Up());
                mSecondarySmokeGen->SetOrientation(rotation);
                mSecondaryFireGen->SetOrientation(rotation);
                mSecondarySparksGen->SetOrientation(rotation);
                mSecondarySmokeGen->SetTranslation(info.GetPoint());
                mSecondaryFireGen->SetTranslation(info.GetPoint());
                mSecondarySparksGen->SetTranslation(info.GetPoint());
                mSecondarySmokeGen->ForceParticleCreation(1);
                mSecondaryFireGen->ForceParticleCreation(mRunningSlowish ? 2 : 3);
                mSecondarySparksGen->ForceParticleCreation(mRunningSlowish ? 3 : 5);
                if (mRunningSlowish) {
                  break;
                }
                lastPoint = info.GetPoint();
              }
            }
            if (!mRunningSlowish && mLastParticleCollisionLoc) {
              if (filteredContacts.GetCount() < 3 ||
                  (*mLastParticleCollisionLoc - lastPoint).MagSquared() > 3.f) {
                const CVector3f midpoint =
                    CVector3f::Lerp(lastPoint, *mLastParticleCollisionLoc, 0.5f);
                mSecondaryFireGen->SetTranslation(midpoint);
                mSecondaryFireGen->ForceParticleCreation(2);
              }
            }
            mLastParticleCollisionLoc = lastPoint;
            if (firstCollision != kInvalidUniqueId) {
              if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(firstCollision))) {
                if (CanDamage(*actor, mgr)) {
                  mgr.ApplyDamage(GetUniqueId(), actor->GetUniqueId(), GetOwnerId(),
                                  GetCurrentDamageInfo().MakeScaledForTime(dt), GetFilter(),
                                  swoosh.mVelocity.AsNormalized());
                }
              }
            }
            mgr.ApplyDamageToWorld(GetOwnerId(), *this, swoosh.mTranslation,
                                   GetCurrentDamageInfo().MakeScaledForTime(dt), GetFilter());
            collisions.push_back(sphere);
            if (collisions.size() == 32) {
              return true;
            }
          }
        }
      }
      nearList.clear();
      mgr.BuildNearList(nearList, *bounds,
                        CMaterialFilter::MakeInclude(CMaterialList(kMT_NonSolidDamageable)),
                        mgr.GetPlayer());
      for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
        if (CSnakeWeedSwarm* swarm = TCastToPtr< CSnakeWeedSwarm >(mgr.ObjectById(*it))) {
          for (int i = start; i < end; ++i) {
            const CParticleSwoosh::SSwooshData& swoosh = swooshes[i];
            if (previous != i && swoosh.mActive) {
              const float sphereRadius = radius;
              const CSphere sphere(swoosh.mTranslation, sphereRadius);
              swarm->ScareSnakeWeeds(mgr, sphere.GetCenter(),
                                     rstl::max_val(sphere.GetRadius(), 1.f) *
                                         swarm->GetWeaponDamageRadius());
            }
          }
        }
      }
      for (int i = start; i < end; ++i) {
        const CParticleSwoosh::SSwooshData& swoosh = swooshes[i];
        if (previous != i && swoosh.mActive) {
          TUniqueId firstCollision = kInvalidUniqueId;
          CCollisionInfoList contacts;
          const float sphereRadius = radius;
          const CSphere sphere(swoosh.mTranslation, sphereRadius);
          CCollidableSphere primitive(sphere, CMaterialList(kMT_Solid));
          DynamicObjectCollision(mgr, nearList, primitive, firstCollision, contacts);
          if (firstCollision != kInvalidUniqueId) {
            if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(firstCollision))) {
              if (CanDamage(*actor, mgr)) {
                mgr.ApplyDamage(GetUniqueId(), actor->GetUniqueId(), GetOwnerId(),
                                GetCurrentDamageInfo().MakeScaledForTime(dt), GetFilter(),
                                swoosh.mVelocity.AsNormalized());
              }
            }
          }
        }
      }
    }
  }
  RemoveDeadEntriesFromSortedLists();
  return collided;
}

bool CNewFlameThrower::DynamicObjectCollision(CStateManager& mgr, const TEntityList& nearList,
                                              const CCollisionPrimitive& primitive,
                                              TUniqueId& firstCollision,
                                              CCollisionInfoList& collisions) {
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(*it))) {
      if (CPhysicsActor* physicsActor = TCastToPtr< CPhysicsActor >(actor)) {
        if (CCollisionPrimitive::Collide(
                CInternalCollisionStructure::CPrimDesc(primitive, skExcludeProjectilePassthrough,
                                                       CTransform4f::Identity()),
                CInternalCollisionStructure::CPrimDesc(*physicsActor->GetCollisionPrimitive(),
                                                       physicsActor->GetMaterialFilter(),
                                                       physicsActor->GetPrimitiveTransform()),
                collisions)) {
          firstCollision = *it;
          return true;
        }
      } else {
        const rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
        if (bounds) {
          if (CCollisionPrimitive::Collide(
                  CInternalCollisionStructure::CPrimDesc(primitive, skExcludeProjectilePassthrough,
                                                         CTransform4f::Identity()),
                  CInternalCollisionStructure::CPrimDesc(
                      CCollidableAABox(*bounds, CMaterialList(kMT_Solid)),
                      CMaterialFilter::skPassEverything, CTransform4f::Identity()),
                  collisions)) {
            firstCollision = *it;
            return true;
          }
        }
      }
    }
  }
  return false;
}

const bool CNewFlameThrower::AreEffectsFinished() const {
  bool finished = false;
  if (!mMainFireGen.null()) {
    finished = mMainFireGen->GetParticleCount() == 0;
  }
  if (finished && !mMainSmokeGen.null()) {
    finished = mMainSmokeGen->GetParticleCount() == 0;
  }
  if (finished && !mSecondarySmokeGen.null()) {
    finished = mSecondarySmokeGen->GetParticleCount() == 0;
  }
  if (finished && !mSecondaryFireGen.null()) {
    finished = mSecondaryFireGen->GetParticleCount() == 0;
  }
  if (finished && !mSecondarySparksGen.null()) {
    finished = mSecondarySparksGen->GetParticleCount() == 0;
  }
  return finished;
}

void CNewFlameThrower::CreateLights(CStateManager& mgr) {
  DestroyLights(mgr);
  for (int i = 0; i < 4; ++i) {
    const TUniqueId uid = mgr.AllocateUniqueId();
    if (uid != kInvalidUniqueId) {
      const CAssetId lightId = static_cast< uint >(reinterpret_cast< uintptr_t >(this)) + (i & 1);
      CEntity* light = rs_new CGameLight(
          uid, GetAreaId(), false, rstl::string_l("FlamethrowerLight"), CTransform4f::Identity(),
          GetUniqueId(), mMainFireGen->GetLight(), lightId, 0, 0.f);
      mgr.AddObject(light);
      mLightIds.push_back(uid);
    }
  }
}

void CNewFlameThrower::UpdateLights(CStateManager& mgr) {
  CGlobalRandom globalRandom(mRand);
  CParticleSwoosh* const swooshGen = mSwooshFireGen.get();
  const int count = swooshGen->GetSwooshCount();
  int current = count;
  current += swooshGen->GetCurParticle();
  --current;
  const int previous = current % count;
  const int stride = rstl::max_val(2, count / 4);
  for (int i = 0, offset = 0; i < mLightIds.size(); ++i, offset += stride) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightIds[i]))) {
      bool active = true;
      if (offset >= swooshGen->GetSwooshCount()) {
        active = false;
      }
      const CParticleSwoosh::SSwooshData& swoosh =
          swooshGen->GetSwooshes()[(offset + previous) % swooshGen->GetSwooshCount()];
      if (!swoosh.mActive) {
        active = false;
      }
      light->SetActive(static_cast< bool >(active));
      if (active) {
        CLight lightData = mMainFireGen->GetLight();
        if (CColorElement* color = mMainFire->mLCLR) {
          int time = mRand.Range(0, 16);
          CParticleGlobals::SetEmitterTime(time);
          CColor value = CColor::Yellow();
          color->GetValue(time, value);
          lightData.SetColor(value);
        }
        if (CRealElement* intensity = mMainFire->mLINT) {
          int time = mRand.Range(0, 16);
          CParticleGlobals::SetEmitterTime(time);
          float value = 1.f;
          intensity->GetValue(time, value);
          lightData.SetAngleAttenuation(value, 0.f, 0.f);
        }
        light->SetLight(lightData);
        light->SetTranslation(swoosh.mTranslation);
      }
    }
  }
}

void CNewFlameThrower::DestroyLights(CStateManager& mgr) {
  for (int i = 0; i < mLightIds.size(); ++i) {
    mgr.DeleteObjectRequest(mLightIds[i]);
  }
  mLightIds.clear();
}

void CNewFlameThrower::SetLightsActive(CStateManager& mgr, bool active) {
  for (int i = 0; i < mLightIds.size(); ++i) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightIds[i]))) {
      light->SetActive(active);
    }
  }
}

void CNewFlameThrower::AddToSortedLists(const CCollisionInfo& info, int time) {
  for (int i = 0; i < 3; ++i) {
    rstl::vector< SSortedListEntry >& entries = mFlameContactPoints[i];
    if (entries.size() == entries.capacity()) {
      entries.reserve(entries.capacity() > 0 ? entries.capacity() * 2 : 10);
    }
    entries.push_back(SSortedListEntry(info.GetPoint()[i], time));
  }
  mNewPointAdded = true;
}

int CNewFlameThrower::GetApproxNumSortedListPointsInSphere(const CSphere& sphere) {
  if (mNewPointAdded) {
    for (int i = 0; i < 3; ++i) {
      rstl::vector< SSortedListEntry >& entries = mFlameContactPoints[i];
      rstl::sort(entries.begin(), entries.end());
    }
  }
  int count = 0x7fffffff;
  for (int i = 0; i < 3; ++i) {
    const rstl::vector< SSortedListEntry >& entries = mFlameContactPoints[i];
    count = rstl::min_val(
        count,
        int(rstl::upper_bound(entries.begin(), entries.end(),
                              SSortedListEntry(sphere.GetCenter()[i] + sphere.GetRadius(), 0)) -
            rstl::lower_bound(entries.begin(), entries.end(),
                              SSortedListEntry(sphere.GetCenter()[i] - sphere.GetRadius(), 0))));
    if (count == 0) {
      return count;
    }
  }
  return count;
}

void CNewFlameThrower::RemoveDeadEntriesFromSortedLists() {
  for (int i = 0; i < 3; ++i) {
    rstl::vector< SSortedListEntry >& entries = mFlameContactPoints[i];
    int last = entries.size() - 1;
    for (int j = 0; j < entries.size(); ++j) {
      --entries[j].mRemainingTime;
      if (entries[j].mRemainingTime == 0) {
        entries[j] = entries[last];
        entries.pop_back();
        --last;
      }
    }
  }
}

void CNewFlameThrower::SetWorldDarkening(CStateManager& mgr, TAreaId area, float speed,
                                         float target) {
  if (mActiveLighting && mCurrentLitArea != area &&
      mCurrentLitArea != kInvalidAreaId) {
    CGameArea* litArea = mgr.World()->Area(mCurrentLitArea);
    if (litArea->IsPostConstructed()) {
      litArea->SetWeaponWorldLighting(1.f, 1.f);
    }
  }
  mCurrentLitArea = area;
  mActiveLighting = target != 1.f;
  if (mCurrentLitArea != kInvalidAreaId) {
    CGameArea* litArea = mgr.World()->Area(mCurrentLitArea);
    if (litArea->IsPostConstructed()) {
      litArea->SetWeaponWorldLighting(speed, target);
    }
  }
}

bool CNewFlameThrower::CanDamage(CActor& actor, CStateManager& mgr) {
  if (actor.GetDamageVulnerability()->GetVulnerability(
          mCurDamageInfo.GetWeaponMode(), CDamageVulnerability::kRD_No) == kVN_PassThrough) {
    return false;
  }
  if (TCastToPtr< CScriptTrigger >(actor)) {
    return CanCollideWithTrigger(actor, mgr).GetActorId() != kInvalidUniqueId;
  }
  if (TCastToPtr< CScriptPlatform >(actor) || TCastToPtr< CCollisionActor >(actor) ||
      PATTERNED_CAST_TO(CPuddleToadGamma, &actor)) {
    return true;
  }
  return CanCollideWithGameObject(actor, mgr).GetActorId() != kInvalidUniqueId;
}

CNewFlameThrower::~CNewFlameThrower() {}
