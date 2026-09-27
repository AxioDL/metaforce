#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloudModifier.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CTri.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "rstl/algorithm.hpp"

#include <float.h>

CFishCloudModifier::CFishCloudModifier(TUniqueId uid, const bool active, const rstl::string& name,
                                       const CEntityInfo& info, const CVector3f& pos,
                                       bool isRepulsor, bool swirl, float radius, float priority)
: CActor(uid, active, name, info, CTransform4f::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(kMT_NoStepLogic), CActorParameters::None(), kInvalidUniqueId)
, mRadius(radius)
, mPriority(priority)
, mIsRepulsor(isRepulsor)
, mSwirl(swirl) {}

ENTITY_ACCEPT_IMPL(CFishCloudModifier)

void CFishCloudModifier::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                         CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case kSM_Activate:
  case kSM_InitializedInArea:
    if (GetActive()) {
      AddSelf(mgr);
    }
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    RemoveSelf(mgr);
    break;
  }
}

void CFishCloudModifier::AddSelf(CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->mState != kSS_Modify || it->mMsg != kSM_Follow) {
      continue;
    }
    const TUniqueId uid = mgr.GetIdForScript(it->mObjId);
    if (uid == kInvalidUniqueId) {
      continue;
    }
    if (CFishCloud* cloud = TCastToPtr< CFishCloud >(mgr.ObjectById(uid))) {
      if (mIsRepulsor) {
        cloud->AddRepulsor(GetUniqueId(), mSwirl, mRadius, mPriority);
      } else {
        cloud->AddAttractor(GetUniqueId(), mSwirl, mRadius, mPriority);
      }
    }
  }
}

void CFishCloudModifier::RemoveSelf(CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->mState != kSS_Modify || it->mMsg != kSM_Follow) {
      continue;
    }
    const TUniqueId uid = mgr.GetIdForScript(it->mObjId);
    if (uid == kInvalidUniqueId) {
      continue;
    }
    if (CFishCloud* cloud = TCastToPtr< CFishCloud >(mgr.ObjectById(uid))) {
      if (mIsRepulsor) {
        cloud->RemoveRepulsor(GetUniqueId());
      } else {
        cloud->RemoveAttractor(GetUniqueId());
      }
    }
  }
}

CFishCloud::CModifierSource::CModifierSource(const TUniqueId& source, bool repulsor, bool swirl,
                                             float radius, float priority)
: mSource(source)
, mRadius(radius)
, mPriority(priority)
, mIsRepulsor(repulsor)
, mIsSwirl(swirl) {}

bool CFishCloud::CModifierSource::operator<(const CModifierSource& other) const {
  if (mSource == other.mSource) {
    return mIsRepulsor < other.mIsRepulsor;
  }
  return mSource < other.mSource;
}

CFishCloud::CBoid::CBoid(const CVector3f& pos, const CVector3f& vel, float scale)
: mPos(pos), mVel(vel), mScale(scale), mNext(nullptr), mActive(true) {}

CFishCloud::CFishCloud(TUniqueId uid, const bool active, const rstl::string& name,
                       const CEntityInfo& info, const CVector3f& scale, const CTransform4f& xf,
                       const CModelData& mData, const CAnimRes& aRes, int numBoids, float speed,
                       float separationRadius, float cohesionMagnitude, float alignmentWeight,
                       float separationMagnitude, float weaponRepelMagnitude,
                       float playerRepelMagnitude, float containmentMagnitude, float scatterVel,
                       float maxScatterAngle, float weaponRepelDampingSpeed,
                       float playerRepelDampingSpeed, float containmentRadius, int updateShift,
                       const CColor& color, bool killable, float weaponKillRadius, CAssetId part1,
                       int partCount1, CAssetId part2, int partCount2, CAssetId part3,
                       int partCount3, CAssetId part4, int partCount4, int deathSfx,
                       bool repelFromThreats, bool hotInThermal)
: CActor(uid, active, name, info, xf, mData, CMaterialList(kMT_NoStepLogic),
         CActorParameters::None().HotInThermal(hotInThermal), kInvalidUniqueId)
, mUpdateMask((1 << updateShift) - 1)
, mScale(scale)
, mRandomMovementTimer(0.f)
, mSpeed(speed)
, mNumBoids(numBoids)
, mSeparationRadius(separationRadius)
, mCohesionMagnitude(cohesionMagnitude)
, mAlignmentWeight(alignmentWeight)
, mSeparationMagnitude(separationMagnitude)
, mWeaponRepelMagnitude(weaponRepelMagnitude)
, mPlayerRepelMagnitude(playerRepelMagnitude)
, mScatterVel(scatterVel)
, mMaxScatterAngle(maxScatterAngle)
, mContainmentMagnitude(containmentMagnitude)
, mPlayerRepelDampingSpeed(playerRepelDampingSpeed)
, mWeaponRepelDampingSpeed(weaponRepelDampingSpeed)
, mPlayerRepelDamping(playerRepelDampingSpeed)
, mWeaponRepelDamping(weaponRepelDampingSpeed)
, mColor(color)
, mWeaponKillRadius(weaponKillRadius)
, mContainmentRadius(containmentRadius)
, mDeathSfx(
      CSfxManager::TranslateSFXID(deathSfx == -1 ? CSfxManager::kInternalInvalidSfxId : deathSfx))
, mPartitionPitch(CVector3f::Zero())
, mOoPartitionPitch(CVector3f::Zero())
, mRandomMovement(false)
, mWorldSpace(false)
, mEnableWeaponRepelDamping(false)
, mValidModel(false)
, mKillable(killable)
, mRepelFromThreats(repelFromThreats)
, mEnablePlayerRepelDamping(false)
, mUpdateWithoutPartitions(false) {
  mModifierSources.reserve(10);
  const CVector3f& forward = GetTransform().GetForward();
  const CVector3f& up = GetTransform().GetUp();
  const CVector3f& right = GetTransform().GetRight();
  mWorldSpace = !(close_enough(right.GetX(), 1.f) && close_enough(right.GetX(), 0.f) &&
                         close_enough(right.GetX(), 0.f) && close_enough(forward.GetX(), 0.f) &&
                         close_enough(forward.GetX(), 1.f) && close_enough(forward.GetX(), 0.f) &&
                         close_enough(up.GetX(), 0.f) && close_enough(up.GetX(), 0.f) &&
                         close_enough(up.GetX(), 1.f));
  if (aRes.GetId() != kInvalidAssetId) {
    mModels.push_back(rs_new CModelData(aRes));
    mModels.push_back(rs_new CModelData(aRes));
    mModels.push_back(rs_new CModelData(aRes));
    mModels.push_back(rs_new CModelData(aRes));
    mValidModel = true;
  }
  if (part1 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part1)));
  }
  if (part2 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part2)));
  }
  if (part3 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part3)));
  }
  if (part4 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part4)));
  }
  for (int i = 0; i < mParticleDescs.size(); ++i) {
    mParticleGens.push_back(rs_new CElementGen(mParticleDescs[i]));
    mParticleGens[i]->SetParticleEmission(false);
  }
  mDeathParticleCounts.push_back(partCount1);
  mDeathParticleCounts.push_back(partCount2);
  mDeathParticleCounts.push_back(partCount3);
  mDeathParticleCounts.push_back(partCount4);
  const CAABox& aabb = GetBoundingBox();
  mPartitionPitch = (aabb.GetMaxPoint() - aabb.GetMinPoint()) / 7.f;
  mOoPartitionPitch =
      CVector3f(1.f / mPartitionPitch.GetX(), 1.f / mPartitionPitch.GetY(),
                1.f / mPartitionPitch.GetZ());
}

void CFishCloud::InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which) {
  mPosWorkspaces.clear();
  mNrmWorkspaces.clear();
  for (int i = 0; i < 4; ++i) {
    float* normals;
    mPosWorkspaces.push_back(
        mModels[i]->PickAnimatedModel(which).AllocateNewWorkspace(&normals));
    mNrmWorkspaces.push_back(normals);
    mModels[i]->EnableLooping(true);
    mModels[i]->AdvanceAnimation(
        (float(i) / 4.f) *
            mModels[i]->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")),
        mgr, GetCurrentAreaId(), true);
  }
  mWhichModel = which;
}

ENTITY_ACCEPT_IMPL(CFishCloud)

void CFishCloud::CalculateRenderBounds() {
  const CAABox aabb = GetBoundingBox();
  SetRenderBounds(aabb);
}

CAABox CFishCloud::GetBoundingBox() const {
  const CAABox aabb = GetUntransformedBoundingBox();
  return aabb.GetTransformedAABox(GetTransform());
}

CAABox CFishCloud::GetUntransformedBoundingBox() const {
  const CVector3f extent(0.75f * mScale.GetX(), 0.75f * mScale.GetY(),
                         0.75f * mScale.GetZ());
  return CAABox(-extent, extent);
}

bool CFishCloud::PointInBox(const CAABox& aabb, const CVector3f& point) const {
  if (!mWorldSpace) {
    return aabb.PointInside(point);
  }
  const CVector3f localPoint = GetTransform().TransposeRotate(point - GetTranslation());
  return GetUntransformedBoundingBox().PointInside(localPoint);
}

CPlane CFishCloud::FindClosestPlane(const CAABox& aabb, const CVector3f& point) const {
  if (!mWorldSpace) {
    float minDistance = FLT_MAX;
    CAABox::EBoxFaceId minFace = CAABox::kF_YMin;
    for (int i = 0; i < 6; ++i) {
      const CTri tri = aabb.GetTri(static_cast< CAABox::EBoxFaceId >(i), 0);
      const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
      const float distance = plane.GetHeight(point);
      if (distance >= 0.f && distance < minDistance) {
        minFace = static_cast< CAABox::EBoxFaceId >(i);
        minDistance = distance;
      }
    }
    const CTri tri = aabb.GetTri(minFace, 0);
    return CPlane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
  }
  const CTransform4f& xf = GetTransform();
  const CVector3f localPoint = xf.TransposeRotate(point - GetTranslation());
  const CAABox localBounds = GetUntransformedBoundingBox();
  float minDistance = FLT_MAX;
  CAABox::EBoxFaceId minFace = CAABox::kF_YMin;
  for (int i = 0; i < 6; ++i) {
    const CTri tri = localBounds.GetTri(static_cast< CAABox::EBoxFaceId >(i), 0);
    const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
    const float distance = plane.GetHeight(localPoint);
    if (distance >= 0.f && distance < minDistance) {
      minFace = static_cast< CAABox::EBoxFaceId >(i);
      minDistance = distance;
    }
  }
  const CTri tri = localBounds.GetTri(minFace, 0);
  return CPlane(xf * tri.GetPointA(), xf * tri.GetPointC(), xf * tri.GetPointB());
}

void CFishCloud::PlaceBoid(CStateManager& mgr, CBoid& boid, const CAABox& aabb) {
  CRandom16& random = *mgr.Random();
  const CPlane plane = FindClosestPlane(aabb, boid.mPos);
  boid.mPos -= plane.GetHeight(boid.mPos) * plane.GetNormal() + 0.0001f * plane.GetNormal();
  boid.mVel = CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, 0.f);
  if (!mWorldSpace) {
    if (!aabb.PointInside(boid.mPos)) {
      const CVector3f min = aabb.GetMinPoint();
      boid.mPos = CVector3f(random.Float() * aabb.GetWidth() + min.GetX(),
                              random.Float() * aabb.GetHeight() + min.GetY(),
                              random.Float() * aabb.GetDepth() + min.GetZ());
    }
  } else if (!PointInBox(aabb, boid.mPos)) {
    const CAABox localBounds = GetUntransformedBoundingBox();
    const CVector3f min = localBounds.GetMinPoint();
    const CVector3f pos(random.Float() * localBounds.GetWidth() + min.GetX(),
                        random.Float() * localBounds.GetHeight() + min.GetY(),
                        random.Float() * localBounds.GetDepth() + min.GetZ());
    boid.mPos = GetTransform() * pos;
  }
}

void CFishCloud::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case kSM_Activate:
  case kSM_Deactivate:
  case kSM_Deleted:
    break;
  case kSM_Registered: {
    mBoids.reserve(mNumBoids);
    const CAABox bounds = GetUntransformedBoundingBox();
    CRandom16& random = *mgr.Random();
    const CVector3f& min = bounds.GetMinPoint();
    for (int i = 0; i < mBoids.capacity(); ++i) {
      const CVector3f pos(random.Float() * bounds.GetWidth() + min[kDX],
                          random.Float() * bounds.GetHeight() + min[kDY],
                          random.Float() * bounds.GetDepth() + min[kDZ]);
      mBoids.push_back(CBoid(GetTransform() * pos,
                                CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, 0.f),
                                0.2f * CMath::PowF(random.Float(), 7.f) + 0.9f));
    }
    CreatePartitionList();
    if (mValidModel) {
      InitAnimBoids(mgr, CModelData::kWM_Normal);
    }
    break;
  }
  default:
    break;
  }
}

bool CFishCloud::AddAttractor(TUniqueId source, bool swirl, float radius, float priority) {
  const CModifierSource modifier(source, false, swirl, radius, priority);
  AUTO(it, rstl::binary_find(mModifierSources.begin(), mModifierSources.end(), modifier));
  if (it != mModifierSources.end()) {
    it->SetAffectRadius(radius);
    it->SetAffectPriority(priority);
    return true;
  }
  if (mModifierSources.size() < mModifierSources.capacity()) {
    AUTO(insertIt,
         rstl::lower_bound(mModifierSources.begin(), mModifierSources.end(), modifier));
    mModifierSources.insert(insertIt, modifier);
    return true;
  }
  return false;
}

bool CFishCloud::AddRepulsor(TUniqueId source, bool swirl, float radius, float priority) {
  const CModifierSource modifier(source, true, swirl, radius, priority);
  AUTO(it, rstl::binary_find(mModifierSources.begin(), mModifierSources.end(), modifier));
  if (it != mModifierSources.end()) {
    it->SetAffectRadius(radius);
    it->SetAffectPriority(priority);
    return true;
  }
  if (mModifierSources.size() < mModifierSources.capacity()) {
    AUTO(insertIt,
         rstl::lower_bound(mModifierSources.begin(), mModifierSources.end(), modifier));
    mModifierSources.insert(insertIt, modifier);
    return true;
  }
  return false;
}

void CFishCloud::RemoveAttractor(TUniqueId source) {
  const CModifierSource modifier(source, false, false, 0.f, 0.f);
  AUTO(it, rstl::binary_find(mModifierSources.begin(), mModifierSources.end(), modifier));
  if (it != mModifierSources.end()) {
    mModifierSources.erase(it);
  }
}

void CFishCloud::RemoveRepulsor(TUniqueId source) {
  const CModifierSource modifier(source, true, false, 0.f, 0.f);
  AUTO(it, rstl::binary_find(mModifierSources.begin(), mModifierSources.end(), modifier));
  if (it != mModifierSources.end()) {
    mModifierSources.erase(it);
  }
}

rstl::optional_object< CAABox > CFishCloud::GetTouchBounds() const { return GetBoundingBox(); }

void CFishCloud::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    return;
  }
  mWeaponRepelDamping =
      rstl::max_val(0.f, mWeaponRepelDamping - mWeaponRepelDampingSpeed * dt * 0.1f);
  if (mEnableWeaponRepelDamping) {
    mWeaponRepelDamping = rstl::min_val(
        mWeaponRepelMagnitude, mWeaponRepelDampingSpeed * dt + mWeaponRepelDamping);
  }
  mPlayerRepelDamping =
      rstl::max_val(0.f, mPlayerRepelDamping - mPlayerRepelDampingSpeed * dt * 0.1f);
  if (mEnablePlayerRepelDamping) {
    mPlayerRepelDamping = rstl::min_val(
        mPlayerRepelMagnitude, mPlayerRepelDampingSpeed * dt + mPlayerRepelDamping);
  }
  mEnableWeaponRepelDamping = false;
  mEnablePlayerRepelDamping = false;
  ++mThinkCounter;
  UpdateParticles(dt);
  rstl::reserved_vector< CBoid*, 25 > nearList;
  UpdatePartitionList();
  CRandom16& random = *mgr.Random();
  const CAABox bounds = GetBoundingBox();
  int index = 0;
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it, ++index) {
    if (it->mActive && (index & mUpdateMask) == (mThinkCounter & mUpdateMask)) {
      nearList.clear();
      if (mUpdateWithoutPartitions) {
        OldBuildBoidNearList(it->mPos, mSeparationRadius, nearList);
      } else {
        BuildBoidNearList(it->mPos, mSeparationRadius, nearList);
      }
      for (int i = 0; i != 5; ++i) {
        switch (i) {
        case 1:
          ApplySeparation(*it, nearList);
          break;
        case 2:
          if (!mRandomMovement || random.Float() > mRandomMovementTimer) {
            ApplyCohesion(*it, nearList);
          }
          break;
        case 3:
          if (!mRandomMovement || random.Float() > mRandomMovementTimer) {
            ApplyAlignment(*it, nearList);
          }
          break;
        case 4:
          ApplyWander(mgr, *it);
          break;
        }
        if (it->mVel.MagSquared() > 3.2f) {
          break;
        }
      }
      if (!mRandomMovement && it->mVel.MagSquared() < 3.2f) {
        for (AUTO(mod, mModifierSources.begin()); mod != mModifierSources.end(); ++mod) {
          if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mod->GetSource()))) {
            if (mod->IsSwirl()) {
              ApplyRotation(*it, mod->GetAffectPriority(), actor->GetTranslation(),
                            mod->GetAffectRadius(), mod->IsRepulsor());
            } else if (mod->IsRepulsor()) {
              ApplyRepulsion(*it, actor->GetTranslation(), mod->GetAffectRadius(),
                             mod->GetAffectPriority());
            } else {
              ApplyAttraction(*it, actor->GetTranslation(), mod->GetAffectRadius(),
                              mod->GetAffectPriority());
            }
          } else {
            if (mod->IsRepulsor()) {
              RemoveRepulsor(mod->GetSource());
            } else {
              RemoveAttractor(mod->GetSource());
            }
            break;
          }
        }
      }
    }
  }
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    if (it->mActive) {
      ApplyContainment(*it, bounds);
      CVector3f& velocity = it->mVel;
      const float speed = velocity.Magnitude();
      if (!close_enough(speed, 0.f)) {
        const float inverseSpeed = 1.f / speed;
        velocity *= inverseSpeed;
      }
      velocity.SetZ(0.99f * velocity.GetZ());
    }
  }
  if (mRandomMovementTimer > 0.f) {
    mRandomMovementTimer -= dt;
  } else {
    mRandomMovementTimer = 0.f;
    mRandomMovement = false;
  }
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    if (it->mActive) {
      it->mPos += mSpeed * (dt * it->mVel);
      if (!PointInBox(bounds, it->mPos)) {
        PlaceBoid(mgr, *it, bounds);
      }
    }
  }
  if (mValidModel) {
    for (int i = 0; i < 4; ++i) {
      mModels[i]->AnimationData()->SetPlaybackRate(1.f);
      mModels[i]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    }
  }
}

void CFishCloud::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);
  if (mValidModel) {
    for (int i = 0; i < 4; ++i) {
      mModels[i]->AnimationData()->PreRender();
    }
  }
  SetPreRenderClipped(false);
}

void CFishCloud::RenderBoid(int idx, const CBoid& boid, uint& drawMask, const bool thermalHot,
                            const CModelFlags& flags) const {
  const uint modelIndex = idx & 3;
  uint mask = drawMask;
  CModelData& modelData = *mModels[modelIndex];
  CAnimData& animData = *modelData.AnimationData();
  CSkinnedModel& model = modelData.PickAnimatedModel(CModelData::kWM_Normal);
  const uint bit = 1 << modelIndex;
  if (mask & bit) {
    mask &= ~bit;
    animData.BuildPose();
    model.Calculate(animData.GetPose(), rstl::optional_object< CVertexMorphEffect >(), nullptr,
                    mPosWorkspaces[modelIndex].get());
  }
  gpRender->SetModelMatrix(CTransform4f::LookAt(boid.mPos, boid.mPos + boid.mVel));
  const float* const positions = mPosWorkspaces[modelIndex].get();
  const float* const normals = mNrmWorkspaces[modelIndex];
  if (thermalHot) {
    const CModelFlags thermalFlags = CModelFlags::Normal().DepthCompareUpdate(true, false);
    CModelData::ThermalDraw(model, positions, normals, CColor(0xffffffff),
                            CColor(static_cast< uchar >(0), static_cast< uchar >(0),
                                   static_cast< uchar >(0), static_cast< uchar >(64)),
                            thermalFlags);
  } else {
    model.Draw(positions, normals, flags);
  }
  drawMask = mask;
}

void CFishCloud::Render(const CStateManager& mgr) const {
  if (!GetActive()) {
    return;
  }
  const bool thermalHot = mgr.GetThermalDrawFlag() == kTD_Hot;
  CModelFlags flags = CModelFlags::Normal();
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    CColor color = mColor;
    color.SetAlpha(static_cast< uchar >(76));
    flags = CModelFlags(CModelFlags::kT_Blend, color);
  } else {
    flags = CModelFlags(CModelFlags::kT_One, mColor);
  }
  RenderParticles();
  if (mValidModel) {
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
    uint drawMask = ~0;
    int index = 0;
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it, ++index) {
      if (it->mActive) {
        RenderBoid(index, *it, drawMask, thermalHot, flags);
      }
    }
  } else {
    CModelData& modelData = const_cast< CModelData& >(*GetModelData());
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
    gpRender->SetModelMatrix(CTransform4f::Identity());
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
      if (it->mActive) {
        modelData.SetScale(CVector3f(it->mScale, it->mScale, it->mScale));
        modelData.Render(mgr, CTransform4f::LookAt(it->mPos, it->mPos + it->mVel), nullptr,
                         flags);
      }
    }
  }
}

void CFishCloud::KillBoid(CBoid& boid) {
  boid.mActive = false;
  AddParticles(boid.mPos);
  const int areaId = GetCurrentAreaId().Value();
  CAudioSys::C3DEmitterParmData parms(250.f, 0.1f, 1, 127, 20);
  parms.mPos = boid.mPos;
  parms.mDir = CVector3f::Up();
  parms.mSfxId = mDeathSfx;
  CSfxManager::AddEmitter(parms, true, CSfxManager::kMedPriority, false, areaId);
}

void CFishCloud::Touch(CActor& other, CStateManager& mgr) {
  CActor::Touch(other, mgr);
  if (CWeapon* weapon = TCastToPtr< CWeapon >(other)) {
    if (!mEnableWeaponRepelDamping && mRepelFromThreats) {
      int index = 0;
      for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it, ++index) {
        if ((index & 3) == (mThinkCounter & 3)) {
          ApplyRepulsion(*it, weapon->GetTranslation(), 8.f,
                         mWeaponRepelMagnitude - mWeaponRepelDamping);
        }
      }
    }
    mEnableWeaponRepelDamping = true;
    if (mKillable) {
      const rstl::optional_object< CAABox > touchBounds = weapon->GetTouchBounds();
      if (touchBounds.valid()) {
        const CAABox bounds = *touchBounds;
        const float radius = mWeaponKillRadius;
        for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
          if (it->mActive) {
            const CVector3f extent(radius, radius, radius);
            const CAABox boidBounds(it->mPos - extent, it->mPos + extent);
            if (bounds.DoBoundsOverlap(boidBounds)) {
              KillBoid(*it);
            }
          }
        }
      }
    }
  }
  if (mRepelFromThreats) {
    if (CPlayer* player = TCastToPtr< CPlayer >(other)) {
      CRandom16& random = *mgr.Random();
      const CVector3f playerPos = player->GetTranslation();
      for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
        CVector3f adjustedPos = playerPos;
        const CVector3f delta = it->GetTranslation() - adjustedPos;
        const float dz = delta.GetZ();
        if (dz > 0.f && dz < 2.3) {
          adjustedPos.SetZ(it->GetTranslation().GetZ());
        }
        adjustedPos[kDX] += 0.2f * random.Float() - 0.1f;
        adjustedPos[kDY] += 0.2f * random.Float() - 0.1f;
        ApplyRepulsion(*it, adjustedPos, 8.f, mPlayerRepelMagnitude - mPlayerRepelDamping);
      }
    }
    mEnablePlayerRepelDamping = true;
  }
}

void CFishCloud::CreatePartitionList() {
  const CAABox bounds = GetBoundingBox();
  mBoidPartitionLists.reserve(343);
}

void CFishCloud::UpdatePartitionList() {
  mBoidPartitionLists.clear();
  for (int i = 0; i < mBoidPartitionLists.capacity(); ++i) {
    mBoidPartitionLists.push_back(nullptr);
  }
  const CAABox bounds = GetBoundingBox();
  const CVector3f& min = bounds.GetMinPoint();
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    const CVector3f indices =
        CVector3f::ByElementMultiply(mOoPartitionPitch, it->GetTranslation() - min);
    const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
    if (index >= 0 && index < 343) {
      it->mNext = mBoidPartitionLists[index];
      mBoidPartitionLists[index] = &*it;
    }
  }
}

CFishCloud::CBoid* CFishCloud::GetListAt(const CVector3f& pos) {
  const CAABox bounds = GetBoundingBox();
  const CVector3f indices =
      CVector3f::ByElementMultiply(mOoPartitionPitch, pos - bounds.GetMinPoint());
  const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
  if (index < 0 || index >= 343) {
    return nullptr;
  }
  return mBoidPartitionLists[index];
}

void CFishCloud::BuildBoidNearList(const CVector3f& pos, float radius,
                                   rstl::reserved_vector< CBoid*, 25 >& nearList) {
  const float radiusSquared = radius * radius;
  const CAABox bounds = GetBoundingBox();
  const CVector3f& min = bounds.GetMinPoint();
  const CVector3f& max = bounds.GetMaxPoint();
  const float x = rstl::max_val(radius * mOoPartitionPitch.GetX(), mPartitionPitch.GetX());
  const float y = rstl::max_val(radius * mOoPartitionPitch.GetY(), mPartitionPitch.GetY());
  const float z = rstl::max_val(radius * mOoPartitionPitch.GetZ(), mPartitionPitch.GetZ());
  int remaining = 25;
  for (float ox = 0.01f - x; ox < x; ox += mPartitionPitch.GetX()) {
    const float px = ox + pos.GetX();
    if (px < min.GetX()) {
      continue;
    }
    if (px >= max.GetX()) {
      break;
    }
    for (float oy = 0.01f - y; oy < y; oy += mPartitionPitch.GetY()) {
      const float py = oy + pos.GetY();
      if (py < min.GetY()) {
        continue;
      }
      if (py >= max.GetY()) {
        break;
      }
      for (float oz = 0.01f - z; oz < z; oz += mPartitionPitch.GetZ()) {
        const float pz = oz + pos.GetZ();
        if (pz < min.GetZ()) {
          continue;
        }
        if (pz >= max.GetZ()) {
          break;
        }
        const CVector3f indices =
            CVector3f::ByElementMultiply(mOoPartitionPitch, CVector3f(px, py, pz) - min);
        const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
        if (index < 0) {
          continue;
        }
        if (index >= 343) {
          break;
        }
        for (CBoid* boid = mBoidPartitionLists[index]; boid != nullptr; boid = boid->mNext) {
          if (boid->mActive) {
            const CVector3f delta = boid->mPos - pos;
            const float distanceSquared = delta.MagSquared();
            if (distanceSquared != 0.f && distanceSquared < radiusSquared) {
              nearList.push_back(boid);
              if (--remaining == 0) {
                return;
              }
            }
          }
        }
      }
    }
  }
}

void CFishCloud::OldBuildBoidNearList(const CVector3f& pos, float radius,
                                      rstl::reserved_vector< CBoid*, 25 >& nearList) {
  const float radiusSquared = radius * radius;
  int remaining = 25;
  for (CBoid* boid = GetListAt(pos); boid != nullptr && remaining != 0; boid = boid->mNext) {
    if (boid->mActive) {
      const CVector3f delta = boid->GetTranslation() - pos;
      const float distanceSquared = delta.MagSquared();
      if (distanceSquared != 0.f && distanceSquared < radiusSquared) {
        nearList.push_back(boid);
        --remaining;
      }
    }
  }
}

static inline CVector3f FishCloudCross(const CVector3f& lhs, const CVector3f& rhs) {
  const float lX = lhs.GetX();
  const float lY = lhs.GetY();
  const float lZ = lhs.GetZ();
  const float rX = rhs.GetX();
  const float rY = rhs.GetY();
  const float rZ = rhs.GetZ();
  const float z = lX * rY - rX * lY;
  const float y = lZ * rX - rZ * lX;
  const float x = lY * rZ - rY * lZ;
  return CVector3f(x, y, z);
}

void CFishCloud::ApplyRotation(CBoid& boid, float magnitude, const CVector3f& point, float radius,
                               bool clockwise) {
  CVector3f delta = boid.mPos - point;
  delta[kDZ] = 0.f;
  const float distance = delta.Magnitude();
  const CVector3f align = clockwise ? FishCloudCross(delta.AsNormalized(), CVector3f::Up())
                                    : FishCloudCross(CVector3f::Up(), delta / distance);
  const CVector3f velocity = boid.mVel;
  const float weight = distance > radius ? 0.f : 1.f - distance / radius;
  const float angle = CVector3f::GetAngleDiff(velocity, align) / M_PIF;
  const float weightedAngle = angle * weight;
  boid.mVel += weightedAngle * (magnitude * align);
}

void CFishCloud::ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f average(0.f, 0.f, 0.f);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      average += (*it)->mVel;
    }
    average = average / float(nearList.size());
    const CVector3f velocity = boid.mVel;
    const float angle = CVector3f::GetAngleDiff(velocity, average) / M_PIF;
    boid.mVel += angle * (mAlignmentWeight * average);
  }
}

void CFishCloud::ApplyWander(CStateManager& mgr, CBoid& boid) {
  const float x = boid.mVel.GetX();
  const float y = boid.mVel.GetY();
  const float angle = mMaxScatterAngle * (M_PIF * (mgr.Random()->Float() - 0.5f));
  const CVector3f scatter(x * CMath::FastCosR(angle) - y * CMath::FastSinR(angle),
                          x * CMath::FastSinR(angle) + y * CMath::FastCosR(angle), 0.f);
  boid.mVel += mScatterVel * scatter;
}

void CFishCloud::ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f average(0.f, 0.f, 0.f);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      average += (*it)->GetTranslation();
    }
    average = average / float(nearList.size());
    ApplyCohesion(boid, average, mSeparationRadius, mCohesionMagnitude);
  }
}

void CFishCloud::ApplyCohesion(CBoid& boid, const CVector3f& point, float radius, float magnitude) {
  const CVector3f delta = point - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float weight = distanceSquared > radius ? 1.f : distanceSquared / radius;
    boid.mVel += weight * delta.AsNormalized() * magnitude;
  }
}

void CFishCloud::ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f nearest(0.f, 0.f, 0.f);
    float minDistanceSquared = FLT_MAX;
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      const CVector3f delta = boid.GetTranslation() - (*it)->GetTranslation();
      const float distanceSquared = delta.MagSquared();
      if (distanceSquared < minDistanceSquared) {
        minDistanceSquared = distanceSquared;
        nearest = (*it)->GetTranslation();
      }
    }
    ApplySeparation(boid, nearest, mSeparationRadius, mSeparationMagnitude);
  }
}

void CFishCloud::ApplySeparation(CBoid& boid, const CVector3f& point, float radius,
                                 float magnitude) {
  const CVector3f delta = boid.GetTranslation() - point;
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float radiusSquared = radius * radius;
    if (distanceSquared < radiusSquared) {
      const float weight = 1.f - distanceSquared / radiusSquared;
      boid.mVel += weight * delta.AsNormalized() * magnitude;
    }
  }
}

void CFishCloud::ApplyAttraction(CBoid& boid, const CVector3f& point, float radius,
                                 float magnitude) {
  const CVector3f delta = point - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float radiusSquared = radius * radius;
    if (distanceSquared < radiusSquared) {
      const float weight = 1.f - distanceSquared / radiusSquared;
      boid.mVel += weight * delta.AsNormalized() * magnitude;
    }
  }
}

void CFishCloud::ApplyRepulsion(CBoid& boid, const CVector3f& point, float radius,
                                float magnitude) {
  ApplySeparation(boid, point, radius, magnitude);
}

void CFishCloud::ApplyContainment(CBoid& boid, const CAABox& aabb) {
  const float radius = mContainmentRadius;
  if (boid.mVel.CanBeNormalized()) {
    const CVector3f futurePos = boid.mPos + radius * (mSpeed * boid.mVel.AsNormalized());
    if (!PointInBox(aabb, futurePos)) {
      ApplyAttraction(boid, aabb.GetCenterPoint(), 100000.f, mContainmentMagnitude);
    }
  }
}

void CFishCloud::AddParticles(const CVector3f& pos) {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    mParticleGens[i]->SetParticleEmission(true);
    mParticleGens[i]->SetTranslation(pos);
    mParticleGens[i]->ForceParticleCreation(mDeathParticleCounts[i]);
    mParticleGens[i]->SetParticleEmission(false);
  }
}

void CFishCloud::UpdateParticles(float dt) {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    mParticleGens[i]->Update(dt);
  }
}

void CFishCloud::RenderParticles() const {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    gpRender->AddParticleGen(*mParticleGens[i]);
  }
}

CFishCloud::~CFishCloud() {}
