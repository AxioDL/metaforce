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

static constexpr CMaterialFilter matFilter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});

CSnakeWeedSwarm::CSnakeWeedSwarm(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                                 const zeus::CVector3f& pos, const zeus::CVector3f& scale, const CAnimRes& animRes,
                                 const CActorParameters& actParms, float f1, float f2, float f3, float f4, float f5,
                                 float f6, float f7, float f8, float f9, float f10, float f11, float f12, float f13,
                                 float f14, const CDamageInfo& dInfo, float /*f15*/, u32 sfxId1, u32 sfxId2, u32 sfxId3,
                                 CAssetId w4, u32 w5, CAssetId w6, float f16)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Trigger, EMaterialTypes::NonSolidDamageable), actParms, kInvalidUniqueId)
, xe8_scale(scale)
, xf4_boidSpacing(f1)
, xf8_height(f2)
, xfc_(f3)
, x100_weaponDamageRadius(f4)
, x104_(f5)
, x108_(f6)
, x10c_(f7)
, x110_(f8)
, x114_(f9)
, x118_(f10)
, x11c_(std::cos(zeus::degToRad(f11)))
, x120_(f12)
, x124_(f13)
, x128_(f14)
, x140_24_hasGround(false)
, x140_25_modelAssetDirty(false)
, x140_26_playerTouching(false)
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
    for (int i = 0; i < 4; ++i)
      x1b0_modelData[i]->SetXRayModel(actParms.GetXRayAssets());
    x140_25_modelAssetDirty = true;
  }
  if (actParms.GetThermalAssets().first.IsValid()) {
    for (int i = 0; i < 4; ++i)
      x1b0_modelData[i]->SetInfraModel(actParms.GetThermalAssets());
    x140_25_modelAssetDirty = true;
  }
  if (w4.IsValid()) {
    x1dc_particleGenDesc = g_SimplePool->GetObj({FOURCC('PART'), w4});
    x1ec_particleGen1 = std::make_unique<CElementGen>(x1dc_particleGenDesc);
  }
  if (w6.IsValid()) {
    x1dc_particleGenDesc = g_SimplePool->GetObj({FOURCC('PART'), w6});
    x1f4_particleGen2 = std::make_unique<CElementGen>(x1dc_particleGenDesc);
  }
}

void CSnakeWeedSwarm::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CSnakeWeedSwarm::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  for (int i = 0; i < 4; ++i) {
    const auto& modelData = x1b0_modelData[i];
    modelData->EnableLooping(true);
    modelData->AdvanceAnimation(modelData->GetAnimationData()->GetAnimTimeRemaining("Whole Body"sv) * (i * 0.25f), mgr,
                                x4_areaId, true);
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
    SetCalculateLighting(true);
    return;
  }

  SetCalculateLighting(false);
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

void CSnakeWeedSwarm::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  if (xe4_30_outOfFrustum)
    return;

  if (x1ec_particleGen1)
    g_Renderer->AddParticleGen(*x1ec_particleGen1);
  if (x1f4_particleGen2)
    g_Renderer->AddParticleGen(*x1f4_particleGen2);

  if (x90_actorLights) {
    for (const auto& modelData : x1b0_modelData) {
      x90_actorLights->ActivateLights(*modelData->PickAnimatedModel(x1c4_which).GetModelInst());
    }
  } else {
    CGraphics::DisableAllLights();
    g_Renderer->SetAmbientColor(zeus::skWhite);
  }

  u32 posesToBuild = -1;
  for (u32 i = 0; i < x134_boids.size(); ++i)
    RenderBoid(i, x134_boids[i], posesToBuild);
  CGraphics::DisableAllLights();
}

void CSnakeWeedSwarm::Touch(CActor& actor, CStateManager& mgr) {
  if (TCastToPtr<CGameProjectile> proj = actor) {
    if (proj->GetDamageInfo().GetWeaponMode().GetType() != EWeaponType::AI)
      HandleRadiusDamage(x100_weaponDamageRadius, mgr, proj->GetTransform().origin);
  }
  if (TCastToPtr<CPlayer> player = actor) {
    for (const auto& boid : x134_boids) {
      float m = (boid.GetPosition() - player->GetTransform().origin).magnitude();
      if (m < x104_ && boid.GetState() == EBoidState::Raised) {
        mgr.SendScriptMsg(player, kInvalidUniqueId, EScriptObjectMessage::InSnakeWeed);
        x140_26_playerTouching = true;
      }
    }
  }
}

void CSnakeWeedSwarm::HandleRadiusDamage(float radius, CStateManager& mgr, const zeus::CVector3f& pos) {
  float radiusSquared = radius * radius;
  for (auto& boid : x134_boids) {
    const zeus::CVector3f& boidPosition = boid.GetPosition();
    if ((boidPosition - pos).magSquared() < radiusSquared &&
        (boid.GetState() == EBoidState::Raised || boid.GetState() == EBoidState::Raising)) {
      boid.SetState(EBoidState::x3);
      boid.Set_x18(x118_ * mgr.GetActiveRandom()->Float() + x114_);
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
    EBoidState state = boid.GetState();
    if (state == EBoidState::Raised) {
      raisedBoids++;
      if (x1f4_particleGen2 && emitParticle) {
        EmitParticles2(pos);
      }
    } else if (state == EBoidState::Raising) {
      boid.Set_x14(boid.Get_x14() - dt * boid.Get_x18());
      if (boid.Get_x14() <= 0.f) {
        boid.Set_x14(0.f);
        boid.SetState(EBoidState::Raised);
      }
    } else if (state == EBoidState::x2) {
      boid.Set_x10(boid.Get_x10() - dt);
      if (boid.Get_x10() <= 0.f) {
        boid.SetState(EBoidState::Raising);
        CSfxManager::AddEmitter(x1d4_sfx3, pos, zeus::skZero3f, true, false, 0x7f, x4_areaId);
        EmitParticles1(pos);
      }
    } else if (state == EBoidState::x3) {
      boid.Set_x14(dt * boid.Get_x18() + boid.Get_x14());
      float max = x110_ * boid.Get_x20();
      if (boid.Get_x14() >= max) {
        boid.Set_x14(max);
        float random = mgr.GetActiveRandom()->Float();
        boid.Set_x10(x10c_ * random + x108_);
        boid.SetState(EBoidState::x2);
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
    mgr.ApplyDamage(x8_uid, mgr.GetPlayer().GetUniqueId(), x8_uid, CDamageInfo(x15c_damageInfo, dt), matFilter,
                    zeus::skZero3f);
  }
  x140_26_playerTouching = false;
}

zeus::CAABox CSnakeWeedSwarm::GetBoidBox() {
  const zeus::CVector3f& scale = xe8_scale * 0.75f;
  return zeus::CAABox(GetTransform().origin - scale, GetTransform().origin + scale);
}

void CSnakeWeedSwarm::FindGround(const CStateManager& mgr) {
  const zeus::CAABox& box = GetBoidBox();
  const CRayCastResult& result =
      mgr.RayStaticIntersection(box.center(), zeus::skDown, box.max.z() - box.min.z(), matFilter);
  if (result.IsValid()) {
    int ct = GetNumBoidsX() * GetNumBoidsY();
    x134_boids.reserve(ct);
    x1c8_boidPositions->reserve(ct);
    x1c8_boidPositions->push_back(result.GetPoint());
    x1cc_boidPlacement->resize(ct, EBoidPlacement::None);
    x140_24_hasGround = true;
  }
}

int CSnakeWeedSwarm::GetNumBoidsY() {
  const zeus::CAABox& box = GetBoidBox();
  return (int)((box.max.y() - box.min.y()) / xf4_boidSpacing) + 1;
}

int CSnakeWeedSwarm::GetNumBoidsX() {
  const zeus::CAABox& box = GetBoidBox();
  return (int)((box.max.x() - box.min.x()) / xf4_boidSpacing) + 1;
}

void CSnakeWeedSwarm::CreateBoids(CStateManager& mgr, int num) {
  int width = GetNumBoidsX();
  for (int i = 0; i < num; ++i) {
    if (x1c8_boidPositions->empty())
      break;
    const zeus::CVector3f pos = x1c8_boidPositions->back();
    x1c8_boidPositions->pop_back();
    const zeus::CVector2i& idx = GetBoidIndex(pos);
    if (CreateBoid(pos, mgr)) {
      x1cc_boidPlacement->at(idx.x + width * idx.y) = EBoidPlacement::Placed;
      AddBoidPosition(zeus::CVector3f(pos.x(), pos.y() - xf4_boidSpacing, pos.z()));
      AddBoidPosition(zeus::CVector3f(pos.x(), xf4_boidSpacing + pos.y(), pos.z()));
      AddBoidPosition(zeus::CVector3f(pos.x() - xf4_boidSpacing, pos.y(), pos.z()));
      AddBoidPosition(zeus::CVector3f(xf4_boidSpacing + pos.x(), pos.y(), pos.z()));
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

zeus::CVector2i CSnakeWeedSwarm::GetBoidIndex(const zeus::CVector3f& pos) {
  const zeus::CAABox& box = GetBoidBox();
  int x = (pos.x() - box.min.x()) / xf4_boidSpacing;
  int y = (pos.y() - box.min.y()) / xf4_boidSpacing;
  return zeus::CVector2i(x, y);
}

bool CSnakeWeedSwarm::CreateBoid(const zeus::CVector3f& vec, CStateManager& mgr) {
  const zeus::CVector3f& pos = vec + zeus::CVector3f(GetBoidOffsetX(vec), GetBoidOffsetY(vec), xf8_height);
  const CRayCastResult& result = mgr.RayStaticIntersection(pos, zeus::skDown, 2.f * xf8_height, matFilter);
  if (result.IsValid() && result.GetPlane().normal().dot(zeus::skUp) > x11c_) {
    const zeus::CVector3f& boidPosition = result.GetPoint() - zeus::CVector3f(0.f, 0.f, x128_);
    x134_boids.push_back(
        {boidPosition, x110_, x114_ + x118_, (x124_ - x120_) * mgr.GetActiveRandom()->Float() + x120_});
    return true;
  }
  return false;
}

float CSnakeWeedSwarm::GetBoidOffsetY(const zeus::CVector3f& pos) {
  float f = 2.4729404f * pos.y() + 0.3478602f * pos.x() * pos.x();
  return xfc_ * (2.f * std::abs(f - std::trunc(f)) - 1.f);
}

float CSnakeWeedSwarm::GetBoidOffsetX(const zeus::CVector3f& pos) {
  float f = 8.21395f * pos.x() + 0.112869f * pos.y() * pos.y();
  return xfc_ * (2.f * std::abs(f - std::trunc(f)) - 1.f);
}

void CSnakeWeedSwarm::AddBoidPosition(const zeus::CVector3f& pos) {
  int x = GetNumBoidsX();
  int y = GetNumBoidsY();
  const zeus::CVector2i& v = GetBoidIndex(pos);
  if (-1 < v.x && v.x < x && -1 < v.y && v.y < y) {
    int idx = v.x + x * v.y;
    if (x1cc_boidPlacement->at(idx) == EBoidPlacement::None) {
      x1cc_boidPlacement->at(idx) = EBoidPlacement::Ready;
      x1c8_boidPositions->push_back(pos);
    }
  }
}

void CSnakeWeedSwarm::CalculateTouchBounds() {
  if (x134_boids.empty()) {
    x144_touchBounds = zeus::CAABox(GetTransform().origin, GetTransform().origin);
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

  x1ec_particleGen1->SetParticleEmission(true);
  x1ec_particleGen1->SetTranslation(pos);
  x1ec_particleGen1->ForceParticleCreation(x1fc_);
  x1ec_particleGen1->SetParticleEmission(false);
}

void CSnakeWeedSwarm::EmitParticles2(const zeus::CVector3f& pos) {
  if (!x1f4_particleGen2)
    return;

  x1f4_particleGen2->SetParticleEmission(true);
  x1f4_particleGen2->SetTranslation(pos);
  x1f4_particleGen2->ForceParticleCreation(1);
  x1f4_particleGen2->SetParticleEmission(false);
}

void CSnakeWeedSwarm::RenderBoid(u32 idx, const CBoid& boid, u32& posesToBuild) const {
  const u32 modelIdx = idx & 3;
  const auto& modelData = x1b0_modelData[modelIdx];
  CSkinnedModel& model = modelData->PickAnimatedModel(x1c4_which);
  const CModelFlags useFlags(0, 0, 3, zeus::skWhite);
  if (posesToBuild & 1 << modelIdx) {
    posesToBuild &= ~(1 << modelIdx);
    modelData->GetAnimationData()->BuildPose();
    model.Calculate(modelData->GetAnimationData()->GetPose(), useFlags, {}, nullptr);
  }
  const zeus::CTransform& xf =
      zeus::CTransform::Translate(boid.GetPosition() - zeus::CVector3f(0.f, 0.f, boid.Get_x14())) *
      zeus::CTransform::Scale(boid.Get_x20());
  CGraphics::SetModelMatrix(xf);
  modelData->GetAnimationData()->Render(model, useFlags, {}, nullptr);
}

void CSnakeWeedSwarm::ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& mgr) {
  EWeaponType type = info.GetWeaponMode().GetType();
  if (type == EWeaponType::Bomb || type == EWeaponType::PowerBomb)
    HandleRadiusDamage(info.GetRadius(), mgr, pos);
}

std::optional<zeus::CAABox> CSnakeWeedSwarm::GetTouchBounds() const {
  if (x140_24_hasGround)
    return x144_touchBounds;
  return std::nullopt;
}

} // namespace urde
