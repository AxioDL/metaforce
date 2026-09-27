#include "MetroidPrime/ScriptObjects/CSnakeWeedSwarm.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include <math.h>

CSnakeWeedSwarm::CBoid::CBoid(const CVector3f& pos, float depth, float speed, float scale)
: mPos(pos), mState(kSWBS_Raising), mDepth(depth), mSpeed(speed), mScale(scale) {}

void CSnakeWeedSwarm::CBoid::SetBoidState(ESnakeWeedBoidState state) { mState = state; }
CSnakeWeedSwarm::ESnakeWeedBoidState CSnakeWeedSwarm::CBoid::GetBoidState() const {
  return mState;
}
const CVector3f& CSnakeWeedSwarm::CBoid::GetLocation() const { return mPos; }
float CSnakeWeedSwarm::CBoid::GetTimeOut() const { return mTimeOut; }
void CSnakeWeedSwarm::CBoid::SetTimeOut(float time) { mTimeOut = time; }
float CSnakeWeedSwarm::CBoid::GetDepth() const { return mDepth; }
void CSnakeWeedSwarm::CBoid::SetDepth(float depth) { mDepth = depth; }
float CSnakeWeedSwarm::CBoid::GetSpeed() const { return mSpeed; }
void CSnakeWeedSwarm::CBoid::SetSpeed(float speed) { mSpeed = speed; }
float CSnakeWeedSwarm::CBoid::GetSize() const { return mScale; }

CSnakeWeedSwarm::CSnakeWeedSwarm(TUniqueId uid, const bool active, const rstl::string& name,
                                 const CEntityInfo& info, const CVector3f& pos,
                                 const CVector3f& scale, const CAnimRes& animRes,
                                 const CActorParameters& actParms, float spacing, float height,
                                 float variance, float weaponDamageRadius, float maxPlayerDistance,
                                 float loweredTime, float loweredTimeVariation, float maxDepth,
                                 float speed, float speedVariation, float slopeAngle,
                                 float scaleMin, float scaleMax, float distanceBelowGround,
                                 const CDamageInfo& damageInfo, float f15, uint sfxId1, uint sfxId2,
                                 uint sfxId3, uint particle1, uint particleCount, uint particle2,
                                 float f16)
: CActor(uid, active, name, info, CTransform4f::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(kMT_Trigger, kMT_NonSolidDamageable), actParms, kInvalidUniqueId)
, mScale(scale)
, mBoidSpacing(spacing)
, mHeight(height)
, mVariance(variance)
, mWeaponDamageRadius(weaponDamageRadius)
, mMaxPlayerDistance(maxPlayerDistance)
, mLoweredTime(loweredTime)
, mLoweredTimeVariation(loweredTimeVariation)
, mMaxDepth(maxDepth)
, mSpeed(speed)
, mSpeedVariation(speedVariation)
, mCosSlopeAngle(cosf((M_PIF / 180.f) * slopeAngle))
, mScaleMin(scaleMin)
, mScaleMax(scaleMax)
, mDistanceBelowGround(distanceBelowGround)
, x12c_(0)
, mHasGround(false)
, mModelAssetDirty(false)
, mPlayerTouching(false)
, mTouchBounds(CAABox::MakeMaxInvertedBox())
, mDamageInfo(damageInfo)
, mBoidPositions(rs_new rstl::vector< CVector3f >)
, mBoidPlacement(rs_new rstl::vector< EBoidPlacement >)
, mSfx1(CSfxManager::TranslateSFXID(
      static_cast< int >(sfxId1) == -1 ? CSfxManager::kInternalInvalidSfxId : sfxId1))
, mSfx2(CSfxManager::TranslateSFXID(
      static_cast< int >(sfxId2) == -1 ? CSfxManager::kInternalInvalidSfxId : sfxId2))
, mSfx3(CSfxManager::TranslateSFXID(
      static_cast< int >(sfxId3) == -1 ? CSfxManager::kInternalInvalidSfxId : sfxId3))
, mParticleGenDescA(nullptr)
, mParticleGenDescB(nullptr)
, mParticleGen1(nullptr)
, mParticleGen2(nullptr)
, mParticleCount(particleCount)
, x200_(f16)
, mParticleTimer(0.f) {
  SetActorLights(actParms.GetLighting().MakeActorLights());
  mModelData.push_back(rs_new CModelData(animRes));
  mModelData.push_back(rs_new CModelData(animRes));
  mModelData.push_back(rs_new CModelData(animRes));
  mModelData.push_back(rs_new CModelData(animRes));
  if (actParms.GetXRay().first != 0) {
    for (int i = 0; i < 4; ++i) {
      mModelData[i]->SetXRayModel(actParms.GetXRay());
    }
    mModelAssetDirty = true;
  }
  if (actParms.GetInfra().first != 0) {
    for (int i = 0; i < 4; ++i) {
      mModelData[i]->SetInfraModel(actParms.GetInfra());
    }
    mModelAssetDirty = true;
  }
  if (particle1 != kInvalidAssetId) {
    mParticleGenDescA =
        rs_new TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', particle1)));
    mParticleGen1 = rs_new CElementGen(*mParticleGenDescA);
  }
  if (particle2 != kInvalidAssetId) {
    mParticleGenDescA =
        rs_new TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', particle2)));
    mParticleGen2 = rs_new CElementGen(*mParticleGenDescA);
  }
}

ENTITY_ACCEPT_IMPL(CSnakeWeedSwarm)

void CSnakeWeedSwarm::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    InitAnimBoids(mgr, CModelData::kWM_Normal);
    SetCalculateLighting(true);
    ActorLights()->SetCastShadows(true);
    break;
  case kSM_Deleted:
    if (mSfxHandle) {
      CSfxManager::RemoveEmitter(mSfxHandle);
      mSfxHandle.Clear();
    }
    break;
  }
}

void CSnakeWeedSwarm::InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which) {
  mPosWorkspaces.clear();
  mNrmWorkspaces.clear();
  for (int i = 0; i < 4; ++i) {
    float* normals;
    mPosWorkspaces.push_back(
        mModelData[i]->PickAnimatedModel(which).AllocateNewWorkspace(&normals));
    mNrmWorkspaces.push_back(normals);
    mModelData[i]->EnableLooping(true);
    mModelData[i]->AdvanceAnimation(
        mModelData[i]->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")) *
            (float(i) / 4.f),
        mgr, GetCurrentAreaId(), true);
  }
  mWhich = which;
}

CAABox CSnakeWeedSwarm::GetBoundingBox() const {
  const CVector3f scale =
      CVector3f(0.75f * mScale.GetX(), 0.75f * mScale.GetY(), 0.75f * mScale.GetZ());
  CVector3f min;
  const CVector3f max = GetTranslation() + scale;
  min = GetTranslation() - scale;
  return CAABox(min, max);
}

rstl::optional_object< CAABox > CSnakeWeedSwarm::GetTouchBounds() const {
  if (!mHasGround) {
    return rstl::optional_object_null();
  }
  return mTouchBounds;
}

void CSnakeWeedSwarm::UpdateTouchBounds() {
  if (mBoids.size() > 0) {
    mTouchBounds = CAABox::MakeMaxInvertedBox();
    const float radius = mWeaponDamageRadius;
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
      mTouchBounds.AccumulateBounds(it->GetLocation() - CVector3f(radius, radius, radius));
      mTouchBounds.AccumulateBounds(it->GetLocation() + CVector3f(radius, radius, radius));
    }
  } else {
    mTouchBounds = CAABox(GetTranslation(), GetTranslation());
  }
  SetTransformDirty(true);
}

void CSnakeWeedSwarm::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  if (GetActive()) {
    if (mParticleGen1.get()) {
      mParticleGen1->Update(dt);
    }
    if (mParticleGen2.get()) {
      mParticleGen2->Update(dt);
    }
    mParticleTimer -= dt;
    bool emitParticle = false;
    if (mParticleTimer < 0.f) {
      mParticleTimer = 0.f;
      emitParticle = true;
    }
    if (!mHasGround) {
      CreateSwarm(mgr);
    }
    if (mHasGround && mBoidPositions.get() && mBoidPositions->size() > 0) {
      FloodFill(mgr, CCast::FtoS(dt * mBoidPlacement->size() * 1.f) + 1);
    }
    if (mModelAssetDirty) {
      CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
      if (mWhich != which) {
        InitAnimBoids(mgr, which);
      }
    }
    for (int i = 0; i < 4; ++i) {
      mModelData[i]->AnimationData()->SetPlaybackRate(1.f);
      mModelData[i]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    }
    int raisedBoids = 0;
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
      CBoid& boid = *it;
      switch (boid.GetBoidState()) {
      case kSWBS_Raised:
        ++raisedBoids;
        if (mParticleGen2.get() && emitParticle) {
          AddContinuousParticles(boid.GetLocation());
        }
        break;
      case kSWBS_Raising:
        boid.SetDepth(boid.GetDepth() - dt * boid.GetSpeed());
        if (boid.GetDepth() <= 0.f) {
          boid.SetDepth(0.f);
          boid.SetBoidState(kSWBS_Raised);
        }
        break;
      case kSWBS_Lowered:
        boid.SetTimeOut(boid.GetTimeOut() - dt);
        if (boid.GetTimeOut() <= 0.f) {
          boid.SetBoidState(kSWBS_Raising);
          CSfxManager::AddEmitter(mSfx3, boid.GetLocation(), CVector3f::Zero(), true, false,
                                  CSfxManager::kMedPriority, GetCurrentAreaId().Value());
          AddRetreatParticles(boid.GetLocation());
        }
        break;
      case kSWBS_Lowering:
        boid.SetDepth(boid.GetDepth() + dt * boid.GetSpeed());
        if (boid.GetDepth() > mMaxDepth * boid.GetSize()) {
          boid.SetDepth(mMaxDepth * boid.GetSize());
          boid.SetTimeOut(mLoweredTimeVariation * mgr.Random()->Float() + mLoweredTime);
          boid.SetBoidState(kSWBS_Lowered);
        }
        break;
      }
    }
    if (raisedBoids > 0) {
      if (!mSfxHandle) {
        mSfxHandle =
            CSfxManager::AddEmitter(mSfx1, GetTranslation(), CVector3f::Zero(), true, true,
                                    CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      }
    } else if (mSfxHandle) {
      CSfxManager::RemoveEmitter(mSfxHandle);
      mSfxHandle.Clear();
    }
    if (mPlayerTouching) {
      mgr.ApplyDamage(
          GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
          mDamageInfo.MakeScaledForTime(dt),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
          CVector3f::Zero());
    }
    mPlayerTouching = false;
  }
}

void CSnakeWeedSwarm::Touch(CActor& actor, CStateManager& mgr) {
  if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
    if (projectile->GetCurrentDamageInfo().GetWeaponMode().GetType() != kWT_AI) {
      ScareSnakeWeeds(mgr, projectile->GetTranslation(), mWeaponDamageRadius);
    }
  }
  if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
      if ((it->GetLocation() - player->GetTranslation()).Magnitude() < mMaxPlayerDistance &&
          it->GetBoidState() == kSWBS_Raised) {
        mgr.DeliverScriptMsg(player, kInvalidUniqueId, kSM_InSnakeWeed);
        mPlayerTouching = true;
        return;
      }
    }
  }
}

void CSnakeWeedSwarm::ScareSnakeWeeds(CStateManager& mgr, const CVector3f& pos, float radius) {
  const float radiusSquared = radius * radius;
  for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it) {
    CBoid& boid = *it;
    if ((boid.GetLocation() - pos).MagSquared() < radiusSquared &&
        (boid.GetBoidState() == kSWBS_Raised || boid.GetBoidState() == kSWBS_Raising)) {
      boid.SetBoidState(kSWBS_Lowering);
      boid.SetSpeed(mSpeedVariation * mgr.Random()->Float() + mSpeed);
      CSfxManager::AddEmitter(mSfx2, boid.GetLocation(), CVector3f::Zero(), true, false,
                              CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      AddRetreatParticles(boid.GetLocation());
    }
  }
}

void CSnakeWeedSwarm::ApplyRadiusDamage(CVector3f pos, const CDamageInfo& info,
                                        CStateManager& mgr) {
  EWeaponType type = info.GetWeaponMode().GetType();
  if (type == kWT_Bomb || type == kWT_PowerBomb) {
    ScareSnakeWeeds(mgr, pos, info.GetRadius());
  }
}

void CSnakeWeedSwarm::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (!frustum.BoxInFrustumPlanes(mTouchBounds)) {
    SetPreRenderClipped(true);
  } else {
    SetPreRenderClipped(false);
    for (int i = 0; i < 4; ++i) {
      mModelData[i]->AnimationData()->PreRender();
    }
    bool buildLights = false;
    if (ActorLights() && mHasGround) {
      if (GetPreRenderHasMoved()) {
        buildLights = true;
        SetPreRenderHasMoved(false);
      } else if (ActorLights()->GetIsDirty() == true) {
        buildLights = true;
      }
      if (GetCalculateLighting()) {
        const CAABox bounds = mTouchBounds;
        if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
          ActorLights()->BuildConstantAmbientLighting(CColor::White());
        } else {
          if (buildLights == true && GetCurrentAreaId() != kInvalidAreaId) {
            CWorld* world = mgr.World();
            if (world->IsAreaValid(GetCurrentAreaId())) {
              ActorLights()->BuildAreaLightList(mgr, *world->GetArea(GetCurrentAreaId()), bounds);
            }
          }
          ActorLights()->BuildDynamicLightList(mgr, bounds);
        }
      }
    }
  }
}

void CSnakeWeedSwarm::RenderBoid(uint index, const CBoid& boid, uint& posesToBuild) const {
  const uint modelIndex = index & 3;
  int mask = posesToBuild;
  CModelData& data = *mModelData[modelIndex];
  CAnimData& animData = *data.AnimationData();
  CSkinnedModel& model = data.PickAnimatedModel(mWhich);
  const int bit = 1 << modelIndex;
  if (mask & bit) {
    mask &= ~bit;
    animData.BuildPose();
    model.Calculate(animData.GetPose(), rstl::optional_object< CVertexMorphEffect >(), nullptr,
                    mPosWorkspaces[modelIndex].get());
  }
  gpRender->SetModelMatrix(
      CTransform4f::Translate(boid.GetLocation() - CVector3f(0.f, 0.f, boid.GetDepth())) *
      CTransform4f::Scale(boid.GetSize()));
  model.Draw(mPosWorkspaces[modelIndex].get(), mNrmWorkspaces[modelIndex],
             CModelFlags(CModelFlags::kT_Opaque, 1.f));
  posesToBuild = mask;
}

void CSnakeWeedSwarm::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (!GetPreRenderClipped()) {
    if (mParticleGen1.get()) {
      gpRender->AddParticleGen(*mParticleGen1);
    }
    if (mParticleGen2.get()) {
      gpRender->AddParticleGen(*mParticleGen2);
    }
    uint posesToBuild = -1;
    if (GetActorLights()) {
      GetActorLights()->ActivateLights();
    } else {
      CGraphics::DisableAllLights();
      gpRender->SetAmbientColor(CColor::White());
    }
    uint index = 0;
    for (AUTO(it, mBoids.begin()); it != mBoids.end(); ++it, ++index) {
      RenderBoid(index, *it, posesToBuild);
    }
    CGraphics::DisableAllLights();
  }
}

void CSnakeWeedSwarm::CreateSwarm(CStateManager& mgr) {
  static const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CAABox box = GetBoundingBox();
  const CVector3f pos = box.GetCenterPoint();
  const CVector3f down(0.f, 0.f, -1.f);
  const CRayCastResult result = mgr.RayStaticIntersection(
      pos, down, box.GetMaxPoint().GetZ() - box.GetMinPoint().GetZ(), filter);
  const CVector3f point = result.GetPoint();
  if (result.IsValid()) {
    mBoids.reserve(GetGridWidth() * GetGridDepth());
    mBoidPositions->reserve(GetGridWidth() * GetGridDepth());
    mBoidPlacement->resize(GetGridWidth() * GetGridDepth(), kBP_None);
    mBoidPositions->push_back(point);
    mHasGround = true;
  }
}

CVector2i CSnakeWeedSwarm::GetGridPosition(CVector3f pos) {
  const CAABox box = GetBoundingBox();
  return CVector2i(static_cast< int >((pos.GetX() - box.GetMinPoint().GetX()) / mBoidSpacing),
                   static_cast< int >((pos.GetY() - box.GetMinPoint().GetY()) / mBoidSpacing));
}

int CSnakeWeedSwarm::GetGridWidth() {
  const CAABox box = GetBoundingBox();
  return static_cast< int >((box.GetMaxPoint().GetX() - box.GetMinPoint().GetX()) /
                            mBoidSpacing) +
         1;
}

int CSnakeWeedSwarm::GetGridDepth() {
  const CAABox box = GetBoundingBox();
  return static_cast< int >((box.GetMaxPoint().GetY() - box.GetMinPoint().GetY()) /
                            mBoidSpacing) +
         1;
}

void CSnakeWeedSwarm::PushBackPoint(CVector3f pos) {
  const int width = GetGridWidth();
  const int depth = GetGridDepth();
  const CVector2i& grid = CVector2i(GetGridPosition(pos));
  if (grid[0] < 0 || grid[0] >= width) {
    return;
  }
  if (grid[1] < 0 || grid[1] >= depth) {
    return;
  }
  const int index = grid[0] + width * grid[1];
  if ((*mBoidPlacement)[index] == kBP_None) {
    (*mBoidPlacement)[index] = kBP_Ready;
    mBoidPositions->push_back(pos);
  }
}

void CSnakeWeedSwarm::FloodFill(CStateManager& mgr, int count) {
  const int width = GetGridWidth();
  for (int i = 0; i < count && mBoidPositions->size() > 0; ++i) {
    const CVector3f pos = mBoidPositions->back();
    mBoidPositions->pop_back();
    const CVector2i& grid = CVector2i(GetGridPosition(pos));
    if (PlaceSnakeWeedAtPoint(pos, mgr)) {
      (*mBoidPlacement)[grid[0] + width * grid[1]] = kBP_Placed;
      PushBackPoint(CVector3f(pos[kDX], pos[kDY] - mBoidSpacing, pos[kDZ]));
      PushBackPoint(CVector3f(pos[kDX], mBoidSpacing + pos[kDY], pos[kDZ]));
      PushBackPoint(CVector3f(pos[kDX] - mBoidSpacing, pos[kDY], pos[kDZ]));
      PushBackPoint(CVector3f(mBoidSpacing + pos[kDX], pos[kDY], pos[kDZ]));
    } else {
      (*mBoidPlacement)[grid[0] + width * grid[1]] = kBP_Invalid;
    }
  }
  UpdateTouchBounds();
  if (mBoidPositions->size() == 0) {
    mBoidPositions = nullptr;
    mBoidPlacement = nullptr;
  }
}

float CSnakeWeedSwarm::GetXVariance(const CVector3f& pos) {
  const float f = 8.21395f * pos.GetX() + 0.112869f * (pos.GetY() * pos.GetY());
  return mVariance * (2.f * CMath::AbsF(f - static_cast< int >(f)) - 1.f);
}

float CSnakeWeedSwarm::GetYVariance(const CVector3f& pos) {
  const float f = 2.4729404f * pos.GetY() + 0.3478602f * (pos.GetX() * pos.GetX());
  return mVariance * (2.f * CMath::AbsF(f - static_cast< int >(f)) - 1.f);
}

bool CSnakeWeedSwarm::PlaceSnakeWeedAtPoint(const CVector3f& point, CStateManager& mgr) {
  static const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CVector3f pos = point + CVector3f(GetXVariance(point), GetYVariance(point), mHeight);
  const CVector3f down(0.f, 0.f, -1.f);
  const CRayCastResult result = mgr.RayStaticIntersection(pos, down, 2.f * mHeight, filter);
  if (result.IsValid() && CVector3f::Dot(result.GetPlane().GetNormal(), CVector3f(0.f, 0.f, 1.f)) >
                              mCosSlopeAngle) {
    const CVector3f boidPos = result.GetPoint() - CVector3f(0.f, 0.f, mDistanceBelowGround);
    const CBoid boid(boidPos, mMaxDepth, mSpeed + mSpeedVariation,
                     (mScaleMax - mScaleMin) * mgr.Random()->Float() + mScaleMin);
    mBoids.push_back(boid);
    return true;
  }
  return false;
}

void CSnakeWeedSwarm::AddContinuousParticles(const CVector3f& pos) {
  if (mParticleGen2.get()) {
    mParticleGen2->SetParticleEmission(true);
    mParticleGen2->SetTranslation(pos);
    mParticleGen2->ForceParticleCreation(1);
    mParticleGen2->SetParticleEmission(false);
  }
}

void CSnakeWeedSwarm::AddRetreatParticles(const CVector3f& pos) {
  if (mParticleGen1.get()) {
    mParticleGen1->SetParticleEmission(true);
    mParticleGen1->SetTranslation(pos);
    mParticleGen1->ForceParticleCreation(mParticleCount);
    mParticleGen1->SetParticleEmission(false);
  }
}

CSnakeWeedSwarm::~CSnakeWeedSwarm() {}
