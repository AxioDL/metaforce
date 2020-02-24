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

CSnakeWeedSwarm::CSnakeWeedSwarm(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                                 const zeus::CVector3f& pos, const zeus::CVector3f& scale, const CAnimRes& animRes,
                                 const CActorParameters& actParms, float f1, float f2, float f3, float f4, float f5,
                                 float f6, float f7, float f8, float f9, float f10, float f11, float f12, float f13,
                                 float f14, const CDamageInfo& dInfo, float /*f15*/, u32 sfxId1, u32 sfxId2, u32 sfxId3,
                                 CAssetId w4, u32 w5, CAssetId w6, float f16)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Trigger, EMaterialTypes::NonSolidDamageable), actParms, kInvalidUniqueId)
, xe8_scale(scale)
, xf4_(f1)
, xf8_(f2)
, xfc_(f3)
, x100_(f4)
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
, x15c_dInfo(dInfo)
, x1c8_(std::make_unique<std::vector<zeus::CVector3f>>())
, x1cc_(std::make_unique<std::vector<int>>())
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
    x1dc_ = g_SimplePool->GetObj({FOURCC('PART'), w4});
    x1ec_particleGen1 = std::make_unique<CElementGen>(x1dc_);
  }
  if (w6.IsValid()) {
    x1dc_ = g_SimplePool->GetObj({FOURCC('PART'), w6});
    x1f4_particleGen2 = std::make_unique<CElementGen>(x1dc_);
  }
}

void CSnakeWeedSwarm::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CSnakeWeedSwarm::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  //  x178_.clear();
  //  x19c_.clear();
  for (int i = 0; i < 4; ++i) {
    //    CSkinnedModel& skinnedModel = x1b0_modelData[i]->PickAnimatedModel(which);
    //    x178_.push_back(skinnedModel.Clone());
    //    x19c_.push_back(ptr);
    x1b0_modelData[i]->EnableLooping(true);
    x1b0_modelData[i]->AdvanceAnimation(
        // TODO calculation may be wrong
        x1b0_modelData[i]->GetAnimationData()->GetAnimTimeRemaining("Whole Body"sv) * (i * 0.25f), mgr, x4_areaId,
        true);
  }
  x1c4_ = which;
}

void CSnakeWeedSwarm::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId id, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, id, mgr);
  if (msg == EScriptObjectMessage::Deleted) {
    if (x1d8_) {
      CSfxManager::RemoveEmitter(x1d8_);
      x1d8_.reset();
    }
  } else if (msg == EScriptObjectMessage::InitializedInArea) {
    AllocateSkinnedModels(mgr, CModelData::EWhichModel::Normal);
    SetCalculateLighting(true);
    x90_actorLights->SetCastShadows(true);
  }
}
void CSnakeWeedSwarm::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (frustum.aabbFrustumTest(x144_)) {
    SetCalculateLighting(false);
    for (int i = 0; i < 4; ++i) {
      x1b0_modelData[i]->GetAnimationData()->PreRender();
    }
    bool bVar2 = false;
    if (x90_actorLights) {
      if (x140_24_) {
        if (!xe4_29_actorLightsDirty) {
          if (x90_actorLights->GetIsDirty()) {
            bVar2 = true;
          }
        } else {
          bVar2 = true;
          xe4_29_actorLightsDirty = false;
        }
        if (xe4_31_calculateLighting && x90_actorLights) {
          auto visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
          if (visor == CPlayerState::EPlayerVisor::Thermal) {
            x90_actorLights->BuildConstantAmbientLighting(zeus::skWhite);
          } else {
            if (bVar2) {
              if (x4_areaId != kInvalidAreaId) {
                CGameArea* area = mgr.GetWorld()->GetArea(x4_areaId);
                if (area != nullptr) {
                  x90_actorLights->BuildAreaLightList(mgr, *area, x144_);
                }
              }
            }
            x90_actorLights->BuildDynamicLightList(mgr, x144_);
          }
        }
      }
    }
  } else {
    SetCalculateLighting(true);
  }
}

void CSnakeWeedSwarm::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  if (!xe4_30_outOfFrustum) {
    if (x1ec_particleGen1) {
      g_Renderer->AddParticleGen(*x1ec_particleGen1);
    }
    if (x1f4_particleGen2) {
      g_Renderer->AddParticleGen(*x1f4_particleGen2);
    }
    if (x90_actorLights) {
      for (auto modelData : x1b0_modelData) {
        x90_actorLights->ActivateLights(*modelData->PickAnimatedModel(x1c4_).GetModelInst());
      }
    } else {
      CGraphics::DisableAllLights();
      g_Renderer->SetAmbientColor(zeus::skWhite);
    }
    u32 l18 = -1;
    for (u32 i = 0; i < x134_boids.size(); ++i) {
      RenderBoid(i, &x134_boids[i], &l18);
    }
    CGraphics::DisableAllLights();
  }
}

void CSnakeWeedSwarm::Touch(CActor& actor, CStateManager& mgr) {
  if (TCastToPtr<CGameProjectile> proj = actor) {
    if (proj->GetDamageInfo().GetWeaponMode().GetType() != EWeaponType::AI) {
      sub_8023ca48(x100_, mgr, proj->GetTransform().origin);
    }
  }
  if (TCastToPtr<CPlayer> player = actor) {
    for (auto boid : x134_boids) {
      float m = (boid.GetPosition() - player->GetTransform().origin).magnitude();
      if (m < x104_ && boid.GetState() == EBoidState::x0) {
        mgr.SendScriptMsg(player, kInvalidUniqueId, EScriptObjectMessage::InSnakeWeed);
        x140_26_ = true;
      }
    }
  }
}

void CSnakeWeedSwarm::sub_8023ca48(float radius, CStateManager& mgr, const zeus::CVector3f& pos) {
  float dVar8 = radius * radius;
  for (auto boid : x134_boids) {
    const zeus::CVector3f& boidPosition = boid.GetPosition();
    if ((boidPosition - pos).magSquared() < dVar8 &&
        (boid.GetState() == EBoidState::x0 || boid.GetState() == EBoidState::x1)) {
      boid.SetState(EBoidState::x3);
      float random = mgr.GetActiveRandom()->Float();
      boid.Set_x18(x118_ * random + x114_);
      CSfxManager::AddEmitter(x1d2_sfx2, boidPosition, zeus::skZero3f, true, false, 0x7f, x4_areaId);
      EmitParticles1(boidPosition);
    }
  }
}

void CSnakeWeedSwarm::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  if (x30_24_active) {
    if (x1ec_particleGen1) {
      x1ec_particleGen1->Update(dt);
    }
    if (x1f4_particleGen2) {
      x1f4_particleGen2->Update(dt);
    }
    x204_ -= dt;
    bool bVar5 = x204_ < 0.f;
    if (bVar5) {
      x204_ = 0.f;
    }
    x204_ = std::max(x204_ - dt, 0.f);
    if (!x140_24_) {
      sub_8023c238(mgr);
    }
    if (x140_24_ && x1c8_ && !x1c8_->empty()) {
      int v = (u64)(dt * x1cc_->size()) >> 0x20; // TODO ?
      sub_8023bca8(mgr, v + 1);
    }
    CModelData::EWhichModel model = CModelData::GetRenderingModel(mgr);
    if (x140_26_ && x1c4_ != model) {
      AllocateSkinnedModels(mgr, model);
    }
    for (int i = 0; i < 4; ++i) {
      x1b0_modelData[i]->GetAnimationData()->SetPlaybackRate(1.f);
      x1b0_modelData[i]->AdvanceAnimation(dt, mgr, x4_areaId, true);
    }
    int numBoidsInx0 = 0;
    for (auto boid : x134_boids) {
      const zeus::CVector3f& pos = boid.GetPosition();
      EBoidState state = boid.GetState();
      if (state == EBoidState::x0) {
        numBoidsInx0++;
        if (x1f4_particleGen2 && bVar5) {
          EmitParticles2(pos);
        }
      } else if (state == EBoidState::x1) {
        float x14 = boid.Get_x14() - dt * boid.Get_x18();
        boid.Set_x14(x14);
        //        printf("setting x14 to %f (dt %f)\n", x14, dt * boid.Get_x18());
        if (x14 <= 0.f) {
          boid.Set_x14(0.f);
          boid.SetState(EBoidState::x0);
        }
      } else if (state == EBoidState::x2) {
        float x10 = boid.Get_x10() - dt;
        boid.Set_x10(x10);
        if (x10 <= 0.f) {
          boid.SetState(EBoidState::x1);
          CSfxManager::AddEmitter(x1d4_sfx3, pos, zeus::skZero3f, true, false, 0x7f, x4_areaId);
          EmitParticles1(pos);
        }
      } else if (state == EBoidState::x3) {
        float x14 = dt * boid.Get_x18() + boid.Get_x14();
        boid.Set_x14(x14);
        float dVar8 = x110_ * boid.Get_x20();
        if (dVar8 < x14) {
          boid.Set_x14(dVar8);
          float random = mgr.GetActiveRandom()->Float();
          boid.Set_x10(x10c_ * random + x108_);
          boid.SetState(EBoidState::x2);
        }
      }
    }
    if (numBoidsInx0 == 0) {
      if (x1d8_) {
        CSfxManager::RemoveEmitter(x1d8_);
        x1d8_.reset();
      }
    } else {
      if (!x1d8_) {
        x1d8_ = CSfxManager::AddEmitter(x1d0_sfx1, GetTransform().origin, zeus::skZero3f, true, true, 0x7f, x4_areaId);
      }
    }
    if (x140_26_) {
      mgr.ApplyDamage(x8_uid, mgr.GetPlayer().GetUniqueId(), x8_uid, CDamageInfo(x15c_dInfo, dt),
                      CMaterialFilter::MakeInclude(CMaterialList(EMaterialTypes::Player, EMaterialTypes::Stone)),
                      zeus::skZero3f);
    }
    x140_26_ = false;
  }
}

zeus::CAABox CSnakeWeedSwarm::sub_8023d3f4() {
  const zeus::CVector3f& scale = xe8_scale * 0.75f;
  const zeus::CVector3f& min = GetTransform().origin - scale;
  const zeus::CVector3f& max = GetTransform().origin + scale;
  return zeus::CAABox(min, max);
}

static CMaterialList matList1 = CMaterialList(EMaterialTypes::RadarObject, EMaterialTypes::Stone);
static CMaterialFilter matFilter1 = CMaterialFilter::MakeInclude(matList1);

void CSnakeWeedSwarm::sub_8023c238(const CStateManager& mgr) {
  const zeus::CAABox& box = sub_8023d3f4();
  const CRayCastResult& result =
      mgr.RayStaticIntersection(box.center(), zeus::skDown, box.max.z() - box.min.z(), matFilter1);
  if (result.IsValid()) {
    int ct = sub_8023c154() * sub_8023c0fc();
    x134_boids.reserve(ct);
    x1c8_->reserve(ct);
    x1cc_->resize(ct, 0);
    x1c8_->push_back(result.GetPoint());
    x140_24_ = true;
  }

  //  matFilter.IncludeList().Add(CMaterialList(matFilter.IncludeList().GetValue() >> 0x20));
}

int CSnakeWeedSwarm::sub_8023c0fc() {
  const zeus::CAABox& box = sub_8023d3f4();
  return (int)((box.max.y() - box.min.y()) / xf4_) + 1;
}

int CSnakeWeedSwarm::sub_8023c154() {
  const zeus::CAABox& box = sub_8023d3f4();
  return (int)((box.max.x() - box.min.x()) / xf4_) + 1;
}

void CSnakeWeedSwarm::sub_8023bca8(CStateManager& mgr, int v) {
  int n = sub_8023c154();
  for (int i = 0; i < v; ++i) {
    if (x1c8_->empty())
      break;
    zeus::CVector3f vec = x1c8_->back();
    x1c8_->pop_back();
    const zeus::CVector2i& v2i = sub_8023c1ac(vec);
    if (CreateBoid(vec, mgr)) {
      x1cc_->at(v2i.x + n * v2i.y) = 3;
      sub_8023bfb8(zeus::CVector3f(vec.x(), vec.y() - xf4_, vec.z()));
      sub_8023bfb8(zeus::CVector3f(vec.x(), xf4_ + vec.y(), vec.z()));
      sub_8023bfb8(zeus::CVector3f(vec.x() - xf4_, vec.y(), vec.z()));
      sub_8023bfb8(zeus::CVector3f(xf4_ + vec.x(), vec.y(), vec.z()));
    } else {
      x1cc_->at(v2i.x + n * v2i.y) = 2;
    }
  }
  sub_8023d204();
  if (x1c8_->empty()) {
    x1c8_ = nullptr;
    x1cc_ = nullptr;
  }
}

zeus::CVector2i CSnakeWeedSwarm::sub_8023c1ac(const zeus::CVector3f& vec) {
  const zeus::CAABox& box = sub_8023d3f4();
  int x = (vec.x() - box.min.x()) / xf4_;
  int y = (vec.y() - box.min.y()) / xf4_;
  return zeus::CVector2i(x, y);
}

// TODO double check these params
static CMaterialList matList2 = CMaterialList(EMaterialTypes::RadarObject, EMaterialTypes::Stone);
static CMaterialFilter matFilter2 = CMaterialFilter::MakeInclude(matList2);

bool CSnakeWeedSwarm::CreateBoid(const zeus::CVector3f& vec, CStateManager& mgr) {
  const zeus::CVector3f pos = vec + zeus::CVector3f(sub_8023bc38(vec), sub_8023bbc8(vec), xf8_);
  const CRayCastResult result = mgr.RayStaticIntersection(pos, zeus::skDown, 2.f * xf8_, matFilter2);
  if (result.IsInvalid() || result.GetPlane().normal().dot(zeus::skUp) <= x11c_) {
    return false;
  } else {
    const zeus::CVector3f boidPosition = result.GetPoint() - zeus::CVector3f(0.f, 0.f, x128_);
    float random = mgr.GetActiveRandom()->Float();
    const CBoid boid(boidPosition, x110_, x114_ + x118_, (x124_ - x120_) * random + x120_);
    x134_boids.push_back(boid);
//    printf("created boid @ %f|%f|%f, new size %zu\n", boidPosition.x(), boidPosition.y(), boidPosition.z(),
//           x134_boids.size());
    return true;
  }
}

float CSnakeWeedSwarm::sub_8023bbc8(const zeus::CVector3f& vec) {
  float f = 2.4729404f * vec.y() + 0.3478602f * vec.x() * vec.x();
  return xfc_ * (2.f * std::abs(f - (int)f) - 1.f);
}

float CSnakeWeedSwarm::sub_8023bc38(const zeus::CVector3f& vec) {
  float f = 8.21395f * vec.x() + 0.112869f * vec.y() * vec.y();
  return xfc_ * (2.f * std::abs(f - (int)f) - 1.f);
}

void CSnakeWeedSwarm::sub_8023bfb8(const zeus::CVector3f& vec) {
  int x = sub_8023c154();
  int y = sub_8023c0fc();
  zeus::CVector2i v = sub_8023c1ac(vec);
  if (-1 < v.x && v.x < x && -1 < v.y && v.y < y) {
    v.x += x * v.y;
    if (x1cc_->at(v.x) == 0) {
      x1cc_->at(v.x) = 1;
      x1c8_->push_back(vec);
    }
  }
}

void CSnakeWeedSwarm::sub_8023d204() {
  if (x134_boids.empty()) {
    x144_ = zeus::CAABox(GetTransform().origin, GetTransform().origin);
  } else {
    x144_ = zeus::skInvertedBox;
    for (auto boid : x134_boids) {
      x144_.accumulateBounds(boid.GetPosition() - x100_);
      x144_.accumulateBounds(boid.GetPosition() + x100_);
    }
  }
  xe4_27_notInSortedLists = true;
}

void CSnakeWeedSwarm::EmitParticles1(const zeus::CVector3f& pos) {
  if (x1ec_particleGen1) {
    x1ec_particleGen1->SetParticleEmission(true);
    x1ec_particleGen1->SetTranslation(pos);
    x1ec_particleGen1->ForceParticleCreation(x1fc_);
    x1ec_particleGen1->SetParticleEmission(false);
  }
}

void CSnakeWeedSwarm::EmitParticles2(const zeus::CVector3f& pos) {
  if (x1f4_particleGen2) {
    x1f4_particleGen2->SetParticleEmission(true);
    x1f4_particleGen2->SetTranslation(pos);
    x1f4_particleGen2->ForceParticleCreation(1);
    x1f4_particleGen2->SetParticleEmission(false);
  }
}

void CSnakeWeedSwarm::RenderBoid(u32 p1, const CBoid* boid, u32* p3) const {
  u32 idx = p1 & 3;
  u32 var3 = *p3;
  auto modelData = x1b0_modelData[idx];
  CSkinnedModel& model = modelData->PickAnimatedModel(x1c4_);
  CModelFlags useFlags(0, 0, 3, zeus::skWhite);
  if ((var3 & 1 << idx) != 0) {
    var3 &= ~(1 << idx);
    modelData->GetAnimationData()->BuildPose();
    model.Calculate(modelData->GetAnimationData()->GetPose(), useFlags, {}, nullptr);
  }
  //  printf("rendering boid with x14 %f\n", boid->Get_x14());
  const zeus::CTransform& xf =
      zeus::CTransform::Translate(boid->GetPosition() /*- zeus::CVector3f(0.f, 0.f, boid->Get_x14())*/) *
      zeus::CTransform::Scale(boid->Get_x20());
  g_Renderer->SetModelMatrix(xf);
  model.Draw(useFlags);
  *p3 = var3;
}

void CSnakeWeedSwarm::ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& mgr) {
  EWeaponType type = info.GetWeaponMode().GetType();
  if (type == EWeaponType::Bomb || type == EWeaponType::PowerBomb) {
    sub_8023ca48(info.GetRadius(), mgr, pos);
  }
}

} // namespace urde
