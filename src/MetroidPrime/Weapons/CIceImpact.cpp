#define CSTATEMANAGER_OUT_OF_LINE_GETPLAYER
#include "MetroidPrime/Weapons/CIceImpact.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Math/CTri.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "WorldFormat/COBBTree.hpp"
#include "rstl/math.hpp"

#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

CMarkerGrid::CMarkerGrid(const CAABox& bounds)
: mBounds(bounds)
, mGridUnits((mBounds.GetMaxPoint() - mBounds.GetMinPoint()) * 0.0625f)
, mGridState(0) {}

uint CMarkerGrid::GetValue(const uint x, const uint y, const uint z) const {
  const uint bitOffset = (x & 3) << 1;
  const uint gridOffset = (y << 2) + (z << 6) + (x >> 2);
  return static_cast< uint >((3 << bitOffset) & mGridState[gridOffset]) >> bitOffset;
}

bool CMarkerGrid::GetCoords(const CVector3f& point, uint& x, uint& y, uint& z) const {
  if (!GetBounds().PointInside(point)) {
    return false;
  }
  const CVector3f& p = point - mBounds.GetMinPoint();
  x = p.GetX() / mGridUnits.GetX();
  y = p.GetY() / mGridUnits.GetY();
  z = p.GetZ() / mGridUnits.GetZ();

  return true;
}

void CMarkerGrid::SetValue(const uint x, const uint y, const uint z, const uint val) {
  const uint bitOffset = (x & 3) << 1;
  const uint gridOffset = (y << 2) + (z << 6) + (x >> 2);
  mGridState[gridOffset] = (mGridState[gridOffset] & ~(3 << bitOffset)) | (val << bitOffset);
}

bool CMarkerGrid::AABoxTouchesData(const CAABox& box, const uint val) const {
  if (!GetBounds().DoBoundsOverlap(box)) {
    return false;
  }

  CAABox inBox = box;
  if (!inBox.Inside(mBounds)) {
    CVector3f min = mBounds.GetMinPoint();
    min.SetX(rstl::max_val(min[0], box.GetMinPoint()[kDX]));
    min.SetY(rstl::max_val(min[1], box.GetMinPoint()[kDY]));
    min.SetZ(rstl::max_val(min[2], box.GetMinPoint()[kDZ]));
    CVector3f max = mBounds.GetMaxPoint();
    max.SetX(rstl::min_val(max[0], box.GetMaxPoint()[kDX]));
    max.SetY(rstl::min_val(max[1], box.GetMaxPoint()[kDY]));
    max.SetZ(rstl::min_val(max[2], box.GetMaxPoint()[kDZ]));
    inBox = CAABox(min, max);
  }

  uint c1x, c1y, c1z;
  GetCoords(inBox.GetMinPoint(), c1x, c1y, c1z);
  uint c2x, c2y, c2z;
  GetCoords(inBox.GetMaxPoint(), c2x, c2y, c2z);

  for (uint i = c1z; i < c2z; ++i) {
    for (uint j = c1y; j < c2y; ++j) {
      for (uint k = c1x; k < c2x; ++k) {
        if ((val & GetValue(k, j, i)) != 0) {
          return true;
        }
      }
    }
  }

  return false;
}

void CMarkerGrid::MarkCells(const CSphere& sphere, const uint val) {
  int width = static_cast< int >((sphere.GetRadius() - mGridUnits[0]) / mGridUnits[0]);
  int length = static_cast< int >((sphere.GetRadius() - mGridUnits[1]) / mGridUnits[1]);
  int height = static_cast< int >((sphere.GetRadius() - mGridUnits[2]) / mGridUnits[2]);
  uint x, y, z;
  if (!GetCoords(sphere.GetCenter(), x, y, z)) {
    return;
  }
  for (uint i = z - width; i < z + width; ++i) {
    for (uint j = y - length; j < y + length; ++j) {
      for (uint k = x - height; k < x + height; ++k) {
        SetValue(k, j, i, val | GetValue(x, y, z));
      }
    }
  }
}

CVector3f CMarkerGrid::GetWorldPositionForCell(const uint x, const uint y, const uint z) const {
  return CVector3f(x * mGridUnits[0], y * mGridUnits[1], z * mGridUnits[2]) +
         mBounds.GetMinPoint() + 0.5f * mGridUnits;
}

CIceImpact::CIceImpact(const TLockedToken< CGenDescription >& particle, TUniqueId uid, TAreaId aid,
                       bool active, const rstl::string& name, const CTransform4f& xf, uint flags,
                       const CVector3f& scale, const CColor& color)
: CEffect(uid, CEntityInfo(aid, CEntity::NullConnectionList), active, name, xf)
, mElementGen(rs_new CElementGen(TToken< CGenDescription >(particle), CElementGen::kMOT_One,
                                    CElementGen::kOSF_One))
, mLightId(kInvalidUniqueId)
, mGenAssetId(TToken< CGenDescription >(particle).GetTag().GetId())
, mLifeTimer(0.f)
, mLatestDamageTime(4.f)
, mSearchDirection(0)
, mHalfBounds(8.f)
, mParticleRemainder(0.f)
, mSphereGenRange(GetTranslation(), mHalfBounds - 1.6f)
, mGrid(
      CAABox(xf.GetTranslation() - CVector3f(mHalfBounds, mHalfBounds, mHalfBounds),
             xf.GetTranslation() + CVector3f(mHalfBounds, mHalfBounds, mHalfBounds)))
, mFollowPlayerArea(flags & 2)
, mHasRenderBounds(false) {
  SetThermalFlags(kTF_Hot);
  mImpactSpheres.push_back(SImpactSphere(GetTranslation(), 2.4f, 1.6f, 0.f, 0.f));
  for (int i = 1; i < 3; ++i) {
    mImpactSpheres.push_back(SImpactSphere(GetTranslation(), 0.f, 1.f, 0.f, 0.f));
    mImpactSpheres[i].mPreviousRadius = mImpactSpheres[i].mRadius;
    mImpactSpheres[i].mRadius += mImpactSpheres[i].mRadiusStep;
  }
  mGrid.MarkCells(CSphere(GetTranslation(), 2.4f), 2);
}

CIceImpact::~CIceImpact() {}

void CIceImpact::CalculateRenderBounds() {
  const rstl::optional_object< CAABox > bounds = mElementGen->GetBounds();
  if (bounds) {
    mHasRenderBounds = true;
    SetRenderBounds(*bounds);
  } else {
    const CVector3f pos = GetTranslation();
    mHasRenderBounds = false;
    SetRenderBounds(CAABox(pos, pos));
  }
}

void CIceImpact::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);
  SetPreRenderClipped(!mHasRenderBounds ||
                      !frustum.BoxInFrustumPlanes(GetRenderBoundsCached()));
}

void CIceImpact::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (GetPreRenderClipped()) {
    return;
  }
  if (mgr.GetThermalDrawFlag() == kTD_Hot) {
    EnsureRendered(mgr);
  } else {
    gpRender->AddParticleGen(*mElementGen);
  }
}

void CIceImpact::Render(const CStateManager& mgr) const {
  CElementGen::SetSubtractBlend(true);
  CCubeModel::SetRenderModelBlack(true);
  mElementGen->Render();
  CElementGen::SetSubtractBlend(false);
  CCubeModel::SetRenderModelBlack(false);
}

void CIceImpact::Think(float dt, CStateManager& mgr) {
  mLifeTimer += dt;
  if (mLifeTimer < 0.8f && mElementGen->GetParticleCount() < 400) {
    for (int i = 0; i < mImpactSpheres.size(); ++i) {
      SImpactSphere& sphere = mImpactSpheres[i];
      if (sphere.mRadius > sphere.mMaxRadius) {
        const rstl::optional_object< SImpactSphere > next = GenerateNewSphere();
        if (next) {
          sphere = *next;
        }
      }
    }
    for (int i = 0; i < mImpactSpheres.size(); ++i) {
      SImpactSphere& sphere = mImpactSpheres[i];
      if (sphere.mRadius > sphere.mMaxRadius) {
        continue;
      }
      sphere.mPreviousRadius = sphere.mRadius;
      sphere.mRadius += sphere.mRadiusStep;
      const CSphere a = CSphere(sphere.mPos, sphere.mRadius);
      const CSphere b = CSphere(sphere.mPos, sphere.mPreviousRadius);
      const CAABox bounds(b.GetCenter() - CVector3f(a.GetRadius(), a.GetRadius(), a.GetRadius()),
                          b.GetCenter() + CVector3f(a.GetRadius(), a.GetRadius(), a.GetRadius()));
      mParticleRemainder = 0.f;
      GenerateParticlesAgainstActors(mgr, bounds, a, b);
      CAreaCollisionCache cache(bounds);
      CGameCollision::BuildAreaCollisionCache(mgr, cache);
      for (int j = 0; j < static_cast< int >(cache.GetNumCaches()); ++j) {
        GenerateParticlesAgainstWorld(mgr, cache.GetOctreeLeafCache(j), a, b);
      }
    }
  }
  mElementGen->SetOrientation(CTransform4f::Identity());
  mElementGen->Update(dt);
  if (mLightId != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
      if (GetActive()) {
        light->SetLight(mElementGen->GetLight());
      }
    }
  }
  if (mFollowPlayerArea) {
    mgr.SetActorAreaId(*this, mgr.GetPlayer()->GetCurrentAreaId());
  }
  if (mElementGen->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

ENTITY_ACCEPT_IMPL(CIceImpact)

void CIceImpact::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    if (mElementGen->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      uint sourceId = mGenAssetId;
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(),
                                      rstl::string_l("IcePLight_") + GetDebugName(), GetTransform(),
                                      GetUniqueId(), mElementGen->GetLight(), sourceId, 1, 0.f));
    }
    break;
  case kSM_Deleted:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
      mLightId = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
  if (mLightId != kInvalidUniqueId) {
    mgr.SendScriptMsgAlways(mLightId, uid, msg);
  }
}

rstl::optional_object< CAABox > CIceImpact::GetTouchBounds() const { return mGrid.GetBounds(); }

void CIceImpact::Touch(CActor& actor, CStateManager& mgr) {
  if (mLifeTimer > mLatestDamageTime) {
    return;
  }
  const rstl::optional_object< CAABox > bounds = actor.GetTouchBounds();
  if (bounds) {
    const CDamageInfo damage(CWeaponMode(kWT_Ice, false, true), 100.f, 0.f, 1.f);
    if (CPatterned* ai = TCastToPtr< CPatterned >(&actor)) {
      const CAABox expandedBounds(bounds->GetMinPoint() - CVector3f(0.f, 0.f, 0.5f),
                                  bounds->GetMaxPoint() + CVector3f(0.f, 0.f, 0.5f));
      if (mGrid.AABoxTouchesData(expandedBounds, 1)) {
        if (static_cast< const CActor* >(ai)->GetDamageVulnerability()->WeaponHits(
                CWeaponMode(kWT_Ice), CDamageVulnerability::kRD_No) &&
            ai->GetKnockBackCtrl().GetEnableFreeze() &&
            ai->BodyCtrl()->GetPercentageFrozen() == 0.f &&
            mLatestDamageTime - mLifeTimer > 0.5f && mLifeTimer < 0.8f) {
          mgr.ApplyDamage(
              GetUniqueId(), actor.GetUniqueId(), kInvalidUniqueId, damage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
        }
      }
    }
    if (actor.GetMaterialList().HasMaterial(kMT_ExcludeFromLineOfSightTest) &&
        mGrid.AABoxTouchesData(*bounds, 1)) {
      mgr.ApplyDamage(
          GetUniqueId(), actor.GetUniqueId(), kInvalidUniqueId, damage,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
    if (CWallCrawlerSwarm* swarm = TCastToPtr< CWallCrawlerSwarm >(&actor)) {
      if (mLatestDamageTime - mLifeTimer > 0.5f) {
        swarm->FreezeCollision(mGrid, mLatestDamageTime - mLifeTimer);
      }
    }
  }
}

bool pointInSphere(const CSphere& sphere, const CVector3f& point) {
  const float r = sphere.GetRadius() * sphere.GetRadius();
  const CVector3f& d = sphere.GetCenter() - point;
  return d.MagSquared() <= r;
}

rstl::optional_object< CIceImpact::SImpactSphere > CIceImpact::GenerateNewSphere() {
  ++mSearchDirection;
  mSearchDirection &= 7;
  const uint forwardZ = mSearchDirection & 1;
  const uint forwardY = mSearchDirection & 2;
  const uint forwardX = mSearchDirection & 4;
  for (uint z = 8; forwardZ ? z < 14 : z >= 1; forwardZ ? ++z : --z) {
    for (uint y = 8; forwardY ? y < 14 : y >= 1; forwardY ? ++y : --y) {
      for (uint x = 8; forwardX ? x < 14 : x >= 1; forwardX ? ++x : --x) {
        if (static_cast< int >(mGrid.GetValue(x, y, z)) == 1) {
          const CVector3f pos = mGrid.GetWorldPositionForCell(x, y, z);
          mGrid.SetValue(x, y, z, 3);
          if (pointInSphere(mSphereGenRange, pos)) {
            mGrid.MarkCells(CSphere(pos, 1.6f), 2);
            return SImpactSphere(pos, 1.6f, 1.6f, 0.f, 0.f);
          }
        }
      }
    }
  }
  return rstl::optional_object_null();
}

bool CIceImpact::GenerateParticlesAgainstWorld(CStateManager& mgr,
                                               const CMetroidAreaCollider::COctreeLeafCache& cache,
                                               const CSphere& a, const CSphere& b) {
  CMetroidAreaCollider::ResetInternalCounters();
  const CMaterialFilter filter =
      CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough));
  for (int i = 0; i < cache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = cache.GetLeaf(i);
    const CAreaOctTree::TriListReference triangles = node.GetTriangleArray();
    const CAreaOctTree& owner = node.GetOwner();
    int numTriangles = triangles.GetSize();
    bool done = false;
    for (int j = 0; j < numTriangles && !done; ++j) {
      ushort idx = triangles.GetAt(j);
      if (CMetroidAreaCollider::DupTriangleListValue(idx) !=
          CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
        CMetroidAreaCollider::DupTriangleListValue(idx) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        const CCollisionSurface surface = owner.GetMasterListTriangle(idx);
        if (filter.Passes(CMaterialList(surface.GetSurfaceFlags()))) {
          const CVector3f v1 = surface.GetPoint(0);
          const CVector3f v2 = surface.GetPoint(1);
          const CVector3f v3 = surface.GetPoint(2);
          done = SubdivideAndGenerateParticles(mgr, v1, v2, v3, a, b);
        }
      }
    }
  }
  return false;
}

bool CIceImpact::GenerateParticlesAgainstActors(CStateManager& mgr, const CAABox& bounds,
                                                const CSphere& a, const CSphere& b) {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(
      nearList, bounds,
      CMaterialFilter::MakeExclude(CMaterialList(kMT_Character, kMT_Player, kMT_Projectile,
                                                 kMT_ProjectilePassthrough, kMT_AIJoint)),
      this);
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it));
    CPhysicsActor* const physics = TCastToPtr< CPhysicsActor >(actor);
    if (physics && physics->GetCollisionPrimitive()->GetPrimType() == 'OBTG') {
      const CCollidableOBBTreeGroup* group =
          static_cast< const CCollidableOBBTreeGroup* >(physics->GetCollisionPrimitive());
      for (int i = 0; i < group->GetContainer()->NumTrees(); ++i) {
        GenerateParticlesAgainstOBBTree(mgr, *group->GetOBBTreeAABox(i),
                                        physics->GetPrimitiveTransform(), a, b);
      }
    } else {
      if (actor &&
          (actor->GetMaterialList().HasMaterial(kMT_Solid) || TCastToPtr< CScriptWater >(actor))) {
        const rstl::optional_object< CAABox > actorBounds = actor->GetTouchBounds();
        if (actorBounds) {
          GenerateParticlesAgainstAABox(mgr, *actorBounds, a, b);
        }
      }
    }
  }
  return true;
}

bool CIceImpact::GenerateParticlesAgainstAABox(CStateManager& mgr, const CAABox& bounds,
                                               const CSphere& a, const CSphere& b) {
  for (int i = 0; i < 12; ++i) {
    const CTri tri = bounds.GetTri(static_cast< CAABox::EBoxFaceId >(i / 2), (i & 1) * 2);
    if (SubdivideAndGenerateParticles(mgr, tri.GetPointA(), tri.GetPointC(), tri.GetPointB(), a,
                                      b)) {
      break;
    }
  }
  return false;
}

bool CIceImpact::GenerateParticlesAgainstOBBTree(CStateManager& mgr, const COBBTree& tree,
                                                 const CTransform4f& xf, const CSphere& a,
                                                 const CSphere& b) {
  const CMaterialFilter filter =
      CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough));
  for (short i = 0; i < tree.NumSurfaceMaterials(); ++i) {
    const CCollisionSurface surface = tree.GetTransformedSurface(i, xf);
    if (filter.Passes(CMaterialList(surface.GetSurfaceFlags()))) {
      if (SubdivideAndGenerateParticles(mgr, surface.GetVert(0), surface.GetVert(1),
                                        surface.GetVert(2), a, b)) {
        break;
      }
    }
  }
  return false;
}

bool CIceImpact::SubdivideAndGenerateParticles(CStateManager& mgr, const CVector3f& v1,
                                               const CVector3f& v2, const CVector3f& v3,
                                               const CSphere& a, const CSphere& b) {
  if (CollisionUtil::TriSphereOverlap(a, v1, v2, v3) == true) {
    if (pointInSphere(b, v1) && pointInSphere(b, v2) && pointInSphere(b, v3)) {
      return false;
    }
    const CVector3f edge1 = v2 - v1;
    const CVector3f edge2 = v3 - v1;
    const CVector3f cross = CVector3f::Cross(edge1, edge2);
    const float mag = CVector3f(cross).Magnitude();
    if (mag > 1.f) {
      const CVector3f point =
          CMath::BaryToWorld(v1, v2, v3, CVector3f(1.f / 3.f, 1.f / 3.f, 1.f / 3.f));
      SubdivideAndGenerateParticles(mgr, v1, v2, point, a, b);
      SubdivideAndGenerateParticles(mgr, v2, v3, point, a, b);
      SubdivideAndGenerateParticles(mgr, v3, v1, point, a, b);
    } else {
      mParticleRemainder += mag;
      const int count = static_cast< int >(mParticleRemainder);
      mParticleRemainder -= count;
      for (int i = 0; i < count; ++i) {
        float rx = mgr.Random()->Float();
        float ry = mgr.Random()->Float();
        float rz = mgr.Random()->Float();
        float inv = 1.f / (rx + ry + rz);
        rx *= inv;
        ry *= inv;
        rz *= inv;
        const CVector3f point = CMath::BaryToWorld(v1, v2, v3, CVector3f(rx, ry, rz));
        uint cx, cy, cz;
        if (!pointInSphere(b, point) && pointInSphere(a, point) &&
            mGrid.GetCoords(point, cx, cy, cz) && (mGrid.GetValue(cx, cy, cz) & 1) == 0) {
          mGrid.SetValue(cx, cy, cz, 1);
          CVector3f direction = CVector3f::Zero();
          switch (mgr.Random()->Range(0, 2)) {
          case 0:
            direction = v1 - point;
            if (!direction.CanBeNormalized()) {
              direction = v2 - point;
            }
            break;
          case 1:
            direction = v2 - point;
            if (!direction.CanBeNormalized()) {
              direction = v3 - point;
            }
            break;
          case 2:
            direction = v3 - point;
            if (!direction.CanBeNormalized()) {
              direction = v1 - point;
            }
            break;
          }
          direction = direction.AsNormalized();
          CVector3f normal = CVector3f(cross).AsNormalized();
          normal[kDX] += 0.4f * (mgr.Random()->Float() - 0.5f);
          normal[kDY] += 0.4f * (mgr.Random()->Float() - 0.5f);
          normal[kDZ] += 0.4f * (mgr.Random()->Float() - 0.5f);
          const CTransform4f xf =
              CTransform4f::LookAt(CVector3f::Zero(), normal.AsNormalized(), direction);
          mElementGen->SetOrientation(xf);
          mElementGen->SetTranslation(point);
          mElementGen->ForceParticleCreation(1);
          if (mElementGen->GetParticleCount() == mElementGen->GetMaxParticles()) {
            return true;
          }
        }
      }
    }
  }
  return false;
}
