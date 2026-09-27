#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CIceImpact.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include <float.h>

static CTransform4f LookAt(const CVector3f& a, const CVector3f& b, const CRelAngle& angle) {
  const float dot = CVector3f::Dot(a, b);
  if (close_enough(dot, 1.f)) {
    return CTransform4f::Identity();
  }
  if (dot > -0.99981f) {
    return CQuaternion::ClampedRotateTo(a, b, angle).BuildTransform4f();
  }
  if (!(a == CVector3f::Right()) && !(b == CVector3f::Right())) {
    return CQuaternion::AxisAngle(CUnitVector3f(CVector3f::Cross(a, CVector3f::Right())), angle)
        .BuildTransform4f();
  }
  return CQuaternion::AxisAngle(CUnitVector3f(CVector3f::Cross(a, CVector3f::Up())), angle)
      .BuildTransform4f();
}

CWallCrawlerSwarm::CBoid::CBoid(const CTransform4f& xf, uint index)
: mTransform(xf)
, mVelocity(0.f, 0.f, 0.f)
, mTargetWaypoint(kInvalidUniqueId)
, mAmbientLighting(0.3f, 0.3f, 0.3f, 1.f)
, mNext(nullptr)
, mTimeToDie(0.f)
, mTimeToExplode(0.f)
, mSurface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f), CVector3f(0.f, 0.f, 1.f), ~0)
, mFramesNotOnSurface(0)
, mIndex(index)
, mActive(false)
, mInFrustum(false)
, mLaunched(false)
, mScarabExplodeTimerEnabled(false)
, mNearPlayer(false) {}

CWallCrawlerSwarm::CWallCrawlerSwarm(
    TUniqueId uid, const bool active, const rstl::string& name, const CEntityInfo& info,
    const CVector3f& boundingBoxExtent, const CTransform4f& xf, uint flavor,
    const CAnimRes& animRes, uint launchAnim, uint attractAnim, uint particle1, uint particle2,
    uint particle3, uint particle4, const CDamageInfo& crabDamage,
    const CDamageInfo& scarabExplodeDamage, float crabDamageCooldown, float boidRadius,
    float touchRadius, float playerTouchRadius, int numBoids, int maxCreatedBoids,
    float animPlaybackSpeed, float separationRadius, float cohesionMagnitude, float alignmentWeight,
    float separationMagnitude, float moveToWaypointWeight, float attractionMagnitude,
    float attractionRadius, float boidGenRate, int maxLaunches, float scarabBoxMargin,
    float scarabScatterXYVelocity, float scarabTimeToExplode, const CHealthInfo& healthInfo,
    const CDamageVulnerability& damageVulnerability, int launchSfx, int scatterSfx,
    CActorParameters actParams)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(),
         CMaterialList(kMT_Scannable, kMT_Trigger, kMT_NonSolidDamageable, kMT_RadarObject),
         actParams, kInvalidUniqueId)
, mAabox(CVector3f(0.f, 0.f, 0.f), CVector3f(0.f, 0.f, 0.f))
, mOccludedTimer(5.f)
, mBoundingBoxExtent(boundingBoxExtent)
, mLastOrbitPosition(0.f, 0.f, 0.f)
, mLastKilledOffset(CVector3f::Zero())
, mSeparationRadius(separationRadius)
, mCohesionMagnitude(cohesionMagnitude)
, mAlignmentWeight(alignmentWeight)
, mSeparationMagnitude(separationMagnitude)
, mMoveToWaypointWeight(moveToWaypointWeight)
, mAttractionMagnitude(attractionMagnitude)
, mAttractionRadius(attractionRadius)
, mScarabScatterXYVelocity(scarabScatterXYVelocity)
, mScarabTimeToExplode(scarabTimeToExplode)
, mAnimPlaybackSpeed(animPlaybackSpeed)
, mWaypointGoalRadius(3.f)
, mPartitionedBoidLists(nullptr)
, mOutlierBoidList(nullptr)
, mBoidGenRate(boidGenRate)
, mBoidGenCooldownTimer(0.f)
, mCrabDamageCooldownTimer(0.f)
, mCrabDamageCooldown(crabDamageCooldown)
, mBoidRadius(boidRadius)
, mTouchRadius(touchRadius)
, mScarabBoxMargin(scarabBoxMargin)
, mPlayerTouchRadius(playerTouchRadius)
, mCrabDamage(crabDamage)
, mScarabExplodeDamage(scarabExplodeDamage)
, mHealthInfo(healthInfo)
, mDamageVulnerability(damageVulnerability)
, mLockOnIdx(-1)
, mWhichModel(CModelData::kWM_Normal)
, mNumBoids(numBoids)
, mMaxCreatedBoids(maxCreatedBoids)
, mCreatedBoids(0)
, mMaxLaunches(maxLaunches)
, mFlavor(static_cast< EFlavor >(flavor))
, mLaunchSfx(
      CSfxManager::TranslateSFXID(launchSfx == -1 ? CSfxManager::kInternalInvalidSfxId : launchSfx))
, mScatterSfx(CSfxManager::TranslateSFXID(scatterSfx == -1 ? CSfxManager::kInternalInvalidSfxId
                                                               : scatterSfx))
, mEnableLighting(true)
, mUseSoftwareLight(true)
, mModelAssetDirty(false) {
  const CAnimRes attractRes(animRes.GetId(), animRes.GetCharacterNodeId(), animRes.GetScale(),
                            attractAnim != -1 ? attractAnim : 0, true);
  const CAnimRes launchRes(animRes.GetId(), animRes.GetCharacterNodeId(), animRes.GetScale(),
                           launchAnim != -1 ? launchAnim : 0, true);
  mModelDatas.push_back(rs_new CModelData(animRes));
  mModelDatas.push_back(rs_new CModelData(animRes));
  mModelDatas.push_back(rs_new CModelData(animRes));
  mModelDatas.push_back(rs_new CModelData(animRes));
  mModelDatas.push_back(rs_new CModelData(attractRes));
  mModelDatas.push_back(rs_new CModelData(attractRes));
  mModelDatas.push_back(rs_new CModelData(attractRes));
  mModelDatas.push_back(rs_new CModelData(attractRes));
  mModelDatas.push_back(rs_new CModelData(launchRes));
  mModelDatas.push_back(rs_new CModelData(animRes));
  if (actParams.GetXRay().first != 0) {
    for (int i = 0; i < 9; ++i) {
      mModelDatas[i]->SetXRayModel(actParams.GetXRay());
    }
    mModelAssetDirty = true;
  }
  if (actParams.GetInfra().first != 0) {
    for (int i = 0; i < 9; ++i) {
      mModelDatas[i]->SetInfraModel(actParams.GetInfra());
    }
    mModelAssetDirty = true;
  }
  if (particle1 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle1)));
  }
  if (particle2 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle2)));
  }
  if (particle3 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle3)));
  }
  if (particle4 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle4)));
  }
  for (int i = 0; i < mParticleDescs.size(); ++i) {
    mParticleGens.push_back(rs_new CElementGen(mParticleDescs[i]));
    mParticleGens[i]->SetParticleEmission(false);
  }
}

CWallCrawlerSwarm::~CWallCrawlerSwarm() {}

ENTITY_ACCEPT_IMPL(CWallCrawlerSwarm)

void CWallCrawlerSwarm::AddDoorRepulsors(CStateManager& mgr) {
  CObjectList& objects = mgr.ObjectListById(kOL_PhysicsActor);
  int count = 0;
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    CEntity* entity = objects[i];
    if (CScriptDoor* door = TCastToPtr< CScriptDoor >(entity)) {
      if (door->GetCurrentAreaId() == GetCurrentAreaId()) {
        ++count;
      }
    }
  }
  mDoorRepulsors.reserve(count);
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    CEntity* entity = objects[i];
    if (CScriptDoor* door = TCastToPtr< CScriptDoor >(entity)) {
      if (door->GetCurrentAreaId() == GetCurrentAreaId()) {
        rstl::optional_object< CAABox > bounds = door->GetTouchBounds();
        if (bounds.valid()) {
          float diagonal = (bounds->GetMinPoint() - bounds->GetMaxPoint()).Magnitude();
          mDoorRepulsors.push_back(CRepulsor(bounds->GetCenterPoint(), 0.75f * diagonal));
        }
      }
    }
  }
}

void CWallCrawlerSwarm::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  mPosWorkspaces.clear();
  mNrmWorkspaces.clear();
  for (int i = 0; i < 9; ++i) {
    float* normals;
    mPosWorkspaces.push_back(
        mModelDatas[i]->PickAnimatedModel(which).AllocateNewWorkspace(&normals));
    mNrmWorkspaces.push_back(normals);
    mModelDatas[i]->EnableLooping(true);
    mModelDatas[i]->AdvanceAnimation(
        mModelDatas[i]->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")) *
            (float(i) / 16.f),
        mgr, GetCurrentAreaId(), true);
  }
  float* normals;
  mPosWorkspaces.push_back(
      mModelDatas[9]->PickAnimatedModel(which).AllocateNewWorkspace(&normals));
  mNrmWorkspaces.push_back(normals);
  mWhichModel = which;
}

void CWallCrawlerSwarm::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                        CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Activate:
  case kSM_Deactivate:
    break;
  case kSM_Registered:
    mBoids.reserve(mNumBoids);
    for (int i = 0; i < mBoids.capacity(); ++i) {
      mBoids.push_back(CBoid(CTransform4f::Identity(), i));
    }
    AllocateSkinnedModels(mgr, CModelData::kWM_Normal);
    AddDoorRepulsors(mgr);
    SetDrawShadow(false);
    break;
  default:
    break;
  }
}

bool CWallCrawlerSwarm::FindBestSurface(const CAreaCollisionCache& cache, CVector3f pos,
                                        float radius, CCollisionSurface& out) {
  bool found = false;
  float minDistance = radius * radius;
  CSphere sphere(pos, radius);
  for (int i = 0; i < int(cache.GetNumCaches()); ++i) {
    const CMetroidAreaCollider::COctreeLeafCache& leafCache = cache.GetOctreeLeafCache(i);
    for (int j = 0; j < leafCache.GetNumLeaves(); ++j) {
      const CAreaOctTree::Node& node = leafCache.GetLeaf(j);
      if (CCollidableSphere::Sphere_AABox_Bool(sphere, node.GetBoundingBox())) {
        const CAreaOctTree::TriListReference triangles = node.GetTriangleArray();
        const CAreaOctTree& tree = node.GetOwner();
        const int triangleCount = triangles.GetSize();
        for (int k = 0; k < triangleCount; ++k) {
          const CCollisionSurface& surface = tree.GetMasterListTriangle(triangles.GetAt(k));
          const CPlane plane = surface.GetPlane();
          const float distance = CMath::AbsF(plane.GetHeight(pos));
          if (distance < minDistance && PointOnSurface(surface, pos, plane)) {
            sphere = CSphere(pos, CMath::FastSqrtF(distance));
            out = surface;
            found = true;
            minDistance = distance;
          }
        }
      }
    }
  }
  return found;
}

CCollisionSurface CWallCrawlerSwarm::FindBestCollisionInBox(CStateManager& mgr,
                                                            const CVector3f& pos) {
  CCollisionSurface result(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                           CVector3f(0.f, 0.f, 1.f), ~0);
  const CAABox& bounds = GetBoundingBox();
  const CVector3f extent(0.5f * bounds.GetWidth(), 0.5f * bounds.GetHeight(),
                         0.5f * bounds.GetDepth());
  for (float scale = 0.1f; scale < 1.f; scale += 0.1f) {
    const CAABox searchBounds(pos - extent * scale, pos + extent * scale);
    CAreaCollisionCache cache(searchBounds);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (FindBestSurface(cache, pos, 2.f * (extent * scale).Magnitude(), result)) {
      return result;
    }
  }
  return result;
}

void CWallCrawlerSwarm::CreateBoid(CStateManager& mgr, int index) {
  const CAABox bounds = GetBoundingBox();
  const TUniqueId waypointId = GetWaypointForState(kSS_Patrol, mgr);
  if (const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypointId))) {
    const CVector3f pos = waypoint->GetTranslation();
    const CCollisionSurface surface = FindBestCollisionInBox(mgr, pos);
    const CVector3f projected = ProjectPointToPlane(pos, surface.GetVert(0), surface.GetNormal());
    const CVector3f normal = surface.GetNormal() * mBoidRadius;
    const CVector3f translation = projected + normal;
    mBoids[index].mTransform = CTransform4f::Translate(translation);
    if (close_enough(CVector3f::Dot(CVector3f(0.f, 0.f, 1.f), surface.GetNormal()), -1.f)) {
      mBoids[index].mTransform.SetRotation(
          CTransform4f::FromColumns(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, -1.f, 0.f),
                                    CVector3f(0.f, 0.f, -1.f), CVector3f::Zero()));
    } else {
      mBoids[index].mTransform.SetRotation(
          LookAt(CVector3f(0.f, 0.f, 1.f), surface.GetNormal(), CRelAngle(M_PIF)));
    }
    mBoids[index].mActive = true;
    mBoids[index].mVelocity = CVector3f::Zero();
    mBoids[index].mTargetWaypoint = waypointId;
    mBoids[index].mFramesNotOnSurface = 0;
    mBoids[index].mTimeToDie = 0.f;
    mBoids[index].mScarabExplodeTimerEnabled = false;
    mBoids[index].mHealth = mHealthInfo.GetHP();
  }
}

void CWallCrawlerSwarm::CalculateRenderBounds() {
  const CAABox bounds = GetBoundingBox();
  SetRenderBounds(bounds);
}

CAABox CWallCrawlerSwarm::GetBoundingBox() const {
  const CVector3f extent(0.75f * mBoundingBoxExtent.GetX(),
                         0.75f * mBoundingBoxExtent.GetY(),
                         0.75f * mBoundingBoxExtent.GetZ());
  const CAABox bounds(-extent, extent);
  return bounds.GetTransformedAABox(GetTransform());
}

rstl::optional_object< CAABox > CWallCrawlerSwarm::GetTouchBounds() const { return mAabox; }

CVector3f CWallCrawlerSwarm::ProjectPointToPlane(const CVector3f& point,
                                                 const CVector3f& planePoint,
                                                 const CVector3f& normal) {
  return point - CVector3f::Dot(point - planePoint, normal) * normal;
}

CVector3f CWallCrawlerSwarm::ProjectVectorToPlane(const CVector3f& point, const CVector3f& normal) {
  return point - CVector3f::Dot(point, normal) * normal;
}

bool CWallCrawlerSwarm::PointOnSurface(const CCollisionSurface& surface, const CVector3f& pos,
                                       const CPlane& plane) {
  const CVector3f projected = ProjectPointToPlane(pos, surface.GetVert(0), plane.GetNormal());
  for (int i = 0; i < 3; ++i) {
    const int next = i + 2;
    const int previous = next == 2 ? next : next - 3;
    const CVector3f edge2 = surface.GetVert(previous) - surface.GetVert(i);
    const CVector3f edge1 = projected - surface.GetVert(i);
    const CVector3f cross = CVector3f::Cross(edge1, edge2);
    if (CVector3f::Dot(plane.GetNormal(), cross) < 0.f) {
      return false;
    }
  }
  return true;
}

bool CWallCrawlerSwarm::CanRenderUnsorted(const CStateManager&) const { return true; }

TUniqueId CWallCrawlerSwarm::GetWaypointForState(EScriptObjectState state, CStateManager& mgr) {
  for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
    if (it->mState == state && it->mMsg == kSM_Follow) {
      return mgr.GetIdForScript(it->mObjId);
    }
  }
  return kInvalidUniqueId;
}

CAABox CWallCrawlerSwarm::BoxForPosition(int x, int y, int z, float margin) const {
  const CAABox bounds = GetBoundingBox();
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const CVector3f size(extent[0] / 5.f, extent[1] / 5.f, extent[2] / 5.f);
  return CAABox(CVector3f(x * size[0] + bounds.GetMinPoint()[0] - margin,
                          y * size[1] + bounds.GetMinPoint()[1] - margin,
                          z * size[2] + bounds.GetMinPoint()[2] - margin),
                CVector3f((x + 1) * size[0] + bounds.GetMinPoint()[0] + margin,
                          (y + 1) * size[1] + bounds.GetMinPoint()[1] + margin,
                          (z + 1) * size[2] + bounds.GetMinPoint()[2] + margin));
}

void CWallCrawlerSwarm::UpdatePartition() {
  mPartitionedBoidLists.clear();
  for (int i = 0; i < 125; ++i) {
    mPartitionedBoidLists.push_back(nullptr);
  }
  mOutlierBoidList = nullptr;
  const CAABox bounds = GetBoundingBox();
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const CVector3f size(extent.GetX() / 5.f, extent.GetY() / 5.f, extent.GetZ() / 5.f);
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    if (it->GetActive()) {
      const CVector3f& pos = it->GetTranslation();
      const CVector3f delta = pos - bounds.GetMinPoint();
      const int x = CCast::ToInt32(delta.GetX() / size.GetX());
      const int y = CCast::ToInt32(delta.GetY() / size.GetY());
      const int z = CCast::ToInt32(delta.GetZ() / size.GetZ());
      const int index = x + 5 * y + 25 * z;
      if (index < 0 || index >= 125 || x < 0 || x >= 5 || y < 0 || y >= 5 || z < 0 || z >= 5) {
        it->mNext = mOutlierBoidList;
        mOutlierBoidList = &*it;
      } else {
        it->mNext = mPartitionedBoidLists[index];
        mPartitionedBoidLists[index] = &*it;
      }
    }
  }
}

CVector3f CWallCrawlerSwarm::FindClosestCell(const CVector3f& pos) const {
  float minDistance = FLT_MAX;
  CVector3f result = CVector3f::Zero();
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        if (mPartitionedBoidLists[rowIndex + z * 25] != nullptr) {
          const CAABox bounds = BoxForPosition(x, y, z, 0.1f);
          const float distance = (bounds.GetCenterPoint() - pos).MagSquared();
          if (distance < minDistance) {
            result = bounds.GetCenterPoint();
            minDistance = distance;
          }
        }
      }
    }
  }
  return result;
}

CWallCrawlerSwarm::CBoid* CWallCrawlerSwarm::GetListAt(const CVector3f& pos) {
  const CAABox bounds = GetBoundingBox();
  const CVector3f delta = pos - bounds.GetMinPoint();
  const int index = CCast::ToInt32(delta.GetX() / (bounds.GetWidth() / 5.f)) +
                    CCast::ToInt32(delta.GetY() / (bounds.GetHeight() / 5.f)) * 5 +
                    CCast::ToInt32(delta.GetZ() / (bounds.GetDepth() / 5.f)) * 25;
  if (index < 0 || index >= 125) {
    return mOutlierBoidList;
  }
  return mPartitionedBoidLists[index];
}

void CWallCrawlerSwarm::BuildBoidNearList(const CBoid& boid, float radius,
                                          rstl::reserved_vector< CBoid*, 50 >& nearList) {
  CBoid* other = GetListAt(boid.GetTranslation());
  const CVector3f pos = boid.GetTranslation();
  while (other != nullptr && nearList.size() < 50) {
    const float distance = (other->GetTranslation() - pos).MagSquared();
    if (distance != 0.f && distance < radius) {
      nearList.push_back(other);
    }
    other = other->mNext;
  }
}

void CWallCrawlerSwarm::ApplySeparation(CBoid& boid,
                                        const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                        CVector3f& ahead) {
  if (nearList.size() > 0) {
    CVector3f closest(0.f, 0.f, 0.f);
    float minDistance = FLT_MAX;
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      const CVector3f delta = boid.GetTranslation() - (*it)->GetTranslation();
      const float distance = delta.MagSquared();
      if (distance != 0.f && distance < minDistance) {
        minDistance = distance;
        closest = (*it)->GetTranslation();
      }
    }
    ApplySeparation(boid, closest, mSeparationRadius, mSeparationMagnitude, ahead);
  }
}

void CWallCrawlerSwarm::ApplySeparation(CBoid& boid, const CVector3f& pos, float radius,
                                        float magnitude, CVector3f& ahead) {
  const CVector3f delta = boid.GetTranslation() - pos;
  if (delta.CanBeNormalized()) {
    const float distance = delta.MagSquared();
    const float radiusSquared = radius * radius;
    if (distance < radiusSquared) {
      const float factor = 1.f - distance / radiusSquared;
      ahead += factor * delta.AsNormalized() * magnitude;
    }
  }
}

void CWallCrawlerSwarm::ApplyCohesion(CBoid& boid,
                                      const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                      CVector3f& ahead) {
  if (nearList.size() > 0) {
    CVector3f center(0.f, 0.f, 0.f);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      center += (*it)->GetTranslation();
    }
    center = (1.f / nearList.size()) * center;
    ApplyCohesion(boid, center, mSeparationRadius, mCohesionMagnitude, ahead);
  }
}

void CWallCrawlerSwarm::ApplyCohesion(CBoid& boid, const CVector3f& pos, float radius,
                                      float magnitude, CVector3f& ahead) {
  const CVector3f delta = pos - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distance = delta.MagSquared();
    const float radiusSquared = radius * radius;
    const float factor = distance > radiusSquared ? 1.f : distance / radiusSquared;
    ahead += factor * delta.AsNormalized() * magnitude;
  }
}

void CWallCrawlerSwarm::ApplyAttraction(CBoid& boid, const CVector3f& pos, float radius,
                                        float magnitude, CVector3f& ahead) {
  const CVector3f delta = pos - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distance = delta.MagSquared();
    const float radiusSquared = radius * radius;
    const float factor = distance > radiusSquared ? 0.f : 1.f - distance / radiusSquared;
    ahead += factor * delta.AsNormalized() * magnitude;
  }
}

void CWallCrawlerSwarm::ApplyAlignment(CBoid& boid,
                                       const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                       CVector3f& ahead) {
  if (nearList.size() > 0) {
    CVector3f direction(0.f, 0.f, 0.f);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      direction += (*it)->GetTransform().GetForward();
    }
    direction = (1.f / nearList.size()) * direction;
    const float angle =
        CVector3f::GetAngleDiff(boid.GetTransform().GetForward(), direction) / M_PIF;
    ahead += angle * (mAlignmentWeight * direction);
  }
}

void CWallCrawlerSwarm::HardwareLight(const CStateManager& mgr, const CAABox& bounds) const {
  CActorLights lights(8, CVector3f::Zero(), 4, 4);
  lights.SetNeedsRelight(true);
  lights.SetCastShadows(false);
  lights.SetFindShadowLight(false);
  lights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()), bounds);
  lights.BuildDynamicLightList(mgr, bounds);
  lights.ActivateLights();
}

CColor CWallCrawlerSwarm::SoftwareLight(const CStateManager& mgr, const CAABox& bounds) const {
  CActorLights lights(8, CVector3f::Zero(), 4, 4);
  lights.SetNeedsRelight(true);
  lights.SetCastShadows(false);
  lights.SetFindShadowLight(false);
  lights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()), bounds);
  lights.BuildDynamicLightList(mgr, bounds);
  CColor result = lights.GetAmbientColor();
  const CVector3f center = bounds.GetCenterPoint();
  for (uint i = 0; i < lights.GetActiveLightCount(); ++i) {
    const CLight& light = lights.GetLight(i);
    const float distance = (light.GetPosition() - center).Magnitude();
    const float attenuation =
        rstl::min_val(1.f, 1.f / (distance * (distance * light.GetAttenuationQuadratic()) +
                                 (distance * light.GetAttenuationLinear() +
                                  light.GetAttenuationConstant())));
    result = CColor::Add(result,
                         CColor::Lerp(CColor::Black(), light.GetColor(), 0.8f * attenuation));
  }
  return result;
}

void CWallCrawlerSwarm::Render(const CStateManager& mgr) const {
  uint drawMask = ~0;
  const bool enableLighting = mEnableLighting;
  const bool useSoftwareLight = mUseSoftwareLight;
  if (!enableLighting) {
    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(CColor(0.5f, 0.5f, 0.5f, 1.f));
  }
  bool thermalHot = mgr.GetThermalDrawFlag() == kTD_Hot;
  CModelFlags flags = CModelFlags::Normal();
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    flags = CModelFlags::AlphaBlended(0.3f);
  }
  const uint lights = CGraphics::GetLightMask();
  CGX::SetChanCtrl(CGX::Channel0,
                   (lights && enableLighting && !useSoftwareLight) ? GX_TRUE : GX_FALSE, GX_SRC_REG,
                   GX_SRC_REG, static_cast< GXLightID >(lights), lights ? GX_DF_CLAMP : GX_DF_NONE,
                   lights ? GX_AF_SPOT : GX_AF_NONE);
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        const int index = rowIndex + z * 25;
        CBoid* boid = mPartitionedBoidLists[index];
        if (boid != nullptr) {
          if (enableLighting) {
            const CAABox bounds = BoxForPosition(x, y, z, 0.f);
            if (useSoftwareLight) {
              if ((index & 3) == (mThinkCounter & 3)) {
                const CColor color = SoftwareLight(mgr, bounds);
                for (CBoid* it = boid; it != nullptr; it = it->mNext) {
                  if (it->GetActive()) {
                    it->mAmbientLighting = CColor::Lerp(it->mAmbientLighting, color, 0.3f);
                  }
                }
              }
            } else {
              HardwareLight(mgr, bounds);
            }
          }
          for (; boid != nullptr; boid = boid->mNext) {
            if (boid->mInFrustum && boid->mActive) {
              RenderBoid(boid, drawMask, thermalHot, flags);
            }
          }
        }
      }
    }
  }
  CBoid* boid = mOutlierBoidList;
  int index = 0;
  for (; boid != nullptr; boid = boid->mNext) {
    ++index;
    if (boid->mInFrustum && boid->mActive) {
      if (enableLighting) {
        const CVector3f pos = boid->GetTranslation();
        const CVector3f extent(mBoidRadius, mBoidRadius, mBoidRadius);
        const CAABox bounds = CAABox(pos - extent, pos + extent);
        if (useSoftwareLight) {
          if ((index & 3) == (mThinkCounter & 3)) {
            const CColor color = SoftwareLight(mgr, bounds);
            if (boid->GetActive()) {
              boid->mAmbientLighting = CColor::Lerp(boid->mAmbientLighting, color, 0.3f);
            }
          }
        } else {
          HardwareLight(mgr, bounds);
        }
      }
      RenderBoid(boid, drawMask, thermalHot, flags);
    }
  }
  CGraphics::DisableAllLights();
  DrawTouchBounds();
}

void CWallCrawlerSwarm::RenderBoid(CBoid* boid, uint& drawMask, bool thermalHot,
                                   const CModelFlags& flags) const {
  uint index = boid->mIndex & 3;
  uint mask = drawMask;
  if (boid->mLaunched) {
    index = 8;
  } else if (boid->mTimeToDie > 0.f) {
    index = 9;
  } else if (boid->mScarabExplodeTimerEnabled || boid->mNearPlayer) {
    index += 4;
  }
  CModelData& data = *mModelDatas[index];
  CAnimData& animData = *data.AnimationData();
  CSkinnedModel& model = data.PickAnimatedModel(mWhichModel);
  if (mask & (1 << index)) {
    mask &= ~(1 << index);
    animData.BuildPose();
    model.Calculate(animData.GetPose(), rstl::optional_object< CVertexMorphEffect >(), nullptr,
                    mPosWorkspaces[index].get());
  }
  gpRender->SetAmbientColor(boid->mAmbientLighting);
  gpRender->SetModelMatrix(boid->GetTransform());
  const float* const positions = mPosWorkspaces[index].get();
  const float* const normals = mNrmWorkspaces[index];
  const float timeToDie = boid->mTimeToDie;
  if (timeToDie > 0.f && !thermalHot) {
    model.Draw(positions, normals, CModelFlags::Normal());
    if (animData.GetIceModel()) {
      gpRender->SetAmbientColor(CColor::White());
      const CSkinnedModel& iceModel = (*animData.GetIceModel())->GetSkinnedModel();
      animData.Render(iceModel,
                      CModelFlags::AlphaBlended(
                          CColor(1.f, 1.f, 1.f, CMath::FastFSel(1.f - timeToDie, timeToDie, 1.f)))
                          .DepthCompareUpdate(true, true),
                      rstl::optional_object< CVertexMorphEffect >(), nullptr);
    }
  } else if (thermalHot) {
    CModelData::ThermalDraw(model, positions, normals, CColor(0xffffffff),
                            CColor(uchar(0), uchar(0), uchar(0), uchar(64)),
                            CModelFlags::Normal().DepthCompareUpdate(true, false));
  } else {
    model.Draw(positions, normals, flags);
  }
  drawMask = mask;
}

void CWallCrawlerSwarm::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (mModelAssetDirty) {
    const CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    if (mWhichModel != which) {
      AllocateSkinnedModels(mgr, which);
    }
  }
  SetTransformDirty(true);
  mBoidGenCooldownTimer -= dt;
  mCrabDamageCooldownTimer -= dt;
  ++mThinkCounter;
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    if (mOccludedTimer > 0.f) {
      mOccludedTimer -= dt;
    }
    if (mOccludedTimer <= 0.f) {
      return;
    }
    if (mThinkCounter & 2) {
      return;
    }
  } else {
    mOccludedTimer = 7.f;
  }
  UpdateParticles(dt);
  mLockOnIdx = GetLockOnIndex(mgr);
  SetTargetable(mLockOnIdx != -1);
  if (mLockOnIdx == -1) {
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  } else {
    AddMaterial(kMT_Target, kMT_Orbit, mgr);
  }
  while ((mMaxCreatedBoids == 0 || mCreatedBoids < mMaxCreatedBoids) &&
         mBoidGenCooldownTimer <= 0.f) {
    bool created = false;
    for (int i = 0; i < mBoids.size(); ++i) {
      if (!mBoids[i].GetActive()) {
        CreateBoid(mgr, i);
        ++mCreatedBoids;
        mBoidGenCooldownTimer += 1.f / mBoidGenRate;
        created = true;
        break;
      }
    }
    if (!created) {
      mBoidGenCooldownTimer += 1.f / mBoidGenRate;
      break;
    }
  }
  UpdatePartition();
  const CAABox bounds = GetBoundingBox();
  int count = 0;
  mAabox = GetBoundingBox();
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        CBoid* boid = mPartitionedBoidLists[rowIndex + z * 25];
        if (boid != nullptr) {
          CAreaCollisionCache cache(BoxForPosition(x, y, z, 0.5f + mBoidRadius));
          CGameCollision::BuildAreaCollisionCache(mgr, cache);
          for (; boid != nullptr; boid = boid->mNext) {
            ++count;
            if (boid->GetActive()) {
              if (mFlavor == kF_Scarab) {
                const CVector3f extent(mScarabBoxMargin, mScarabBoxMargin,
                                       mScarabBoxMargin);
                mAabox.AccumulateBounds(boid->GetTranslation() + extent);
                mAabox.AccumulateBounds(boid->GetTranslation() - extent);
              } else {
                mAabox.AccumulateBounds(boid->GetTranslation());
              }
            }
            if (((mThinkCounter & 1) == (count & 1) && boid->mActive &&
                 boid->mTimeToDie < 0.1f) ||
                boid->mLaunched) {
              UpdateBoid(cache, mgr, dt, *boid);
            }
          }
        }
      }
    }
  }
  for (CBoid* boid = mOutlierBoidList; boid != nullptr; boid = boid->mNext) {
    ++count;
    if (boid->GetActive()) {
      mAabox.AccumulateBounds(boid->GetTranslation());
    }
    if (((mThinkCounter & 1) == (count & 1) && boid->mActive &&
         boid->mTimeToDie < 0.1f) ||
        boid->mLaunched) {
      const float margin = 1.5f + (0.5f + mBoidRadius);
      const CVector3f extent(margin, margin, margin);
      const CAABox boidBounds(boid->GetTranslation() - extent, boid->GetTranslation() + extent);
      CAreaCollisionCache cache(boidBounds);
      CGameCollision::BuildAreaCollisionCache(mgr, cache);
      UpdateBoid(cache, mgr, dt, *boid);
    }
  }
  mModelDatas[8]->AnimationData()->SetPlaybackRate(mAnimPlaybackSpeed);
  mModelDatas[8]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
  CAdvancementDeltas normalDelta;
  CAdvancementDeltas attractDelta;
  int normalCount = 0;
  int attractCount = 0;
  int index = 0;
  bool normalModels[4] = {false, false, false, false};
  bool attractModels[4] = {false, false, false, false};
  AUTO(it, mBoids.begin());
  const rstl::vector< CBoid >::const_iterator end = mBoids.end();
  for (; it != end; ++it, ++index) {
    if (it->mActive && !it->mLaunched) {
      if (it->mScarabExplodeTimerEnabled || it->mNearPlayer) {
        attractModels[index & 3] = true;
        ++attractCount;
      } else {
        normalModels[index & 3] = true;
        ++normalCount;
      }
    }
  }
  for (int i = 0; i < 4; ++i) {
    mModelDatas[i]->AnimationData()->SetPlaybackRate(mAnimPlaybackSpeed);
    normalDelta = mModelDatas[i]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    mModelDatas[i + 4]->AnimationData()->SetPlaybackRate(mAnimPlaybackSpeed);
    attractDelta = mModelDatas[i + 4]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    if (mModelDatas[i]->HasAnimation() && normalModels[i]) {
      UpdateEffects(mgr, *mModelDatas[i]->AnimationData(),
                    normalCount * 44 / mNumBoids + 83);
    }
    if (mModelDatas[i + 4]->HasAnimation() && attractModels[i]) {
      UpdateEffects(mgr, *mModelDatas[i + 4]->AnimationData(),
                    attractCount * 44 / mNumBoids + 83);
    }
    for (int j = i; j < mBoids.size(); j += 4) {
      if (mBoids[j].mActive) {
        if (mBoids[j].mLaunched) {
          mBoids[j].mTransform.AddTranslation(dt * mBoids[j].mVelocity);
        } else if (mBoids[j].mTimeToDie > 0.f) {
          mBoids[j].mTimeToDie -= dt;
          if (mBoids[j].mTimeToDie < 0.7f * mgr.Random()->Float()) {
            KillBoid(mBoids[j], mgr, 1.f, 0.05f);
          }
        } else if (mBoids[j].mScarabExplodeTimerEnabled ||
                   mBoids[j].mNearPlayer) {
          mBoids[j].mVelocity =
              1.5f * mBoids[j].GetTransform().Rotate(attractDelta.GetOffsetDelta()) / dt;
          mBoids[j].mTransform.AddTranslation(dt * mBoids[j].mVelocity);
        } else {
          mBoids[j].mVelocity =
              1.5f * mBoids[j].GetTransform().Rotate(normalDelta.GetOffsetDelta()) / dt;
          mBoids[j].mTransform.AddTranslation(dt * mBoids[j].mVelocity);
        }
      }
    }
  }
  if (mFlavor == kF_Crab) {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
      if (it->mActive && close_enough(it->mTimeToDie, 0.f) && !it->mLaunched) {
        const CVector3f pos = it->GetTranslation();
        if ((playerPos - pos).Magnitude() < mAttractionRadius) {
          it->mNearPlayer = true;
        } else {
          it->mNearPlayer = false;
        }
      }
    }
  }
  if (mFlavor == kF_Parasite && mMaxLaunches > 0) {
    const CVector3f target = mgr.GetPlayer()->GetTranslation() + CVector3f(0.f, 0.f, 1.f);
    static const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    int launched = 0;
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
      if (it->mActive && it->mLaunched) {
        ++launched;
      }
    }
    if (launched < mMaxLaunches) {
      for (AUTO(it, mBoids.begin()); it != mBoids.end() && launched < mMaxLaunches;
           ++it) {
        if (it->mActive && close_enough(it->mTimeToDie, 0.f) && !it->mLaunched &&
            (it->GetTranslation() - target).MagSquared() < 18.f * 18.f &&
            mgr.Random()->Float() <= 0.02f) {
          const CVector3f pos = it->GetTranslation();
          const float distance = (target - pos).Magnitude();
          if (mgr.RayStaticIntersection(pos, (target - pos) / distance, distance, filter)
                  .IsInvalid()) {
            LaunchBoid(*it, target);
            ++launched;
          }
        }
      }
    }
  }
}

void CWallCrawlerSwarm::PreRender(CStateManager&, const CFrustumPlanes& frustum) {
  bool active = false;
  for (int i = 0; i < 5; ++i) {
    mModelDatas[i]->AnimationData()->PreRender();
  }
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    if (it->mActive) {
      it->mInFrustum =
          frustum.SphereInFrustumPlanes(CSphere(it->GetTranslation(), 2.f * mBoidRadius));
      active = true;
    } else {
      it->mInFrustum = false;
    }
  }
  SetPreRenderClipped(!active);
}

void CWallCrawlerSwarm::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  if (GetActive()) {
    RenderParticles();
    if (!GetPreRenderClipped()) {
      if (CanRenderUnsorted(mgr)) {
        Render(mgr);
      } else {
        EnsureRendered(mgr);
      }
    }
  }
}

void CWallCrawlerSwarm::UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt,
                                   CBoid& boid) {
  if (boid.mScarabExplodeTimerEnabled) {
    if (mFlavor == kF_Scarab && boid.mTimeToExplode > 0.f) {
      boid.mTimeToExplode -= 2.f * dt;
      if (boid.mTimeToExplode <= 0.f) {
        ExplodeBoid(boid, mgr);
      }
    }
  } else if (boid.mLaunched) {
    const float radius = 2.f * mBoidRadius;
    const float boidRadius = mBoidRadius;
    const float speed = boid.mVelocity.Magnitude();
    float distance = speed * dt;
    CVector3f pos = boid.GetTranslation();
    const CVector3f step = (-boid.mVelocity / speed) * boidRadius;
    bool found = false;
    while (distance >= 0.f && !found) {
      CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                                CVector3f(0.f, 0.f, 1.f), ~0);
      const CVector3f move = dt * boid.mVelocity;
      const CVector3f offset = 1.5f * move;
      const CVector3f predicted = pos + offset;
      if (FindBestSurface(cache, predicted, radius, surface) &&
          boid.mRemainingLaunchNotOnSurfaceFrames == 0) {
        if (mFlavor != kF_Scarab) {
          boid.mTransform = LookAt(boid.GetTransform().GetUp(), surface.GetNormal(),
                                     CRelAngle::FromRadians(M_PIF))
                                  .MultiplyIgnoreTranslation(boid.GetTransform());
        }
        const CPlane plane = surface.GetPlane();
        const CVector3f correction =
            -(plane.GetHeight(boid.GetTranslation()) - boidRadius - 0.01f) * plane.GetNormal();
        boid.mTransform.AddTranslation(correction);
        boid.mFramesNotOnSurface = 0;
        boid.mLaunched = false;
        if (mFlavor == kF_Scarab) {
          boid.mScarabExplodeTimerEnabled = true;
          boid.mTimeToExplode = mScarabTimeToExplode;
          CSfxManager::AddEmitter(mScatterSfx, boid.GetTranslation(), CVector3f::Zero(), true,
                                  false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
        }
        found = true;
      }
      distance -= boidRadius;
      pos += step;
    }
    if (!found) {
      const float gravity = CPhysicsActor::GravityConstant();
      float acceleration = gravity;
      if (mFlavor == kF_Scarab)
        acceleration = 3.f * gravity;
      boid.mVelocity += dt * CVector3f(0.f, 0.f, -acceleration);
      if (boid.mRemainingLaunchNotOnSurfaceFrames != 0) {
        boid.mRemainingLaunchNotOnSurfaceFrames--;
      }
    }
  } else if (boid.mFramesNotOnSurface >= 30) {
    boid.mActive = false;
  } else {
    const float radius = 2.f * mBoidRadius;
    const float boidRadius = mBoidRadius;
    const CVector3f pos = boid.GetTranslation();
    bool found = false;
    CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                              CVector3f(0.f, 0.f, 1.f), ~0);
    const CVector3f move = dt * boid.mVelocity;
    const CVector3f offset = 1.5f * move;
    const CVector3f predicted = pos + offset;
    if (FindBestSurface(cache, predicted, radius, surface)) {
      boid.mSurface = surface;
      boid.mTransform = LookAt(boid.GetTransform().GetUp(), surface.GetNormal(),
                                 CRelAngle::FromDegrees(180.f * dt))
                              .MultiplyIgnoreTranslation(boid.GetTransform());
      const CPlane plane = surface.GetPlane();
      const float distance = plane.GetHeight(boid.GetTranslation());
      if (distance <= 1.5f * mBoidRadius) {
        const CVector3f correction = -(distance - boidRadius - 0.01f) * plane.GetNormal();
        boid.mTransform.AddTranslation(correction);
        boid.mFramesNotOnSurface = 0;
        found = true;
      }
    }
    if (!found) {
      const float angularSpeed = boid.mVelocity.Magnitude() / boidRadius;
      boid.mTransform = LookAt(boid.GetTransform().GetUp(), boid.GetTransform().GetForward(),
                                 CRelAngle(angularSpeed * dt))
                              .MultiplyIgnoreTranslation(boid.GetTransform());
      ++boid.mFramesNotOnSurface;
    }
    rstl::reserved_vector< CBoid*, 50 > nearList;
    BuildBoidNearList(boid, mSeparationRadius, nearList);
    CVector3f ahead = 0.3f * boid.GetTransform().GetForward();
    for (int i = 0; i < 8; ++i) {
      switch (i) {
      case 0:
        for (AUTO(it, mDoorRepulsors.begin()); it != mDoorRepulsors.end(); ++it) {
          if ((it->mCenter - boid.GetTranslation()).MagSquared() <
              it->mMagnitude * it->mMagnitude) {
            ApplySeparation(boid, it->mCenter, it->mMagnitude, 4.5f, ahead);
          }
        }
        break;
      case 4:
        ApplySeparation(boid, nearList, ahead);
        break;
      case 5:
        MoveToWayPoint(boid, mgr, ahead);
        break;
      case 6:
        ApplyCohesion(boid, nearList, ahead);
        break;
      case 7:
        ApplyAlignment(boid, nearList, ahead);
        break;
      case 3:
        ApplyAttraction(boid, mgr.GetPlayer()->GetTranslation(), mAttractionRadius,
                        mAttractionMagnitude, ahead);
        break;
      default:
        break;
      }
      if (ahead.MagSquared() >= 9.f) {
        break;
      }
    }
    const CVector3f projected = ProjectVectorToPlane(ahead, boid.GetTransform().GetUp());
    const CVector3f forward = boid.GetTransform().GetForward();
    const CVector3f direction = projected.AsNormalized();
    boid.mTransform = LookAt(forward, direction, CRelAngle::FromRadians(M_PIF * dt))
                            .MultiplyIgnoreTranslation(boid.GetTransform());
  }
}

void CWallCrawlerSwarm::LaunchBoid(CBoid& boid, const CVector3f& dir) {
  const CVector3f pos = boid.GetTranslation();
  static float attackTime = 2.f * CMath::SqrtF(2.5f / CPhysicsActor::GravityConstant());
  static float attackVelocity = 15.f / attackTime;
  const float gravity = -CPhysicsActor::GravityConstant();
  const CVector3f difference = dir - pos;
  const float deltaZ = difference.GetZ();
  CVector3f delta(difference.GetX(), difference.GetY(), 0.f);
  const float distance = delta.Magnitude();
  boid.mTransform.SetRotation(CTransform4f::Identity());
  boid.mTransform =
      LookAt(boid.GetTransform().GetForward(), delta.AsNormalized(), CRelAngle::FromRadians(M_PIF))
          .MultiplyIgnoreTranslation(boid.GetTransform());
  const CVector3f forward = boid.GetTransform().GetForward();
  CVector3f velocity = attackVelocity * forward;
  velocity.SetZ(0.5f * attackVelocity);
  if (distance > FLT_EPSILON) {
    delta /= distance;
    const float dot = CVector3f::Dot(delta, velocity);
    if (dot > FLT_EPSILON) {
      float time = 0.f;
      const bool below = deltaZ < 0.f;
      float positive, negative;
      if (CSteeringBehaviors::SolveQuadratic(gravity, velocity.GetZ(), -deltaZ, positive,
                                             negative)) {
        time = below ? negative : positive;
      }
      if (!below) {
        time += distance / dot;
      }
      if (time < 10.f) {
        const CVector3f flatVelocity = (distance / time) * delta;
        velocity.SetX(0.6f * flatVelocity.GetX());
        velocity.SetY(0.6f * flatVelocity.GetY());
        velocity.SetZ(-(0.5f * gravity * time - deltaZ / time));
      }
    }
  }
  boid.mVelocity = velocity;
  boid.mLaunched = true;
  boid.mRemainingLaunchNotOnSurfaceFrames = 1;
  CSfxManager::AddEmitter(mLaunchSfx, pos, CVector3f::Zero(), true, false,
                          CSfxManager::kMedPriority, GetCurrentAreaId().Value());
}

void CWallCrawlerSwarm::ScatterScarabBoid(CBoid& boid, CStateManager& mgr) {
  const CVector3f oldDirection = boid.GetTransform().GetForward();
  boid.mTransform.SetRotation(CTransform4f::Identity());
  boid.mTransform = LookAt(boid.GetTransform().GetForward(), oldDirection, CRelAngle(M_PIF))
                          .MultiplyIgnoreTranslation(boid.GetTransform());
  boid.mVelocity = CVector3f::Zero();
  const float angle = mgr.Random()->Float() * (2.f * M_PIF);
  const float speed = mgr.Random()->Float() * mScarabScatterXYVelocity;
  boid.mVelocity.SetX(speed * CMath::FastCosR(angle));
  boid.mVelocity.SetY(speed * CMath::FastSinR(angle));
  boid.mLaunched = true;
  boid.mRemainingLaunchNotOnSurfaceFrames = 5;
  CSfxManager::AddEmitter(mLaunchSfx, boid.GetTranslation(), CVector3f::Zero(), true, false,
                          CSfxManager::kMedPriority, GetCurrentAreaId().Value());
}

void CWallCrawlerSwarm::MoveToWayPoint(CBoid& boid, CStateManager& mgr, CVector3f& ahead) {
  if (CScriptWaypoint* waypoint =
          TCastToPtr< CScriptWaypoint >(mgr.ObjectById(boid.mTargetWaypoint))) {
    if ((waypoint->GetTranslation() - boid.GetTranslation()).MagSquared() <
        mWaypointGoalRadius * mWaypointGoalRadius) {
      boid.mTargetWaypoint = waypoint->NextWaypoint(mgr);
      if (boid.mTargetWaypoint == kInvalidUniqueId) {
        if (mFlavor == kF_Scarab) {
          ScatterScarabBoid(boid, mgr);
        } else {
          boid.mActive = false;
          return;
        }
      } else {
        waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(boid.mTargetWaypoint));
      }
    }
    ahead += (waypoint->GetTranslation() - boid.GetTranslation()).AsNormalized() *
             mMoveToWaypointWeight;
  }
}

void CWallCrawlerSwarm::Touch(CActor& actor, CStateManager& mgr) {
  CActor::Touch(actor, mgr);
  if (const CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
    const CDamageInfo& damage = projectile->GetCurrentDamageInfo();
    if (mDamageVulnerability.WeaponHurts(damage.GetWeaponMode(),
                                             CDamageVulnerability::kRD_No)) {
      const rstl::optional_object< CAABox > touchBounds = projectile->GetTouchBounds();
      if (touchBounds) {
        const CAABox projectileBounds = *touchBounds;
        const float radius = 0.1f + mTouchRadius;
        const float radiusSq = radius * radius;
        const CVector3f extent(radiusSq, radiusSq, radiusSq);
        for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
          if (it->GetActive()) {
            const CVector3f pos = it->GetTranslation();
            const CAABox bounds(pos - extent, pos + extent);
            if (bounds.DoBoundsOverlap(projectileBounds)) {
              it->mHealth -= damage.GetDamage(mDamageVulnerability);
              if (it->mHealth <= 0.f) {
                KillBoid(*it, mgr, 1.f, 0.1f);
              }
            }
          }
        }
      }
    }
  }
  if (const CPlayer* player = TCastToPtr< CPlayer >(actor)) {
    const float radius =
        close_enough(mPlayerTouchRadius, 0.f) ? mTouchRadius : mPlayerTouchRadius;
    const CAABox playerBounds = *player->GetTouchBounds();
    const CVector3f scarabExtent(mScarabBoxMargin, mScarabBoxMargin, mScarabBoxMargin);
    const CVector3f extent(radius, radius, radius);
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
      if (it->mActive && it->mTimeToDie <= 0.f) {
        if (mFlavor == kF_Scarab && it->mScarabExplodeTimerEnabled) {
          const CAABox bounds =
              CAABox(it->GetTranslation() - scarabExtent, it->GetTranslation() + scarabExtent);
          if (playerBounds.DoBoundsOverlap(bounds)) {
            ExplodeBoid(*it, mgr);
            SetExplodeTimers(it->GetTranslation(), 0.5f, 0.5f, 2.5f);
          }
        }
        const CAABox bounds = CAABox(it->GetTranslation() - extent, it->GetTranslation() + extent);
        if (playerBounds.DoBoundsOverlap(bounds)) {
          if (it->mLaunched && mFlavor == kF_Parasite) {
            mgr.ApplyDamage(
                GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
                CDamageInfo(CWeaponMode(kWT_AI), 0.00002f, 0.f, 0.f),
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                CVector3f::Zero());
            KillBoid(*it, mgr, 0.f, 1.f);
          } else if (mFlavor == kF_Scarab) {
            ExplodeBoid(*it, mgr);
          } else if (mCrabDamageCooldownTimer <= 0.f) {
            mgr.ApplyDamage(
                GetUniqueId(), player->GetUniqueId(), GetUniqueId(), mCrabDamage,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                CVector3f::Zero());
            mCrabDamageCooldownTimer = mCrabDamageCooldown;
            break;
          }
        }
      }
    }
  }
}

int CWallCrawlerSwarm::GetLockOnIndex(const CStateManager& mgr) const {
  const CTransform4f cameraXf = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
  const CVector3f cameraPos = cameraXf.GetTranslation();
  const CVector3f cameraForward = cameraXf.GetForward();
  if (mLockOnIdx != -1) {
    const CBoid& boid = mBoids[mLockOnIdx];
    if (boid.GetActive()) {
      const CVector3f delta = boid.GetTranslation() - cameraPos;
      const float distance = delta.Magnitude();
      const CVector3f dir = delta / distance;
      if (CVector3f::Dot(cameraForward, dir) > 0.9238795f) {
        const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
        if (mgr.RayStaticIntersection(cameraPos, dir, distance, filter).IsInvalid()) {
          return mLockOnIdx;
        }
      }
    }
    return -1;
  }
  float maxDot = 0.5f;
  int index = 0;
  int result = -1;
  const float maxDistance = mgr.GetPlayer()->GetOrbitMaxTargetDistance(mgr);
  const float maxDistanceSq = maxDistance * maxDistance;
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it, ++index) {
    if (it->GetActive()) {
      const CVector3f delta = it->GetTranslation() - cameraPos;
      if (delta.MagSquared() > maxDistanceSq) {
        continue;
      }
      if (delta.CanBeNormalized()) {
        const float dot = CVector3f::Dot(cameraForward, delta.AsNormalized());
        if (dot > maxDot) {
          result = index;
          maxDot = dot;
        }
      }
    }
  }
  return result;
}

void CWallCrawlerSwarm::ApplyRadiusDamage(CVector3f pos, const CDamageInfo& info,
                                          CStateManager& mgr) {
  const float radiusSquared = info.GetRadius() * info.GetRadius();
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    if (it->GetActive()) {
      const CVector3f boidPos = it->GetTranslation();
      if ((boidPos - pos).MagSquared() < radiusSquared) {
        it->mHealth -= info.GetRadiusDamage(mDamageVulnerability);
        if (it->mHealth <= 0.f) {
          KillBoid(*it, mgr, 1.f, 0.1f);
        }
      }
    }
  }
}

void CWallCrawlerSwarm::SetExplodeTimers(const CVector3f& pos, float radius, float minTime,
                                         float maxTime) {
  const float radiusSquared = radius * radius;
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    if (it->GetActive() && it->mTimeToDie <= 0.f) {
      const float distanceSquared = (it->GetTranslation() - pos).MagSquared();
      if (distanceSquared < radiusSquared) {
        const float time = (distanceSquared / radiusSquared) * (maxTime - minTime) + minTime;
        if (it->mTimeToExplode > time || it->mTimeToExplode == 0.f) {
          it->mTimeToExplode = time;
        }
      }
    }
  }
}

void CWallCrawlerSwarm::ExplodeBoid(CBoid& boid, CStateManager& mgr) {
  KillBoid(boid, mgr, 0.f, 1.f);
  mgr.ApplyDamageToWorld(GetUniqueId(), *this, boid.GetTranslation(), mScarabExplodeDamage,
                         CMaterialFilter::MakeInclude(CMaterialList(kMT_Player)));
}

CVector3f CWallCrawlerSwarm::GetAimPosition(const CStateManager&, float dt) const {
  if (mLockOnIdx == -1) {
    return mLastOrbitPosition;
  }
  return mLastOrbitPosition + dt * mBoids[mLockOnIdx].mVelocity;
}

CVector3f CWallCrawlerSwarm::GetOrbitPosition(const CStateManager&) const {
  if (mLockOnIdx == -1) {
    return mLastOrbitPosition;
  }
  mLastOrbitPosition = mBoids[mLockOnIdx].GetTranslation();
  return mLastOrbitPosition;
}

void CWallCrawlerSwarm::KillBoid(CBoid& boid, CStateManager& mgr, float deathRattleChance,
                                 float deadChance) {
  mLastKilledOffset = boid.GetTranslation();
  AddParticle(boid.GetTransform());
  boid.mActive = false;
  const float deadRoll = mgr.Random()->Float();
  const float deathRattleRoll = mgr.Random()->Float();
  if (deathRattleRoll < deathRattleChance) {
    SendScriptMsgs(kSS_DeathRattle, mgr, kSM_None);
  }
  if (deadRoll < deadChance) {
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
  }
}

void CWallCrawlerSwarm::AddParticle(const CTransform4f& xf) {
  static const int particleCounts[] = {8, 2, 0, 0};
  for (int i = 0; i < mParticleGens.size(); ++i) {
    mParticleGens[i]->SetParticleEmission(true);
    mParticleGens[i]->SetTranslation(xf.GetTranslation());
    mParticleGens[i]->ForceParticleCreation(particleCounts[i]);
    mParticleGens[i]->SetParticleEmission(false);
  }
}

void CWallCrawlerSwarm::UpdateParticles(float dt) {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    mParticleGens[i]->Update(dt);
  }
}

void CWallCrawlerSwarm::RenderParticles() const {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    gpRender->AddParticleGen(*mParticleGens[i]);
  }
}

void CWallCrawlerSwarm::FreezeCollision(const CMarkerGrid& grid, float duration) {
  const float radius = mTouchRadius * mTouchRadius;
  const float xy = radius + 0.3f;
  const float z = radius + 0.5f;
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    if (it->GetActive()) {
      const CVector3f extent(xy, xy, z);
      const CAABox bounds = CAABox(it->GetTranslation() - extent, it->GetTranslation() + extent);
      if (grid.AABoxTouchesData(bounds, 1)) {
        it->mTimeToDie = 1.f;
      }
    }
  }
}

void CWallCrawlerSwarm::UpdateEffects(CStateManager& mgr, CAnimData& animData, const int volume) {
  const CSoundPOINode* nodes;
  int count = 0;
  nodes = animData.GetSoundPOIList(count);
  if (count > 0 && nodes != nullptr) {
    for (int i = 0; i < count; ++i) {
      const CSoundPOINode& node = nodes[i];
      const int character = node.GetCharacterIndex();
      if (node.GetPoiType() != kPT_Sound) {
        continue;
      }
      if (character != -1 && character != animData.GetCharacterIndex()) {
        continue;
      }
      const uint soundId = node.GetSoundId();
      const int area = GetCurrentAreaId().Value();
      const ushort sfx = CSfxManager::TranslateSFXID(soundId);
      if ((soundId & 0x80000000) != 0) {
        continue;
      }
      const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
      const CVector3f pos = FindClosestCell(playerPos);
      static float maxDistance = node.GetMaxDistance();
      static float falloff = node.GetFallOff();
      CAudioSys::C3DEmitterParmData params(maxDistance, falloff, 1, CMath::Clamp(0, volume, 127),
                                           20);
      params.mPos = pos;
      params.mDir = CVector3f::Zero();
      params.mSfxId = sfx;
      CSfxManager::AddEmitter(params, true, CSfxManager::kMedPriority, false, area);
    }
  }
}
