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
: x0_transform(xf)
, x30_velocity(0.f, 0.f, 0.f)
, x3c_targetWaypoint(kInvalidUniqueId)
, x40_ambientLighting(0.3f, 0.3f, 0.3f, 1.f)
, x44_next(nullptr)
, x48_timeToDie(0.f)
, x4c_timeToExplode(0.f)
, x50_surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f), CVector3f(0.f, 0.f, 1.f), ~0)
, x7c_24_framesNotOnSurface(0)
, x7c_16_index(index)
, x80_24_active(false)
, x80_25_inFrustum(false)
, x80_26_launched(false)
, x80_27_scarabExplodeTimerEnabled(false)
, x80_28_nearPlayer(false) {}

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
, xe8_aabox(CVector3f(0.f, 0.f, 0.f), CVector3f(0.f, 0.f, 0.f))
, x104_occludedTimer(5.f)
, x118_boundingBoxExtent(boundingBoxExtent)
, x124_lastOrbitPosition(0.f, 0.f, 0.f)
, x130_lastKilledOffset(CVector3f::Zero())
, x13c_separationRadius(separationRadius)
, x140_cohesionMagnitude(cohesionMagnitude)
, x144_alignmentWeight(alignmentWeight)
, x148_separationMagnitude(separationMagnitude)
, x14c_moveToWaypointWeight(moveToWaypointWeight)
, x150_attractionMagnitude(attractionMagnitude)
, x154_attractionRadius(attractionRadius)
, x158_scarabScatterXYVelocity(scarabScatterXYVelocity)
, x15c_scarabTimeToExplode(scarabTimeToExplode)
, x160_animPlaybackSpeed(animPlaybackSpeed)
, x164_waypointGoalRadius(3.f)
, x168_partitionedBoidLists(nullptr)
, x360_outlierBoidList(nullptr)
, x364_boidGenRate(boidGenRate)
, x368_boidGenCooldownTimer(0.f)
, x36c_crabDamageCooldownTimer(0.f)
, x370_crabDamageCooldown(crabDamageCooldown)
, x374_boidRadius(boidRadius)
, x378_touchRadius(touchRadius)
, x37c_scarabBoxMargin(scarabBoxMargin)
, x380_playerTouchRadius(playerTouchRadius)
, x384_crabDamage(crabDamage)
, x3a0_scarabExplodeDamage(scarabExplodeDamage)
, x3bc_healthInfo(healthInfo)
, x3c4_damageVulnerability(damageVulnerability)
, x42c_lockOnIdx(-1)
, x4dc_whichModel(CModelData::kWM_Normal)
, x548_numBoids(numBoids)
, x54c_maxCreatedBoids(maxCreatedBoids)
, x550_createdBoids(0)
, x554_maxLaunches(maxLaunches)
, x558_flavor(static_cast< EFlavor >(flavor))
, x55c_launchSfx(
      CSfxManager::TranslateSFXID(launchSfx == -1 ? CSfxManager::kInternalInvalidSfxId : launchSfx))
, x55e_scatterSfx(CSfxManager::TranslateSFXID(scatterSfx == -1 ? CSfxManager::kInternalInvalidSfxId
                                                               : scatterSfx))
, x560_24_enableLighting(true)
, x560_25_useSoftwareLight(true)
, x560_26_modelAssetDirty(false) {
  const CAnimRes attractRes(animRes.GetId(), animRes.GetCharacterNodeId(), animRes.GetScale(),
                            attractAnim != -1 ? attractAnim : 0, true);
  const CAnimRes launchRes(animRes.GetId(), animRes.GetCharacterNodeId(), animRes.GetScale(),
                           launchAnim != -1 ? launchAnim : 0, true);
  x4b0_modelDatas.push_back(rs_new CModelData(animRes));
  x4b0_modelDatas.push_back(rs_new CModelData(animRes));
  x4b0_modelDatas.push_back(rs_new CModelData(animRes));
  x4b0_modelDatas.push_back(rs_new CModelData(animRes));
  x4b0_modelDatas.push_back(rs_new CModelData(attractRes));
  x4b0_modelDatas.push_back(rs_new CModelData(attractRes));
  x4b0_modelDatas.push_back(rs_new CModelData(attractRes));
  x4b0_modelDatas.push_back(rs_new CModelData(attractRes));
  x4b0_modelDatas.push_back(rs_new CModelData(launchRes));
  x4b0_modelDatas.push_back(rs_new CModelData(animRes));
  if (actParams.GetXRay().first != 0) {
    for (int i = 0; i < 9; ++i) {
      x4b0_modelDatas[i]->SetXRayModel(actParams.GetXRay());
    }
    x560_26_modelAssetDirty = true;
  }
  if (actParams.GetInfra().first != 0) {
    for (int i = 0; i < 9; ++i) {
      x4b0_modelDatas[i]->SetInfraModel(actParams.GetInfra());
    }
    x560_26_modelAssetDirty = true;
  }
  if (particle1 != kInvalidAssetId) {
    x4f0_particleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle1)));
  }
  if (particle2 != kInvalidAssetId) {
    x4f0_particleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle2)));
  }
  if (particle3 != kInvalidAssetId) {
    x4f0_particleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle3)));
  }
  if (particle4 != kInvalidAssetId) {
    x4f0_particleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle4)));
  }
  for (int i = 0; i < x4f0_particleDescs.size(); ++i) {
    x524_particleGens.push_back(rs_new CElementGen(x4f0_particleDescs[i]));
    x524_particleGens[i]->SetParticleEmission(false);
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
  x4e0_doorRepulsors.reserve(count);
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    CEntity* entity = objects[i];
    if (CScriptDoor* door = TCastToPtr< CScriptDoor >(entity)) {
      if (door->GetCurrentAreaId() == GetCurrentAreaId()) {
        rstl::optional_object< CAABox > bounds = door->GetTouchBounds();
        if (bounds.valid()) {
          float diagonal = (bounds->GetMinPoint() - bounds->GetMaxPoint()).Magnitude();
          x4e0_doorRepulsors.push_back(CRepulsor(bounds->GetCenterPoint(), 0.75f * diagonal));
        }
      }
    }
  }
}

void CWallCrawlerSwarm::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  x430_posWorkspaces.clear();
  x484_nrmWorkspaces.clear();
  for (int i = 0; i < 9; ++i) {
    float* normals;
    x430_posWorkspaces.push_back(
        x4b0_modelDatas[i]->PickAnimatedModel(which).AllocateNewWorkspace(&normals));
    x484_nrmWorkspaces.push_back(normals);
    x4b0_modelDatas[i]->EnableLooping(true);
    x4b0_modelDatas[i]->AdvanceAnimation(
        x4b0_modelDatas[i]->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")) *
            (float(i) / 16.f),
        mgr, GetCurrentAreaId(), true);
  }
  float* normals;
  x430_posWorkspaces.push_back(
      x4b0_modelDatas[9]->PickAnimatedModel(which).AllocateNewWorkspace(&normals));
  x484_nrmWorkspaces.push_back(normals);
  x4dc_whichModel = which;
}

void CWallCrawlerSwarm::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                        CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Activate:
  case kSM_Deactivate:
    break;
  case kSM_Registered:
    x108_boids.reserve(x548_numBoids);
    for (int i = 0; i < x108_boids.capacity(); ++i) {
      x108_boids.push_back(CBoid(CTransform4f::Identity(), i));
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
    const CVector3f normal = surface.GetNormal() * x374_boidRadius;
    const CVector3f translation = projected + normal;
    x108_boids[index].x0_transform = CTransform4f::Translate(translation);
    if (close_enough(CVector3f::Dot(CVector3f(0.f, 0.f, 1.f), surface.GetNormal()), -1.f)) {
      x108_boids[index].x0_transform.SetRotation(
          CTransform4f::FromColumns(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, -1.f, 0.f),
                                    CVector3f(0.f, 0.f, -1.f), CVector3f::Zero()));
    } else {
      x108_boids[index].x0_transform.SetRotation(
          LookAt(CVector3f(0.f, 0.f, 1.f), surface.GetNormal(), CRelAngle(M_PIF)));
    }
    x108_boids[index].x80_24_active = true;
    x108_boids[index].x30_velocity = CVector3f::Zero();
    x108_boids[index].x3c_targetWaypoint = waypointId;
    x108_boids[index].x7c_24_framesNotOnSurface = 0;
    x108_boids[index].x48_timeToDie = 0.f;
    x108_boids[index].x80_27_scarabExplodeTimerEnabled = false;
    x108_boids[index].x78_health = x3bc_healthInfo.GetHP();
  }
}

void CWallCrawlerSwarm::CalculateRenderBounds() {
  const CAABox bounds = GetBoundingBox();
  SetRenderBounds(bounds);
}

CAABox CWallCrawlerSwarm::GetBoundingBox() const {
  const CVector3f extent(0.75f * x118_boundingBoxExtent.GetX(),
                         0.75f * x118_boundingBoxExtent.GetY(),
                         0.75f * x118_boundingBoxExtent.GetZ());
  const CAABox bounds(-extent, extent);
  return bounds.GetTransformedAABox(GetTransform());
}

rstl::optional_object< CAABox > CWallCrawlerSwarm::GetTouchBounds() const { return xe8_aabox; }

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
    if (it->x0_state == state && it->x4_msg == kSM_Follow) {
      return mgr.GetIdForScript(it->x8_objId);
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
  x168_partitionedBoidLists.clear();
  for (int i = 0; i < 125; ++i) {
    x168_partitionedBoidLists.push_back(nullptr);
  }
  x360_outlierBoidList = nullptr;
  const CAABox bounds = GetBoundingBox();
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const CVector3f size(extent.GetX() / 5.f, extent.GetY() / 5.f, extent.GetZ() / 5.f);
  for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
    if (it->GetActive()) {
      const CVector3f& pos = it->GetTranslation();
      const CVector3f delta = pos - bounds.GetMinPoint();
      const int x = CCast::ToInt32(delta.GetX() / size.GetX());
      const int y = CCast::ToInt32(delta.GetY() / size.GetY());
      const int z = CCast::ToInt32(delta.GetZ() / size.GetZ());
      const int index = x + 5 * y + 25 * z;
      if (index < 0 || index >= 125 || x < 0 || x >= 5 || y < 0 || y >= 5 || z < 0 || z >= 5) {
        it->x44_next = x360_outlierBoidList;
        x360_outlierBoidList = &*it;
      } else {
        it->x44_next = x168_partitionedBoidLists[index];
        x168_partitionedBoidLists[index] = &*it;
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
        if (x168_partitionedBoidLists[rowIndex + z * 25] != nullptr) {
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
    return x360_outlierBoidList;
  }
  return x168_partitionedBoidLists[index];
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
    other = other->x44_next;
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
    ApplySeparation(boid, closest, x13c_separationRadius, x148_separationMagnitude, ahead);
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
    ApplyCohesion(boid, center, x13c_separationRadius, x140_cohesionMagnitude, ahead);
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
    ahead += angle * (x144_alignmentWeight * direction);
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
  const bool enableLighting = x560_24_enableLighting;
  const bool useSoftwareLight = x560_25_useSoftwareLight;
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
        CBoid* boid = x168_partitionedBoidLists[index];
        if (boid != nullptr) {
          if (enableLighting) {
            const CAABox bounds = BoxForPosition(x, y, z, 0.f);
            if (useSoftwareLight) {
              if ((index & 3) == (x100_thinkCounter & 3)) {
                const CColor color = SoftwareLight(mgr, bounds);
                for (CBoid* it = boid; it != nullptr; it = it->x44_next) {
                  if (it->GetActive()) {
                    it->x40_ambientLighting = CColor::Lerp(it->x40_ambientLighting, color, 0.3f);
                  }
                }
              }
            } else {
              HardwareLight(mgr, bounds);
            }
          }
          for (; boid != nullptr; boid = boid->x44_next) {
            if (boid->x80_25_inFrustum && boid->x80_24_active) {
              RenderBoid(boid, drawMask, thermalHot, flags);
            }
          }
        }
      }
    }
  }
  CBoid* boid = x360_outlierBoidList;
  int index = 0;
  for (; boid != nullptr; boid = boid->x44_next) {
    ++index;
    if (boid->x80_25_inFrustum && boid->x80_24_active) {
      if (enableLighting) {
        const CVector3f pos = boid->GetTranslation();
        const CVector3f extent(x374_boidRadius, x374_boidRadius, x374_boidRadius);
        const CAABox bounds = CAABox(pos - extent, pos + extent);
        if (useSoftwareLight) {
          if ((index & 3) == (x100_thinkCounter & 3)) {
            const CColor color = SoftwareLight(mgr, bounds);
            if (boid->GetActive()) {
              boid->x40_ambientLighting = CColor::Lerp(boid->x40_ambientLighting, color, 0.3f);
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
  uint index = boid->x7c_16_index & 3;
  uint mask = drawMask;
  if (boid->x80_26_launched) {
    index = 8;
  } else if (boid->x48_timeToDie > 0.f) {
    index = 9;
  } else if (boid->x80_27_scarabExplodeTimerEnabled || boid->x80_28_nearPlayer) {
    index += 4;
  }
  CModelData& data = *x4b0_modelDatas[index];
  CAnimData& animData = *data.AnimationData();
  CSkinnedModel& model = data.PickAnimatedModel(x4dc_whichModel);
  if (mask & (1 << index)) {
    mask &= ~(1 << index);
    animData.BuildPose();
    model.Calculate(animData.GetPose(), rstl::optional_object< CVertexMorphEffect >(), nullptr,
                    x430_posWorkspaces[index].get());
  }
  gpRender->SetAmbientColor(boid->x40_ambientLighting);
  gpRender->SetModelMatrix(boid->GetTransform());
  const float* const positions = x430_posWorkspaces[index].get();
  const float* const normals = x484_nrmWorkspaces[index];
  const float timeToDie = boid->x48_timeToDie;
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
  if (x560_26_modelAssetDirty) {
    const CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    if (x4dc_whichModel != which) {
      AllocateSkinnedModels(mgr, which);
    }
  }
  SetTransformDirty(true);
  x368_boidGenCooldownTimer -= dt;
  x36c_crabDamageCooldownTimer -= dt;
  ++x100_thinkCounter;
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    if (x104_occludedTimer > 0.f) {
      x104_occludedTimer -= dt;
    }
    if (x104_occludedTimer <= 0.f) {
      return;
    }
    if (x100_thinkCounter & 2) {
      return;
    }
  } else {
    x104_occludedTimer = 7.f;
  }
  UpdateParticles(dt);
  x42c_lockOnIdx = GetLockOnIndex(mgr);
  SetTargetable(x42c_lockOnIdx != -1);
  if (x42c_lockOnIdx == -1) {
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  } else {
    AddMaterial(kMT_Target, kMT_Orbit, mgr);
  }
  while ((x54c_maxCreatedBoids == 0 || x550_createdBoids < x54c_maxCreatedBoids) &&
         x368_boidGenCooldownTimer <= 0.f) {
    bool created = false;
    for (int i = 0; i < x108_boids.size(); ++i) {
      if (!x108_boids[i].GetActive()) {
        CreateBoid(mgr, i);
        ++x550_createdBoids;
        x368_boidGenCooldownTimer += 1.f / x364_boidGenRate;
        created = true;
        break;
      }
    }
    if (!created) {
      x368_boidGenCooldownTimer += 1.f / x364_boidGenRate;
      break;
    }
  }
  UpdatePartition();
  const CAABox bounds = GetBoundingBox();
  int count = 0;
  xe8_aabox = GetBoundingBox();
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        CBoid* boid = x168_partitionedBoidLists[rowIndex + z * 25];
        if (boid != nullptr) {
          CAreaCollisionCache cache(BoxForPosition(x, y, z, 0.5f + x374_boidRadius));
          CGameCollision::BuildAreaCollisionCache(mgr, cache);
          for (; boid != nullptr; boid = boid->x44_next) {
            ++count;
            if (boid->GetActive()) {
              if (x558_flavor == kF_Scarab) {
                const CVector3f extent(x37c_scarabBoxMargin, x37c_scarabBoxMargin,
                                       x37c_scarabBoxMargin);
                xe8_aabox.AccumulateBounds(boid->GetTranslation() + extent);
                xe8_aabox.AccumulateBounds(boid->GetTranslation() - extent);
              } else {
                xe8_aabox.AccumulateBounds(boid->GetTranslation());
              }
            }
            if (((x100_thinkCounter & 1) == (count & 1) && boid->x80_24_active &&
                 boid->x48_timeToDie < 0.1f) ||
                boid->x80_26_launched) {
              UpdateBoid(cache, mgr, dt, *boid);
            }
          }
        }
      }
    }
  }
  for (CBoid* boid = x360_outlierBoidList; boid != nullptr; boid = boid->x44_next) {
    ++count;
    if (boid->GetActive()) {
      xe8_aabox.AccumulateBounds(boid->GetTranslation());
    }
    if (((x100_thinkCounter & 1) == (count & 1) && boid->x80_24_active &&
         boid->x48_timeToDie < 0.1f) ||
        boid->x80_26_launched) {
      const float margin = 1.5f + (0.5f + x374_boidRadius);
      const CVector3f extent(margin, margin, margin);
      const CAABox boidBounds(boid->GetTranslation() - extent, boid->GetTranslation() + extent);
      CAreaCollisionCache cache(boidBounds);
      CGameCollision::BuildAreaCollisionCache(mgr, cache);
      UpdateBoid(cache, mgr, dt, *boid);
    }
  }
  x4b0_modelDatas[8]->AnimationData()->SetPlaybackRate(x160_animPlaybackSpeed);
  x4b0_modelDatas[8]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
  CAdvancementDeltas normalDelta;
  CAdvancementDeltas attractDelta;
  int normalCount = 0;
  int attractCount = 0;
  int index = 0;
  bool normalModels[4] = {false, false, false, false};
  bool attractModels[4] = {false, false, false, false};
  AUTO(it, x108_boids.begin());
  const rstl::vector< CBoid >::const_iterator end = x108_boids.end();
  for (; it != end; ++it, ++index) {
    if (it->x80_24_active && !it->x80_26_launched) {
      if (it->x80_27_scarabExplodeTimerEnabled || it->x80_28_nearPlayer) {
        attractModels[index & 3] = true;
        ++attractCount;
      } else {
        normalModels[index & 3] = true;
        ++normalCount;
      }
    }
  }
  for (int i = 0; i < 4; ++i) {
    x4b0_modelDatas[i]->AnimationData()->SetPlaybackRate(x160_animPlaybackSpeed);
    normalDelta = x4b0_modelDatas[i]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    x4b0_modelDatas[i + 4]->AnimationData()->SetPlaybackRate(x160_animPlaybackSpeed);
    attractDelta = x4b0_modelDatas[i + 4]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    if (x4b0_modelDatas[i]->HasAnimation() && normalModels[i]) {
      UpdateEffects(mgr, *x4b0_modelDatas[i]->AnimationData(),
                    normalCount * 44 / x548_numBoids + 83);
    }
    if (x4b0_modelDatas[i + 4]->HasAnimation() && attractModels[i]) {
      UpdateEffects(mgr, *x4b0_modelDatas[i + 4]->AnimationData(),
                    attractCount * 44 / x548_numBoids + 83);
    }
    for (int j = i; j < x108_boids.size(); j += 4) {
      if (x108_boids[j].x80_24_active) {
        if (x108_boids[j].x80_26_launched) {
          x108_boids[j].x0_transform.AddTranslation(dt * x108_boids[j].x30_velocity);
        } else if (x108_boids[j].x48_timeToDie > 0.f) {
          x108_boids[j].x48_timeToDie -= dt;
          if (x108_boids[j].x48_timeToDie < 0.7f * mgr.Random()->Float()) {
            KillBoid(x108_boids[j], mgr, 1.f, 0.05f);
          }
        } else if (x108_boids[j].x80_27_scarabExplodeTimerEnabled ||
                   x108_boids[j].x80_28_nearPlayer) {
          x108_boids[j].x30_velocity =
              1.5f * x108_boids[j].GetTransform().Rotate(attractDelta.GetOffsetDelta()) / dt;
          x108_boids[j].x0_transform.AddTranslation(dt * x108_boids[j].x30_velocity);
        } else {
          x108_boids[j].x30_velocity =
              1.5f * x108_boids[j].GetTransform().Rotate(normalDelta.GetOffsetDelta()) / dt;
          x108_boids[j].x0_transform.AddTranslation(dt * x108_boids[j].x30_velocity);
        }
      }
    }
  }
  if (x558_flavor == kF_Crab) {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
      if (it->x80_24_active && close_enough(it->x48_timeToDie, 0.f) && !it->x80_26_launched) {
        const CVector3f pos = it->GetTranslation();
        if ((playerPos - pos).Magnitude() < x154_attractionRadius) {
          it->x80_28_nearPlayer = true;
        } else {
          it->x80_28_nearPlayer = false;
        }
      }
    }
  }
  if (x558_flavor == kF_Parasite && x554_maxLaunches > 0) {
    const CVector3f target = mgr.GetPlayer()->GetTranslation() + CVector3f(0.f, 0.f, 1.f);
    static const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    int launched = 0;
    for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
      if (it->x80_24_active && it->x80_26_launched) {
        ++launched;
      }
    }
    if (launched < x554_maxLaunches) {
      for (AUTO(it, x108_boids.begin()); it != x108_boids.end() && launched < x554_maxLaunches;
           ++it) {
        if (it->x80_24_active && close_enough(it->x48_timeToDie, 0.f) && !it->x80_26_launched &&
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
    x4b0_modelDatas[i]->AnimationData()->PreRender();
  }
  for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
    if (it->x80_24_active) {
      it->x80_25_inFrustum =
          frustum.SphereInFrustumPlanes(CSphere(it->GetTranslation(), 2.f * x374_boidRadius));
      active = true;
    } else {
      it->x80_25_inFrustum = false;
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
  if (boid.x80_27_scarabExplodeTimerEnabled) {
    if (x558_flavor == kF_Scarab && boid.x4c_timeToExplode > 0.f) {
      boid.x4c_timeToExplode -= 2.f * dt;
      if (boid.x4c_timeToExplode <= 0.f) {
        ExplodeBoid(boid, mgr);
      }
    }
  } else if (boid.x80_26_launched) {
    const float radius = 2.f * x374_boidRadius;
    const float boidRadius = x374_boidRadius;
    const float speed = boid.x30_velocity.Magnitude();
    float distance = speed * dt;
    CVector3f pos = boid.GetTranslation();
    const CVector3f step = (-boid.x30_velocity / speed) * boidRadius;
    bool found = false;
    while (distance >= 0.f && !found) {
      CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                                CVector3f(0.f, 0.f, 1.f), ~0);
      const CVector3f move = dt * boid.x30_velocity;
      const CVector3f offset = 1.5f * move;
      const CVector3f predicted = pos + offset;
      if (FindBestSurface(cache, predicted, radius, surface) &&
          boid.x7c_6_remainingLaunchNotOnSurfaceFrames == 0) {
        if (x558_flavor != kF_Scarab) {
          boid.x0_transform = LookAt(boid.GetTransform().GetUp(), surface.GetNormal(),
                                     CRelAngle::FromRadians(M_PIF))
                                  .MultiplyIgnoreTranslation(boid.GetTransform());
        }
        const CPlane plane = surface.GetPlane();
        const CVector3f correction =
            -(plane.GetHeight(boid.GetTranslation()) - boidRadius - 0.01f) * plane.GetNormal();
        boid.x0_transform.AddTranslation(correction);
        boid.x7c_24_framesNotOnSurface = 0;
        boid.x80_26_launched = false;
        if (x558_flavor == kF_Scarab) {
          boid.x80_27_scarabExplodeTimerEnabled = true;
          boid.x4c_timeToExplode = x15c_scarabTimeToExplode;
          CSfxManager::AddEmitter(x55e_scatterSfx, boid.GetTranslation(), CVector3f::Zero(), true,
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
      if (x558_flavor == kF_Scarab)
        acceleration = 3.f * gravity;
      boid.x30_velocity += dt * CVector3f(0.f, 0.f, -acceleration);
      if (boid.x7c_6_remainingLaunchNotOnSurfaceFrames != 0) {
        boid.x7c_6_remainingLaunchNotOnSurfaceFrames--;
      }
    }
  } else if (boid.x7c_24_framesNotOnSurface >= 30) {
    boid.x80_24_active = false;
  } else {
    const float radius = 2.f * x374_boidRadius;
    const float boidRadius = x374_boidRadius;
    const CVector3f pos = boid.GetTranslation();
    bool found = false;
    CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                              CVector3f(0.f, 0.f, 1.f), ~0);
    const CVector3f move = dt * boid.x30_velocity;
    const CVector3f offset = 1.5f * move;
    const CVector3f predicted = pos + offset;
    if (FindBestSurface(cache, predicted, radius, surface)) {
      boid.x50_surface = surface;
      boid.x0_transform = LookAt(boid.GetTransform().GetUp(), surface.GetNormal(),
                                 CRelAngle::FromDegrees(180.f * dt))
                              .MultiplyIgnoreTranslation(boid.GetTransform());
      const CPlane plane = surface.GetPlane();
      const float distance = plane.GetHeight(boid.GetTranslation());
      if (distance <= 1.5f * x374_boidRadius) {
        const CVector3f correction = -(distance - boidRadius - 0.01f) * plane.GetNormal();
        boid.x0_transform.AddTranslation(correction);
        boid.x7c_24_framesNotOnSurface = 0;
        found = true;
      }
    }
    if (!found) {
      const float angularSpeed = boid.x30_velocity.Magnitude() / boidRadius;
      boid.x0_transform = LookAt(boid.GetTransform().GetUp(), boid.GetTransform().GetForward(),
                                 CRelAngle(angularSpeed * dt))
                              .MultiplyIgnoreTranslation(boid.GetTransform());
      ++boid.x7c_24_framesNotOnSurface;
    }
    rstl::reserved_vector< CBoid*, 50 > nearList;
    BuildBoidNearList(boid, x13c_separationRadius, nearList);
    CVector3f ahead = 0.3f * boid.GetTransform().GetForward();
    for (int i = 0; i < 8; ++i) {
      switch (i) {
      case 0:
        for (AUTO(it, x4e0_doorRepulsors.begin()); it != x4e0_doorRepulsors.end(); ++it) {
          if ((it->x0_center - boid.GetTranslation()).MagSquared() <
              it->xc_magnitude * it->xc_magnitude) {
            ApplySeparation(boid, it->x0_center, it->xc_magnitude, 4.5f, ahead);
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
        ApplyAttraction(boid, mgr.GetPlayer()->GetTranslation(), x154_attractionRadius,
                        x150_attractionMagnitude, ahead);
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
    boid.x0_transform = LookAt(forward, direction, CRelAngle::FromRadians(M_PIF * dt))
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
  boid.x0_transform.SetRotation(CTransform4f::Identity());
  boid.x0_transform =
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
  boid.x30_velocity = velocity;
  boid.x80_26_launched = true;
  boid.x7c_6_remainingLaunchNotOnSurfaceFrames = 1;
  CSfxManager::AddEmitter(x55c_launchSfx, pos, CVector3f::Zero(), true, false,
                          CSfxManager::kMedPriority, GetCurrentAreaId().Value());
}

void CWallCrawlerSwarm::ScatterScarabBoid(CBoid& boid, CStateManager& mgr) {
  const CVector3f oldDirection = boid.GetTransform().GetForward();
  boid.x0_transform.SetRotation(CTransform4f::Identity());
  boid.x0_transform = LookAt(boid.GetTransform().GetForward(), oldDirection, CRelAngle(M_PIF))
                          .MultiplyIgnoreTranslation(boid.GetTransform());
  boid.x30_velocity = CVector3f::Zero();
  const float angle = mgr.Random()->Float() * (2.f * M_PIF);
  const float speed = mgr.Random()->Float() * x158_scarabScatterXYVelocity;
  boid.x30_velocity.SetX(speed * CMath::FastCosR(angle));
  boid.x30_velocity.SetY(speed * CMath::FastSinR(angle));
  boid.x80_26_launched = true;
  boid.x7c_6_remainingLaunchNotOnSurfaceFrames = 5;
  CSfxManager::AddEmitter(x55c_launchSfx, boid.GetTranslation(), CVector3f::Zero(), true, false,
                          CSfxManager::kMedPriority, GetCurrentAreaId().Value());
}

void CWallCrawlerSwarm::MoveToWayPoint(CBoid& boid, CStateManager& mgr, CVector3f& ahead) {
  if (CScriptWaypoint* waypoint =
          TCastToPtr< CScriptWaypoint >(mgr.ObjectById(boid.x3c_targetWaypoint))) {
    if ((waypoint->GetTranslation() - boid.GetTranslation()).MagSquared() <
        x164_waypointGoalRadius * x164_waypointGoalRadius) {
      boid.x3c_targetWaypoint = waypoint->NextWaypoint(mgr);
      if (boid.x3c_targetWaypoint == kInvalidUniqueId) {
        if (x558_flavor == kF_Scarab) {
          ScatterScarabBoid(boid, mgr);
        } else {
          boid.x80_24_active = false;
          return;
        }
      } else {
        waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(boid.x3c_targetWaypoint));
      }
    }
    ahead += (waypoint->GetTranslation() - boid.GetTranslation()).AsNormalized() *
             x14c_moveToWaypointWeight;
  }
}

void CWallCrawlerSwarm::Touch(CActor& actor, CStateManager& mgr) {
  CActor::Touch(actor, mgr);
  if (const CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
    const CDamageInfo& damage = projectile->GetCurrentDamageInfo();
    if (x3c4_damageVulnerability.WeaponHurts(damage.GetWeaponMode(),
                                             CDamageVulnerability::kRD_No)) {
      const rstl::optional_object< CAABox > touchBounds = projectile->GetTouchBounds();
      if (touchBounds) {
        const CAABox projectileBounds = *touchBounds;
        const float radius = 0.1f + x378_touchRadius;
        const float radiusSq = radius * radius;
        const CVector3f extent(radiusSq, radiusSq, radiusSq);
        for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
          if (it->GetActive()) {
            const CVector3f pos = it->GetTranslation();
            const CAABox bounds(pos - extent, pos + extent);
            if (bounds.DoBoundsOverlap(projectileBounds)) {
              it->x78_health -= damage.GetDamage(x3c4_damageVulnerability);
              if (it->x78_health <= 0.f) {
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
        close_enough(x380_playerTouchRadius, 0.f) ? x378_touchRadius : x380_playerTouchRadius;
    const CAABox playerBounds = *player->GetTouchBounds();
    const CVector3f scarabExtent(x37c_scarabBoxMargin, x37c_scarabBoxMargin, x37c_scarabBoxMargin);
    const CVector3f extent(radius, radius, radius);
    for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
      if (it->x80_24_active && it->x48_timeToDie <= 0.f) {
        if (x558_flavor == kF_Scarab && it->x80_27_scarabExplodeTimerEnabled) {
          const CAABox bounds =
              CAABox(it->GetTranslation() - scarabExtent, it->GetTranslation() + scarabExtent);
          if (playerBounds.DoBoundsOverlap(bounds)) {
            ExplodeBoid(*it, mgr);
            SetExplodeTimers(it->GetTranslation(), 0.5f, 0.5f, 2.5f);
          }
        }
        const CAABox bounds = CAABox(it->GetTranslation() - extent, it->GetTranslation() + extent);
        if (playerBounds.DoBoundsOverlap(bounds)) {
          if (it->x80_26_launched && x558_flavor == kF_Parasite) {
            mgr.ApplyDamage(
                GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
                CDamageInfo(CWeaponMode(kWT_AI), 0.00002f, 0.f, 0.f),
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                CVector3f::Zero());
            KillBoid(*it, mgr, 0.f, 1.f);
          } else if (x558_flavor == kF_Scarab) {
            ExplodeBoid(*it, mgr);
          } else if (x36c_crabDamageCooldownTimer <= 0.f) {
            mgr.ApplyDamage(
                GetUniqueId(), player->GetUniqueId(), GetUniqueId(), x384_crabDamage,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                CVector3f::Zero());
            x36c_crabDamageCooldownTimer = x370_crabDamageCooldown;
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
  if (x42c_lockOnIdx != -1) {
    const CBoid& boid = x108_boids[x42c_lockOnIdx];
    if (boid.GetActive()) {
      const CVector3f delta = boid.GetTranslation() - cameraPos;
      const float distance = delta.Magnitude();
      const CVector3f dir = delta / distance;
      if (CVector3f::Dot(cameraForward, dir) > 0.9238795f) {
        const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
        if (mgr.RayStaticIntersection(cameraPos, dir, distance, filter).IsInvalid()) {
          return x42c_lockOnIdx;
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
  for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it, ++index) {
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
  for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
    if (it->GetActive()) {
      const CVector3f boidPos = it->GetTranslation();
      if ((boidPos - pos).MagSquared() < radiusSquared) {
        it->x78_health -= info.GetRadiusDamage(x3c4_damageVulnerability);
        if (it->x78_health <= 0.f) {
          KillBoid(*it, mgr, 1.f, 0.1f);
        }
      }
    }
  }
}

void CWallCrawlerSwarm::SetExplodeTimers(const CVector3f& pos, float radius, float minTime,
                                         float maxTime) {
  const float radiusSquared = radius * radius;
  for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
    if (it->GetActive() && it->x48_timeToDie <= 0.f) {
      const float distanceSquared = (it->GetTranslation() - pos).MagSquared();
      if (distanceSquared < radiusSquared) {
        const float time = (distanceSquared / radiusSquared) * (maxTime - minTime) + minTime;
        if (it->x4c_timeToExplode > time || it->x4c_timeToExplode == 0.f) {
          it->x4c_timeToExplode = time;
        }
      }
    }
  }
}

void CWallCrawlerSwarm::ExplodeBoid(CBoid& boid, CStateManager& mgr) {
  KillBoid(boid, mgr, 0.f, 1.f);
  mgr.ApplyDamageToWorld(GetUniqueId(), *this, boid.GetTranslation(), x3a0_scarabExplodeDamage,
                         CMaterialFilter::MakeInclude(CMaterialList(kMT_Player)));
}

CVector3f CWallCrawlerSwarm::GetAimPosition(const CStateManager&, float dt) const {
  if (x42c_lockOnIdx == -1) {
    return x124_lastOrbitPosition;
  }
  return x124_lastOrbitPosition + dt * x108_boids[x42c_lockOnIdx].x30_velocity;
}

CVector3f CWallCrawlerSwarm::GetOrbitPosition(const CStateManager&) const {
  if (x42c_lockOnIdx == -1) {
    return x124_lastOrbitPosition;
  }
  x124_lastOrbitPosition = x108_boids[x42c_lockOnIdx].GetTranslation();
  return x124_lastOrbitPosition;
}

void CWallCrawlerSwarm::KillBoid(CBoid& boid, CStateManager& mgr, float deathRattleChance,
                                 float deadChance) {
  x130_lastKilledOffset = boid.GetTranslation();
  AddParticle(boid.GetTransform());
  boid.x80_24_active = false;
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
  for (int i = 0; i < x524_particleGens.size(); ++i) {
    x524_particleGens[i]->SetParticleEmission(true);
    x524_particleGens[i]->SetTranslation(xf.GetTranslation());
    x524_particleGens[i]->ForceParticleCreation(particleCounts[i]);
    x524_particleGens[i]->SetParticleEmission(false);
  }
}

void CWallCrawlerSwarm::UpdateParticles(float dt) {
  for (int i = 0; i < x524_particleGens.size(); ++i) {
    x524_particleGens[i]->Update(dt);
  }
}

void CWallCrawlerSwarm::RenderParticles() const {
  for (int i = 0; i < x524_particleGens.size(); ++i) {
    gpRender->AddParticleGen(*x524_particleGens[i]);
  }
}

void CWallCrawlerSwarm::FreezeCollision(const CMarkerGrid& grid, float duration) {
  const float radius = x378_touchRadius * x378_touchRadius;
  const float xy = radius + 0.3f;
  const float z = radius + 0.5f;
  for (AUTO(it, x108_boids.begin()); it != x108_boids.end(); ++it) {
    if (it->GetActive()) {
      const CVector3f extent(xy, xy, z);
      const CAABox bounds = CAABox(it->GetTranslation() - extent, it->GetTranslation() + extent);
      if (grid.AABoxTouchesData(bounds, 1)) {
        it->x48_timeToDie = 1.f;
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
      params.x0_pos = pos;
      params.xc_dir = CVector3f::Zero();
      params.x24_sfxId = sfx;
      CSfxManager::AddEmitter(params, true, CSfxManager::kMedPriority, false, area);
    }
  }
}
