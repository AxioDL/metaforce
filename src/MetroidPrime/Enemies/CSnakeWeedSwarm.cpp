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
: x0_pos(pos), xc_state(kSWBS_Raising), x14_depth(depth), x18_speed(speed), x20_scale(scale) {}

void CSnakeWeedSwarm::CBoid::SetBoidState(ESnakeWeedBoidState state) { xc_state = state; }
CSnakeWeedSwarm::ESnakeWeedBoidState CSnakeWeedSwarm::CBoid::GetBoidState() const {
  return xc_state;
}
const CVector3f& CSnakeWeedSwarm::CBoid::GetLocation() const { return x0_pos; }
float CSnakeWeedSwarm::CBoid::GetTimeOut() const { return x10_timeOut; }
void CSnakeWeedSwarm::CBoid::SetTimeOut(float time) { x10_timeOut = time; }
float CSnakeWeedSwarm::CBoid::GetDepth() const { return x14_depth; }
void CSnakeWeedSwarm::CBoid::SetDepth(float depth) { x14_depth = depth; }
float CSnakeWeedSwarm::CBoid::GetSpeed() const { return x18_speed; }
void CSnakeWeedSwarm::CBoid::SetSpeed(float speed) { x18_speed = speed; }
float CSnakeWeedSwarm::CBoid::GetSize() const { return x20_scale; }

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
, xe8_scale(scale)
, xf4_boidSpacing(spacing)
, xf8_height(height)
, xfc_variance(variance)
, x100_weaponDamageRadius(weaponDamageRadius)
, x104_maxPlayerDistance(maxPlayerDistance)
, x108_loweredTime(loweredTime)
, x10c_loweredTimeVariation(loweredTimeVariation)
, x110_maxDepth(maxDepth)
, x114_speed(speed)
, x118_speedVariation(speedVariation)
, x11c_cosSlopeAngle(cosf((M_PIF / 180.f) * slopeAngle))
, x120_scaleMin(scaleMin)
, x124_scaleMax(scaleMax)
, x128_distanceBelowGround(distanceBelowGround)
, x12c_(0)
, x140_24_hasGround(false)
, x140_25_modelAssetDirty(false)
, x140_26_playerTouching(false)
, x144_touchBounds(CAABox::MakeMaxInvertedBox())
, x15c_damageInfo(damageInfo)
, x1c8_boidPositions(rs_new rstl::vector< CVector3f >)
, x1cc_boidPlacement(rs_new rstl::vector< EBoidPlacement >)
, x1d0_sfx1(CSfxManager::TranslateSFXID(
      static_cast< int >(sfxId1) == -1 ? CSfxManager::kInternalInvalidSfxId : sfxId1))
, x1d2_sfx2(CSfxManager::TranslateSFXID(
      static_cast< int >(sfxId2) == -1 ? CSfxManager::kInternalInvalidSfxId : sfxId2))
, x1d4_sfx3(CSfxManager::TranslateSFXID(
      static_cast< int >(sfxId3) == -1 ? CSfxManager::kInternalInvalidSfxId : sfxId3))
, x1dc_particleGenDesc(nullptr)
, x1e4_particleGenDesc(nullptr)
, x1ec_particleGen1(nullptr)
, x1f4_particleGen2(nullptr)
, x1fc_particleCount(particleCount)
, x200_(f16)
, x204_particleTimer(0.f) {
  SetActorLights(actParms.GetLighting().MakeActorLights());
  x1b0_modelData.push_back(rs_new CModelData(animRes));
  x1b0_modelData.push_back(rs_new CModelData(animRes));
  x1b0_modelData.push_back(rs_new CModelData(animRes));
  x1b0_modelData.push_back(rs_new CModelData(animRes));
  if (actParms.GetXRay().first != 0) {
    for (int i = 0; i < 4; ++i) {
      x1b0_modelData[i]->SetXRayModel(actParms.GetXRay());
    }
    x140_25_modelAssetDirty = true;
  }
  if (actParms.GetInfra().first != 0) {
    for (int i = 0; i < 4; ++i) {
      x1b0_modelData[i]->SetInfraModel(actParms.GetInfra());
    }
    x140_25_modelAssetDirty = true;
  }
  if (particle1 != kInvalidAssetId) {
    x1dc_particleGenDesc =
        rs_new TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', particle1)));
    x1ec_particleGen1 = rs_new CElementGen(*x1dc_particleGenDesc);
  }
  if (particle2 != kInvalidAssetId) {
    x1dc_particleGenDesc =
        rs_new TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', particle2)));
    x1f4_particleGen2 = rs_new CElementGen(*x1dc_particleGenDesc);
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
    if (x1d8_sfxHandle) {
      CSfxManager::RemoveEmitter(x1d8_sfxHandle);
      x1d8_sfxHandle.Clear();
    }
    break;
  }
}

void CSnakeWeedSwarm::InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which) {
  x178_posWorkspaces.clear();
  x19c_nrmWorkspaces.clear();
  for (int i = 0; i < 4; ++i) {
    float* normals;
    x178_posWorkspaces.push_back(
        x1b0_modelData[i]->PickAnimatedModel(which).AllocateNewWorkspace(&normals));
    x19c_nrmWorkspaces.push_back(normals);
    x1b0_modelData[i]->EnableLooping(true);
    x1b0_modelData[i]->AdvanceAnimation(
        x1b0_modelData[i]->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")) *
            (float(i) / 4.f),
        mgr, GetCurrentAreaId(), true);
  }
  x1c4_which = which;
}

CAABox CSnakeWeedSwarm::GetBoundingBox() const {
  const CVector3f scale =
      CVector3f(0.75f * xe8_scale.GetX(), 0.75f * xe8_scale.GetY(), 0.75f * xe8_scale.GetZ());
  CVector3f min;
  const CVector3f max = GetTranslation() + scale;
  min = GetTranslation() - scale;
  return CAABox(min, max);
}

rstl::optional_object< CAABox > CSnakeWeedSwarm::GetTouchBounds() const {
  if (!x140_24_hasGround) {
    return rstl::optional_object_null();
  }
  return x144_touchBounds;
}

void CSnakeWeedSwarm::UpdateTouchBounds() {
  if (x130_boids.size() > 0) {
    x144_touchBounds = CAABox::MakeMaxInvertedBox();
    const float radius = x100_weaponDamageRadius;
    for (AUTO(it, x130_boids.begin()); it != x130_boids.end(); ++it) {
      x144_touchBounds.AccumulateBounds(it->GetLocation() - CVector3f(radius, radius, radius));
      x144_touchBounds.AccumulateBounds(it->GetLocation() + CVector3f(radius, radius, radius));
    }
  } else {
    x144_touchBounds = CAABox(GetTranslation(), GetTranslation());
  }
  SetTransformDirty(true);
}

void CSnakeWeedSwarm::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  if (GetActive()) {
    if (x1ec_particleGen1.get()) {
      x1ec_particleGen1->Update(dt);
    }
    if (x1f4_particleGen2.get()) {
      x1f4_particleGen2->Update(dt);
    }
    x204_particleTimer -= dt;
    bool emitParticle = false;
    if (x204_particleTimer < 0.f) {
      x204_particleTimer = 0.f;
      emitParticle = true;
    }
    if (!x140_24_hasGround) {
      CreateSwarm(mgr);
    }
    if (x140_24_hasGround && x1c8_boidPositions.get() && x1c8_boidPositions->size() > 0) {
      FloodFill(mgr, CCast::FtoS(dt * x1cc_boidPlacement->size() * 1.f) + 1);
    }
    if (x140_25_modelAssetDirty) {
      CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
      if (x1c4_which != which) {
        InitAnimBoids(mgr, which);
      }
    }
    for (int i = 0; i < 4; ++i) {
      x1b0_modelData[i]->AnimationData()->SetPlaybackRate(1.f);
      x1b0_modelData[i]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    }
    int raisedBoids = 0;
    for (AUTO(it, x130_boids.begin()); it != x130_boids.end(); ++it) {
      CBoid& boid = *it;
      switch (boid.GetBoidState()) {
      case kSWBS_Raised:
        ++raisedBoids;
        if (x1f4_particleGen2.get() && emitParticle) {
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
          CSfxManager::AddEmitter(x1d4_sfx3, boid.GetLocation(), CVector3f::Zero(), true, false,
                                  CSfxManager::kMedPriority, GetCurrentAreaId().Value());
          AddRetreatParticles(boid.GetLocation());
        }
        break;
      case kSWBS_Lowering:
        boid.SetDepth(boid.GetDepth() + dt * boid.GetSpeed());
        if (boid.GetDepth() > x110_maxDepth * boid.GetSize()) {
          boid.SetDepth(x110_maxDepth * boid.GetSize());
          boid.SetTimeOut(x10c_loweredTimeVariation * mgr.Random()->Float() + x108_loweredTime);
          boid.SetBoidState(kSWBS_Lowered);
        }
        break;
      }
    }
    if (raisedBoids > 0) {
      if (!x1d8_sfxHandle) {
        x1d8_sfxHandle =
            CSfxManager::AddEmitter(x1d0_sfx1, GetTranslation(), CVector3f::Zero(), true, true,
                                    CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      }
    } else if (x1d8_sfxHandle) {
      CSfxManager::RemoveEmitter(x1d8_sfxHandle);
      x1d8_sfxHandle.Clear();
    }
    if (x140_26_playerTouching) {
      mgr.ApplyDamage(
          GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
          x15c_damageInfo.MakeScaledForTime(dt),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
          CVector3f::Zero());
    }
    x140_26_playerTouching = false;
  }
}

void CSnakeWeedSwarm::Touch(CActor& actor, CStateManager& mgr) {
  if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
    if (projectile->GetCurrentDamageInfo().GetWeaponMode().GetType() != kWT_AI) {
      ScareSnakeWeeds(mgr, projectile->GetTranslation(), x100_weaponDamageRadius);
    }
  }
  if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
    for (AUTO(it, x130_boids.begin()); it != x130_boids.end(); ++it) {
      if ((it->GetLocation() - player->GetTranslation()).Magnitude() < x104_maxPlayerDistance &&
          it->GetBoidState() == kSWBS_Raised) {
        mgr.DeliverScriptMsg(player, kInvalidUniqueId, kSM_InSnakeWeed);
        x140_26_playerTouching = true;
        return;
      }
    }
  }
}

void CSnakeWeedSwarm::ScareSnakeWeeds(CStateManager& mgr, const CVector3f& pos, float radius) {
  const float radiusSquared = radius * radius;
  for (AUTO(it, x130_boids.begin()); it != x130_boids.end(); ++it) {
    CBoid& boid = *it;
    if ((boid.GetLocation() - pos).MagSquared() < radiusSquared &&
        (boid.GetBoidState() == kSWBS_Raised || boid.GetBoidState() == kSWBS_Raising)) {
      boid.SetBoidState(kSWBS_Lowering);
      boid.SetSpeed(x118_speedVariation * mgr.Random()->Float() + x114_speed);
      CSfxManager::AddEmitter(x1d2_sfx2, boid.GetLocation(), CVector3f::Zero(), true, false,
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
  if (!frustum.BoxInFrustumPlanes(x144_touchBounds)) {
    SetPreRenderClipped(true);
  } else {
    SetPreRenderClipped(false);
    for (int i = 0; i < 4; ++i) {
      x1b0_modelData[i]->AnimationData()->PreRender();
    }
    bool buildLights = false;
    if (ActorLights() && x140_24_hasGround) {
      if (GetPreRenderHasMoved()) {
        buildLights = true;
        SetPreRenderHasMoved(false);
      } else if (ActorLights()->GetIsDirty() == true) {
        buildLights = true;
      }
      if (GetCalculateLighting()) {
        const CAABox bounds = x144_touchBounds;
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
  CModelData& data = *x1b0_modelData[modelIndex];
  CAnimData& animData = *data.AnimationData();
  CSkinnedModel& model = data.PickAnimatedModel(x1c4_which);
  const int bit = 1 << modelIndex;
  if (mask & bit) {
    mask &= ~bit;
    animData.BuildPose();
    model.Calculate(animData.GetPose(), rstl::optional_object< CVertexMorphEffect >(), nullptr,
                    x178_posWorkspaces[modelIndex].get());
  }
  gpRender->SetModelMatrix(
      CTransform4f::Translate(boid.GetLocation() - CVector3f(0.f, 0.f, boid.GetDepth())) *
      CTransform4f::Scale(boid.GetSize()));
  model.Draw(x178_posWorkspaces[modelIndex].get(), x19c_nrmWorkspaces[modelIndex],
             CModelFlags(CModelFlags::kT_Opaque, 1.f));
  posesToBuild = mask;
}

void CSnakeWeedSwarm::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (!GetPreRenderClipped()) {
    if (x1ec_particleGen1.get()) {
      gpRender->AddParticleGen(*x1ec_particleGen1);
    }
    if (x1f4_particleGen2.get()) {
      gpRender->AddParticleGen(*x1f4_particleGen2);
    }
    uint posesToBuild = -1;
    if (GetActorLights()) {
      GetActorLights()->ActivateLights();
    } else {
      CGraphics::DisableAllLights();
      gpRender->SetAmbientColor(CColor::White());
    }
    uint index = 0;
    for (AUTO(it, x130_boids.begin()); it != x130_boids.end(); ++it, ++index) {
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
    x130_boids.reserve(GetGridWidth() * GetGridDepth());
    x1c8_boidPositions->reserve(GetGridWidth() * GetGridDepth());
    x1cc_boidPlacement->resize(GetGridWidth() * GetGridDepth(), kBP_None);
    x1c8_boidPositions->push_back(point);
    x140_24_hasGround = true;
  }
}

CVector2i CSnakeWeedSwarm::GetGridPosition(CVector3f pos) {
  const CAABox box = GetBoundingBox();
  return CVector2i(static_cast< int >((pos.GetX() - box.GetMinPoint().GetX()) / xf4_boidSpacing),
                   static_cast< int >((pos.GetY() - box.GetMinPoint().GetY()) / xf4_boidSpacing));
}

int CSnakeWeedSwarm::GetGridWidth() {
  const CAABox box = GetBoundingBox();
  return static_cast< int >((box.GetMaxPoint().GetX() - box.GetMinPoint().GetX()) /
                            xf4_boidSpacing) +
         1;
}

int CSnakeWeedSwarm::GetGridDepth() {
  const CAABox box = GetBoundingBox();
  return static_cast< int >((box.GetMaxPoint().GetY() - box.GetMinPoint().GetY()) /
                            xf4_boidSpacing) +
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
  if ((*x1cc_boidPlacement)[index] == kBP_None) {
    (*x1cc_boidPlacement)[index] = kBP_Ready;
    x1c8_boidPositions->push_back(pos);
  }
}

void CSnakeWeedSwarm::FloodFill(CStateManager& mgr, int count) {
  const int width = GetGridWidth();
  for (int i = 0; i < count && x1c8_boidPositions->size() > 0; ++i) {
    const CVector3f pos = x1c8_boidPositions->back();
    x1c8_boidPositions->pop_back();
    const CVector2i& grid = CVector2i(GetGridPosition(pos));
    if (PlaceSnakeWeedAtPoint(pos, mgr)) {
      (*x1cc_boidPlacement)[grid[0] + width * grid[1]] = kBP_Placed;
      PushBackPoint(CVector3f(pos[kDX], pos[kDY] - xf4_boidSpacing, pos[kDZ]));
      PushBackPoint(CVector3f(pos[kDX], xf4_boidSpacing + pos[kDY], pos[kDZ]));
      PushBackPoint(CVector3f(pos[kDX] - xf4_boidSpacing, pos[kDY], pos[kDZ]));
      PushBackPoint(CVector3f(xf4_boidSpacing + pos[kDX], pos[kDY], pos[kDZ]));
    } else {
      (*x1cc_boidPlacement)[grid[0] + width * grid[1]] = kBP_Invalid;
    }
  }
  UpdateTouchBounds();
  if (x1c8_boidPositions->size() == 0) {
    x1c8_boidPositions = nullptr;
    x1cc_boidPlacement = nullptr;
  }
}

float CSnakeWeedSwarm::GetXVariance(const CVector3f& pos) {
  const float f = 8.21395f * pos.GetX() + 0.112869f * (pos.GetY() * pos.GetY());
  return xfc_variance * (2.f * CMath::AbsF(f - static_cast< int >(f)) - 1.f);
}

float CSnakeWeedSwarm::GetYVariance(const CVector3f& pos) {
  const float f = 2.4729404f * pos.GetY() + 0.3478602f * (pos.GetX() * pos.GetX());
  return xfc_variance * (2.f * CMath::AbsF(f - static_cast< int >(f)) - 1.f);
}

bool CSnakeWeedSwarm::PlaceSnakeWeedAtPoint(const CVector3f& point, CStateManager& mgr) {
  static const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CVector3f pos = point + CVector3f(GetXVariance(point), GetYVariance(point), xf8_height);
  const CVector3f down(0.f, 0.f, -1.f);
  const CRayCastResult result = mgr.RayStaticIntersection(pos, down, 2.f * xf8_height, filter);
  if (result.IsValid() && CVector3f::Dot(result.GetPlane().GetNormal(), CVector3f(0.f, 0.f, 1.f)) >
                              x11c_cosSlopeAngle) {
    const CVector3f boidPos = result.GetPoint() - CVector3f(0.f, 0.f, x128_distanceBelowGround);
    const CBoid boid(boidPos, x110_maxDepth, x114_speed + x118_speedVariation,
                     (x124_scaleMax - x120_scaleMin) * mgr.Random()->Float() + x120_scaleMin);
    x130_boids.push_back(boid);
    return true;
  }
  return false;
}

void CSnakeWeedSwarm::AddContinuousParticles(const CVector3f& pos) {
  if (x1f4_particleGen2.get()) {
    x1f4_particleGen2->SetParticleEmission(true);
    x1f4_particleGen2->SetTranslation(pos);
    x1f4_particleGen2->ForceParticleCreation(1);
    x1f4_particleGen2->SetParticleEmission(false);
  }
}

void CSnakeWeedSwarm::AddRetreatParticles(const CVector3f& pos) {
  if (x1ec_particleGen1.get()) {
    x1ec_particleGen1->SetParticleEmission(true);
    x1ec_particleGen1->SetTranslation(pos);
    x1ec_particleGen1->ForceParticleCreation(x1fc_particleCount);
    x1ec_particleGen1->SetParticleEmission(false);
  }
}

CSnakeWeedSwarm::~CSnakeWeedSwarm() {}
