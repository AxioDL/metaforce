#include "Runtime/World/CSnakeWeedSwarm.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CAnimationParameters.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

static constexpr CMaterialFilter skMaterialFilter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});

CSnakeWeedSwarm::CSnakeWeedSwarm(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                                 const zeus::CVector3f& pos, const zeus::CVector3f& scale, const CAnimRes& animRes,
                                 const CActorParameters& actParms, float spacing, float height, float f3,
                                 float weaponDamageRadius, float maxPlayerDistance, float loweredTime,
                                 float loweredTimeVariation, float maxZOffset, float speed, float speedVariation,
                                 float f11, float scaleMin, float scaleMax, float distanceBelowGround,
                                 const CDamageInfo& dInfo, float /*f15*/, u32 sfxId1, u32 sfxId2, u32 sfxId3,
                                 CAssetId particleGenDesc1, u32 w5, CAssetId particleGenDesc2, float f16)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Trigger, EMaterialTypes::NonSolidDamageable), actParms, kInvalidUniqueId)
, xe8_scale(scale)
, xf4_boidSpacing(spacing)
, xf8_height(height)
, xfc_(f3)
, x100_weaponDamageRadius(weaponDamageRadius)
, x104_maxPlayerDistance(maxPlayerDistance)
, x108_loweredTime(loweredTime)
, x10c_loweredTimeVariation(loweredTimeVariation)
, x110_maxZOffset(maxZOffset)
, x114_speed(speed)
, x118_speedVariation(speedVariation)
, x11c_(std::cos(zeus::degToRad(f11)))
, x120_scaleMin(scaleMin)
, x124_scaleMax(scaleMax)
, x128_distanceBelowGround(distanceBelowGround)
, x15c_damageInfo(dInfo)
, x1c8_boidPositions(std::make_unique<std::vector<zeus::CVector3f>>())
, x1cc_boidPlacement(std::make_unique<std::vector<EBoidPlacement>>())
, x1d0_sfx1(CSfxManager::TranslateSFXID(sfxId1))
, x1d2_sfx2(CSfxManager::TranslateSFXID(sfxId2))
, x1d4_sfx3(CSfxManager::TranslateSFXID(sfxId3))
, x1fc_(w5)
, x200_(f16) {
  SetActorLights(actParms.GetLightParameters().MakeActorLights());
  x1b0_modelData.emplace_back(std::make_unique<CModelData>(animRes));
  x1b0_modelData.emplace_back(std::make_unique<CModelData>(animRes));
  x1b0_modelData.emplace_back(std::make_unique<CModelData>(animRes));
  x1b0_modelData.emplace_back(std::make_unique<CModelData>(animRes));
  if (actParms.GetXRayAssets().first.IsValid()) {
    for (const auto& modelData : x1b0_modelData) {
      modelData->SetXRayModel(actParms.GetXRayAssets());
    }
    x140_25_modelAssetDirty = true;
  }
  if (actParms.GetThermalAssets().first.IsValid()) {
    for (const auto& modelData : x1b0_modelData) {
      modelData->SetInfraModel(actParms.GetThermalAssets());
    }
    x140_25_modelAssetDirty = true;
  }
  if (particleGenDesc1.IsValid()) {
    x1dc_particleGenDesc = g_SimplePool->GetObj({FOURCC('PART'), particleGenDesc1});
    x1ec_particleGen1 = std::make_unique<CElementGen>(x1dc_particleGenDesc);
  }
  if (particleGenDesc2.IsValid()) {
    x1dc_particleGenDesc = g_SimplePool->GetObj({FOURCC('PART'), particleGenDesc2});
    x1f4_particleGen2 = std::make_unique<CElementGen>(x1dc_particleGenDesc);
  }
}

void CSnakeWeedSwarm::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CSnakeWeedSwarm::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  for (int i = 0; i < x1b0_modelData.size(); ++i) {
    auto& modelData = *x1b0_modelData[i];
    modelData.EnableLooping(true);
    const float dt = modelData.GetAnimationData()->GetAnimTimeRemaining("Whole Body"sv) * (i * 0.25f);
    modelData.AdvanceAnimation(dt, mgr, x4_areaId, true);
  }
  x1c4_which = which;
}

void CSnakeWeedSwarm::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId id, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, id, mgr);
  if (msg == EScriptObjectMessage::Deleted) {
    if (x1d8_sfxHandle) {
      CSfxManager::RemoveEmitter(x1d8_sfxHandle);
      x1d8_sfxHandle.reset();
    }
  } else if (msg == EScriptObjectMessage::InitializedInArea) {
    AllocateSkinnedModels(mgr, CModelData::EWhichModel::Normal);
    SetCalculateLighting(true);
    x90_actorLights->SetCastShadows(true);
  }
}

void CSnakeWeedSwarm::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (!frustum.aabbFrustumTest(x144_touchBounds)) {
    xe4_30_outOfFrustum = true;
    return;
  }

  xe4_30_outOfFrustum = false;
  for (const auto& modelData : x1b0_modelData) {
    modelData->GetAnimationData()->PreRender();
  }
  if (x90_actorLights && x140_24_hasGround) {
    bool buildLights = false;
    if (xe4_29_actorLightsDirty) {
      buildLights = true;
      xe4_29_actorLightsDirty = false;
    } else if (x90_actorLights->GetIsDirty()) {
      buildLights = true;
    }
    if (xe4_31_calculateLighting && x90_actorLights) {
      auto visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
      if (visor == CPlayerState::EPlayerVisor::Thermal) {
        x90_actorLights->BuildConstantAmbientLighting(zeus::skWhite);
      } else if (buildLights && x4_areaId != kInvalidAreaId) {
        CGameArea* area = mgr.GetWorld()->GetArea(x4_areaId);
        if (area != nullptr) {
          x90_actorLights->BuildAreaLightList(mgr, *area, x144_touchBounds);
        }
      }
      x90_actorLights->BuildDynamicLightList(mgr, x144_touchBounds);
    }
  }
}

void CSnakeWeedSwarm::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (xe4_30_outOfFrustum) {
    return;
  }

  if (x1ec_particleGen1) {
    g_Renderer->AddParticleGen(*x1ec_particleGen1);
  }
  if (x1f4_particleGen2) {
    g_Renderer->AddParticleGen(*x1f4_particleGen2);
  }

  if (x90_actorLights) {
    for (const auto& modelData : x1b0_modelData) {
      x90_actorLights->ActivateLights(*modelData->PickAnimatedModel(x1c4_which).GetModelInst());
    }
  } else {
    CGraphics::DisableAllLights();
    CGraphics::SetAmbientColor(zeus::skWhite);
  }

  u32 posesToBuild = -1;
  for (u32 i = 0; i < x134_boids.size(); ++i) {
    RenderBoid(i, x134_boids[i], posesToBuild);
  }
  CGraphics::DisableAllLights();
}

void CSnakeWeedSwarm::Touch(CActor& actor, CStateManager& mgr) {
  if (TCastToPtr<CGameProjectile> proj = actor) {
    if (proj->GetDamageInfo().GetWeaponMode().GetType() != EWeaponType::AI) {
      HandleRadiusDamage(x100_weaponDamageRadius, mgr, proj->GetTransform().origin);
    }
  }
  if (TCastToPtr<CPlayer> player = actor) {
    for (const auto& boid : x134_boids) {
      float m = (boid.GetPosition() - player->GetTransform().origin).magnitude();
      if (m < x104_maxPlayerDistance && boid.GetState() == EBoidState::Raised) {
        mgr.SendScriptMsg(player, kInvalidUniqueId, EScriptObjectMessage::InSnakeWeed);
        x140_26_playerTouching = true;
      }
    }
  }
}

void CSnakeWeedSwarm::HandleRadiusDamage(float radius, CStateManager& mgr, const zeus::CVector3f& pos) {
  float radiusSquared = radius * radius;
  for (auto& boid : x134_boids) {
    const auto& boidPosition = boid.GetPosition();
    if ((boidPosition - pos).magSquared() < radiusSquared &&
        (boid.GetState() == EBoidState::Raised || boid.GetState() == EBoidState::Raising)) {
      boid.SetState(EBoidState::Lowering);
      boid.SetSpeed(x118_speedVariation * mgr.GetActiveRandom()->Float() + x114_speed);
      CSfxManager::AddEmitter(x1d2_sfx2, boidPosition, zeus::skZero3f, true, false, 0x7f, x4_areaId);
      EmitParticles1(boidPosition);
    }
  }
}

void CSnakeWeedSwarm::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  if (!x30_24_active)
    return;

  if (x1ec_particleGen1)
    x1ec_particleGen1->Update(dt);
  if (x1f4_particleGen2)
    x1f4_particleGen2->Update(dt);

  x204_particleTimer -= dt;
  bool emitParticle = false;
  if (x204_particleTimer < 0.f) {
    x204_particleTimer = 0.f;
    emitParticle = true;
  }

  if (!x140_24_hasGround)
    FindGround(mgr);
  if (x140_24_hasGround && x1c8_boidPositions && !x1c8_boidPositions->empty()) {
    int n = (u64)(dt * x1cc_boidPlacement->size()) >> 0x20;
    CreateBoids(mgr, n + 1);
  }

  CModelData::EWhichModel model = CModelData::GetRenderingModel(mgr);
  if (x140_25_modelAssetDirty && x1c4_which != model)
    AllocateSkinnedModels(mgr, model);

  for (const auto& modelData : x1b0_modelData) {
    modelData->GetAnimationData()->SetPlaybackRate(1.f);
    modelData->AdvanceAnimation(dt, mgr, x4_areaId, true);
  }

  int raisedBoids = 0;
  for (auto& boid : x134_boids) {
    const zeus::CVector3f& pos = boid.GetPosition();
    const EBoidState state = boid.GetState();
    if (state == EBoidState::Raised) {
      raisedBoids++;
      if (x1f4_particleGen2 && emitParticle) {
        EmitParticles2(pos);
      }
    } else if (state == EBoidState::Raising) {
      boid.SetZOffset(boid.GetZOffset() - dt * boid.GetSpeed());
      if (boid.GetZOffset() <= 0.f) {
        boid.SetZOffset(0.f);
        boid.SetState(EBoidState::Raised);
      }
    } else if (state == EBoidState::Lowered) {
      boid.SetLoweredTimer(boid.GetLoweredTimer() - dt);
      if (boid.GetLoweredTimer() <= 0.f) {
        boid.SetState(EBoidState::Raising);
        CSfxManager::AddEmitter(x1d4_sfx3, pos, zeus::skZero3f, true, false, 0x7f, x4_areaId);
        EmitParticles1(pos);
      }
    } else if (state == EBoidState::Lowering) {
      boid.SetZOffset(boid.GetZOffset() + dt * boid.GetSpeed());
      const float max = x110_maxZOffset * boid.GetScale();
      if (boid.GetZOffset() >= max) {
        boid.SetZOffset(max);
        boid.SetLoweredTimer(x10c_loweredTimeVariation * mgr.GetActiveRandom()->Float() + x108_loweredTime);
        boid.SetState(EBoidState::Lowered);
      }
    }
  }
  if (raisedBoids == 0) {
    if (x1d8_sfxHandle) {
      CSfxManager::RemoveEmitter(x1d8_sfxHandle);
      x1d8_sfxHandle.reset();
    }
  } else if (!x1d8_sfxHandle) {
    x1d8_sfxHandle =
        CSfxManager::AddEmitter(x1d0_sfx1, GetTransform().origin, zeus::skZero3f, true, true, 0x7f, x4_areaId);
  }

  if (x140_26_playerTouching) {
    mgr.ApplyDamage(x8_uid, mgr.GetPlayer().GetUniqueId(), x8_uid, {x15c_damageInfo, dt}, skMaterialFilter,
                    zeus::skZero3f);
  }
  x140_26_playerTouching = false;
}

zeus::CAABox CSnakeWeedSwarm::GetBoidBox() const {
  const auto scale = xe8_scale * 0.75f;
  return {GetTransform().origin - scale, GetTransform().origin + scale};
}

void CSnakeWeedSwarm::FindGround(const CStateManager& mgr) {
  const auto box = GetBoidBox();
  const auto result =
      mgr.RayStaticIntersection(box.center(), zeus::skDown, box.max.z() - box.min.z(), skMaterialFilter);
  if (result.IsValid()) {
    int ct = GetNumBoidsX() * GetNumBoidsY();
    x134_boids.reserve(ct);
    x1c8_boidPositions->reserve(ct);
    x1c8_boidPositions->push_back(result.GetPoint());
    x1cc_boidPlacement->resize(ct, EBoidPlacement::None);
    x140_24_hasGround = true;
  }
}

int CSnakeWeedSwarm::GetNumBoidsY() const {
  const auto box = GetBoidBox();
  return static_cast<int>((box.max.y() - box.min.y()) / xf4_boidSpacing) + 1;
}

int CSnakeWeedSwarm::GetNumBoidsX() const {
  const auto box = GetBoidBox();
  return static_cast<int>((box.max.x() - box.min.x()) / xf4_boidSpacing) + 1;
}

void CSnakeWeedSwarm::CreateBoids(CStateManager& mgr, int num) {
  auto width = GetNumBoidsX();
  for (int i = 0; i < num; ++i) {
    if (x1c8_boidPositions->empty())
      break;
    const auto pos = x1c8_boidPositions->back();
    x1c8_boidPositions->pop_back();
    const auto& idx = GetBoidIndex(pos);
    if (CreateBoid(pos, mgr)) {
      x1cc_boidPlacement->at(idx.x + width * idx.y) = EBoidPlacement::Placed;
      AddBoidPosition({pos.x(), pos.y() - xf4_boidSpacing, pos.z()});
      AddBoidPosition({pos.x(), xf4_boidSpacing + pos.y(), pos.z()});
      AddBoidPosition({pos.x() - xf4_boidSpacing, pos.y(), pos.z()});
      AddBoidPosition({xf4_boidSpacing + pos.x(), pos.y(), pos.z()});
    } else {
      x1cc_boidPlacement->at(idx.x + width * idx.y) = EBoidPlacement::Invalid;
    }
  }
  CalculateTouchBounds();
  if (x1c8_boidPositions->empty()) {
    x1c8_boidPositions = nullptr;
    x1cc_boidPlacement = nullptr;
  }
}

zeus::CVector2i CSnakeWeedSwarm::GetBoidIndex(const zeus::CVector3f& pos) const {
  const auto box = GetBoidBox();
  return {static_cast<int>((pos.x() - box.min.x()) / xf4_boidSpacing),
          static_cast<int>((pos.y() - box.min.y()) / xf4_boidSpacing)};
}

bool CSnakeWeedSwarm::CreateBoid(const zeus::CVector3f& vec, CStateManager& mgr) {
  const auto pos = vec + zeus::CVector3f(GetBoidOffsetX(vec), GetBoidOffsetY(vec), xf8_height);
  const auto result = mgr.RayStaticIntersection(pos, zeus::skDown, 2.f * xf8_height, skMaterialFilter);
  if (result.IsValid() && result.GetPlane().normal().dot(zeus::skUp) > x11c_) {
    const auto boidPosition = result.GetPoint() - zeus::CVector3f(0.f, 0.f, x128_distanceBelowGround);
    x134_boids.emplace_back(boidPosition, x110_maxZOffset, x114_speed + x118_speedVariation,
                            (x124_scaleMax - x120_scaleMin) * mgr.GetActiveRandom()->Float() + x120_scaleMin);
    return true;
  }
  return false;
}

float CSnakeWeedSwarm::GetBoidOffsetY(const zeus::CVector3f& pos) const {
  const float f = 2.4729404f * pos.y() + 0.3478602f * pos.x() * pos.x();
  return xfc_ * (2.f * std::abs(f - std::trunc(f)) - 1.f);
}

float CSnakeWeedSwarm::GetBoidOffsetX(const zeus::CVector3f& pos) const {
  const float f = 8.21395f * pos.x() + 0.112869f * pos.y() * pos.y();
  return xfc_ * (2.f * std::abs(f - std::trunc(f)) - 1.f);
}

void CSnakeWeedSwarm::AddBoidPosition(const zeus::CVector3f& pos) {
  int x = GetNumBoidsX();
  int y = GetNumBoidsY();
  const auto& v = GetBoidIndex(pos);
  if (-1 < v.x && v.x < x && -1 < v.y && v.y < y) {
    int idx = v.x + x * v.y;
    auto& placement = x1cc_boidPlacement->at(idx);
    if (placement == EBoidPlacement::None) {
      placement = EBoidPlacement::Ready;
      x1c8_boidPositions->push_back(pos);
    }
  }
}

void CSnakeWeedSwarm::CalculateTouchBounds() {
  if (x134_boids.empty()) {
    x144_touchBounds = {x34_transform.origin, x34_transform.origin};
  } else {
    x144_touchBounds = zeus::skInvertedBox;
    for (const auto& boid : x134_boids) {
      x144_touchBounds.accumulateBounds(boid.GetPosition() - x100_weaponDamageRadius);
      x144_touchBounds.accumulateBounds(boid.GetPosition() + x100_weaponDamageRadius);
    }
  }
  xe4_27_notInSortedLists = true;
}

void CSnakeWeedSwarm::EmitParticles1(const zeus::CVector3f& pos) {
  if (!x1ec_particleGen1)
    return;

  auto& particleGen = *x1ec_particleGen1;
  particleGen.SetParticleEmission(true);
  particleGen.SetTranslation(pos);
  particleGen.ForceParticleCreation(x1fc_);
  particleGen.SetParticleEmission(false);
}

void CSnakeWeedSwarm::EmitParticles2(const zeus::CVector3f& pos) {
  if (!x1f4_particleGen2)
    return;

  auto& particleGen = *x1f4_particleGen2;
  particleGen.SetParticleEmission(true);
  particleGen.SetTranslation(pos);
  particleGen.ForceParticleCreation(1);
  particleGen.SetParticleEmission(false);
}

void CSnakeWeedSwarm::RenderBoid(u32 idx, const CBoid& boid, u32& posesToBuild) const {
  const u32 modelIdx = idx & 3;
  auto& modelData = *x1b0_modelData[modelIdx];
  auto& model = modelData.PickAnimatedModel(x1c4_which);
  auto& animData = *modelData.GetAnimationData();
  constexpr CModelFlags useFlags(0, 0, 3, zeus::skWhite);
  if (posesToBuild & 1 << modelIdx) {
    posesToBuild &= ~(1 << modelIdx);
    animData.BuildPose();
    model.Calculate(animData.GetPose(), useFlags, std::nullopt, nullptr);
  }
  CGraphics::SetModelMatrix(
      zeus::CTransform::Translate(boid.GetPosition() - zeus::CVector3f(0.f, 0.f, boid.GetZOffset())) *
      zeus::CTransform::Scale(boid.GetScale()));
  animData.Render(model, useFlags, std::nullopt, nullptr);
}

void CSnakeWeedSwarm::ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& mgr) {
  auto type = info.GetWeaponMode().GetType();
  if (type == EWeaponType::Bomb || type == EWeaponType::PowerBomb)
    HandleRadiusDamage(info.GetRadius(), mgr, pos);
}

std::optional<zeus::CAABox> CSnakeWeedSwarm::GetTouchBounds() const {
  if (x140_24_hasGround)
    return x144_touchBounds;
  return std::nullopt;
}

} // namespace urde
