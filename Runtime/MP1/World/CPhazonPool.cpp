#include "Runtime/MP1/World/CPhazonPool.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CPhazonPool::CPhazonPool(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         const zeus::CVector3f& scale, bool active, CAssetId w1, CAssetId w2, CAssetId w3, CAssetId w4,
                         u32 p11, const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                         ETriggerFlags triggerFlags, bool p15, float p16, float p17, float p18, float p19)
: CScriptTrigger(uid, name, info, xf.origin, zeus::skNullBox, dInfo, orientedForce, triggerFlags, active, false, false)
, x168_modelData1(std::make_unique<CModelData>(CStaticRes{w1, zeus::skOne3f}))
, x16c_modelData2(std::make_unique<CModelData>(CStaticRes{w2, zeus::skOne3f}))
, x190_scale(scale)
, x19c_(p16)
, x1a0_(p16)
, x1bc_(p17)
, x1c0_(p19)
, x1c8_(p18)
, x1d8_(p11)
, x1e0_24_(p15) {
  if (w3.IsValid()) {
    x170_elementGen1 = std::make_unique<CElementGen>(g_SimplePool->GetObj(SObjectTag{SBIG('PART'), w3}));
    x170_elementGen1->SetParticleEmission(false);
  }
  if (w4.IsValid()) {
    x174_elementGen2 = std::make_unique<CElementGen>(g_SimplePool->GetObj(SObjectTag{SBIG('PART'), w4}));
    x174_elementGen2->SetGlobalScale(x190_scale);
    x174_elementGen2->SetParticleEmission(false);
  }
}

void CPhazonPool::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CPhazonPool::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate:
  case EScriptObjectMessage::Open:
    UpdateParticleGens(mgr);
    break;
  case EScriptObjectMessage::Close:
    if (!x1e0_25_) {
      x1e0_25_ = true;
      x1c4_ = 0.f;
      SetEmitParticles(false);
    }
    break;
  case EScriptObjectMessage::Decrement:
    if (x1dc_ == 2) {
      x1cc_ += 1.f;
    }
    break;
  case EScriptObjectMessage::Registered:
    if (x170_elementGen1) {
      x170_elementGen1->SetGlobalTranslation(GetTranslation());
    }
    if (x168_modelData1) {
      x178_bounds = x168_modelData1->GetBounds();
    } else {
      x178_bounds = zeus::CAABox{0.5f * -x190_scale, 0.5f * x190_scale};
    }
    x130_bounds = zeus::CAABox{x178_bounds.min * x190_scale, x178_bounds.max * x190_scale};
    break;
  case EScriptObjectMessage::Deleted:
    RemoveInhabitants(mgr);
    break;
  default:
    break;
  }
  CScriptTrigger::AcceptScriptMsg(msg, uid, mgr);
}

void CPhazonPool::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (GetActive()) {
    if (x170_elementGen1) {
      g_Renderer->AddParticleGen(*x170_elementGen1);
    }
    if (x174_elementGen2) {
      g_Renderer->AddParticleGen(*x174_elementGen2);
    }
  }
  CActor::AddToRenderer(frustum, mgr);
  CActor::EnsureRendered(mgr);
}

std::optional<zeus::CAABox> CPhazonPool::GetTouchBounds() const { return CScriptTrigger::GetTouchBounds(); }

void CPhazonPool::Render(CStateManager& mgr) {
  CActor::Render(mgr);
  bool discard = x1a4_ < 0.25f;
  const CModelFlags flags{5, 0, static_cast<u16>(discard ? 3 : 0), zeus::CColor{1.f, x1a4_}};
  if (x168_modelData1) {
    x168_modelData1->Render(mgr, GetTransform() * zeus::CTransform::RotateZ(x1ac_rotZ), nullptr, flags);
  }
  if (x16c_modelData2) {
    const zeus::CTransform rot = zeus::CTransform::RotateZ(x1ac_rotZ) * zeus::CTransform::RotateX(x1ac_rotZ) *
                                 zeus::CTransform::RotateY(x1a8_rotY);
    x16c_modelData2->Render(mgr, GetTransform() * rot, nullptr, flags);
  }
}

void CPhazonPool::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CScriptTrigger::Think(dt, mgr);
  UpdateInhabitants(mgr);
  const zeus::CVector3f scale = (x1dc_ == 1 ? x1a4_ : 1.f) * x190_scale;
  SetTransform({zeus::CMatrix3f{scale}, GetTranslation()});
  x130_bounds = zeus::CAABox{x178_bounds.min * scale, x178_bounds.max * scale};
  x1a8_rotY += dt * x1b0_;
  x1ac_rotZ += dt * x1b4_;
  if (x1a8_rotY > zeus::degToRad(360.f)) {
    x1a8_rotY = 0.f;
  }
  if (x1ac_rotZ > zeus::degToRad(360.f)) {
    x1ac_rotZ = 0.f;
  }
  if (x170_elementGen1) {
    x170_elementGen1->SetModulationColor(zeus::CColor{x1a4_, x1a4_, x1a4_, x1a4_});
    x170_elementGen1->SetGlobalScale(zeus::CVector3f{scale.x()});
    x170_elementGen1->Update(dt);
  }
  if (x174_elementGen2) {
    x174_elementGen2->Update(dt);
  }

  bool shouldFree = false;
  if (x1dc_ == 1) {
    x1d4_ += dt;
    if (x1d4_ > 2.f) {
      x1d4_ = 2.f;
      x1a4_ += dt * x1b8_;
      if (x1a4_ > 1.f) {
        x1a4_ = 1.f;
        x1dc_ = 2;
        SetEmitParticles(true);
        if (x174_elementGen2) {
          x174_elementGen2->SetParticleEmission(true);
        }
      }
    }
  } else if (x1dc_ == 2) {
    float dVar5 = 0.f;
    if (x1e0_24_ || x1e0_25_) {
      x1c4_ -= dt;
      if (x1c4_ <= 0.f) {
        x1c4_ = 0.f;
        dVar5 = dt;
      }
      x1a0_ -= x1bc_ * (dt * x1cc_) + dVar5;
      x1a4_ = x1a0_ / x19c_;
      if (x1a4_ < 0.001f) {
        if (x1e0_24_) {
          shouldFree = true;
        } else {
          SetCallTouch(false);
          if (x1e0_25_) {
            x1dc_ = 0;
            SetActive(false);
          } else {
            x1dc_ = 3;
            x1d0_ = x1c8_;
          }
        }
        SetEmitParticles(false);
      }
      x1cc_ = 0.f;
    }
  } else if (x1dc_ == 3) {
    x1d0_ -= dt;
    if (x1d0_ <= 0.f) {
      x1d0_ = 0.f;
      UpdateParticleGens(mgr);
    }
  }
  if (shouldFree) {
    mgr.FreeScriptObject(GetUniqueId());
  }
}

void CPhazonPool::Touch(CActor& actor, CStateManager& mgr) {
  if (!GetActive() || x1dc_ != 2) {
    return;
  }

  CScriptTrigger::Touch(actor, mgr);
  if (actor.GetMaterialList().HasMaterial(EMaterialTypes::PlatformSlave)) {
    return;
  }

  for (auto& entry : x150_inhabitants) {
    if (entry.first == actor.GetUniqueId()) {
      entry.second = true;
      return;
    }
  }

  if (actor.GetTouchBounds().has_value()) {
    x150_inhabitants.emplace_back(actor.GetUniqueId(), true);
    mgr.SendScriptMsg(&actor, GetUniqueId(), EScriptObjectMessage::AddPhazonPoolInhabitant);
  }
}

void CPhazonPool::UpdateInhabitants(CStateManager& mgr) {
  auto iter = x150_inhabitants.begin();
  const auto end = x150_inhabitants.end();
  while (iter != end) {
    TCastToPtr<CActor> actor = mgr.ObjectById(iter->first);
    if (actor) {
      if (actor->GetTouchBounds().has_value()) {
        (void)GetTriggerBoundsWR(); // unused?
      }
    }
    if (!actor || !iter->second) {
      iter = x150_inhabitants.erase(iter);
      if (actor) {
        mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage::RemovePhazonPoolInhabitant);
      }
    } else {
      mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage::UpdatePhazonPoolInhabitant);
      iter->second = false;
      iter++;
    }
  }
}

void CPhazonPool::UpdateParticleGens(CStateManager& mgr) {
  x1b0_ = mgr.GetActiveRandom()->Range(-0.5f, 0.5f);
  x1b4_ = mgr.GetActiveRandom()->Range(-1.f, 1.f);
  x1b8_ = mgr.GetActiveRandom()->Range(0.25f, 2.f);
  x1cc_ = 0.f;
  x1d4_ = 0.f;
  x1c4_ = x1c0_;
  x1a0_ = x19c_;
  x1dc_ = 1;
  x1e0_25_ = false;
  if (x170_elementGen1) {
    x170_elementGen1->SetGlobalTranslation(GetTranslation());
    x170_elementGen1->SetParticleEmission(false);
  }
  if (x174_elementGen2) {
    x174_elementGen2->SetGlobalTranslation(GetTranslation());
    x174_elementGen2->SetParticleEmission(true);
  }
}

void CPhazonPool::RemoveInhabitants(CStateManager& mgr) {
  auto iter = x150_inhabitants.begin();
  const auto end = x150_inhabitants.end();
  while (iter != end) {
    if (TCastToPtr<CActor> actor = mgr.ObjectById(iter->first)) {
      iter = x150_inhabitants.erase(iter);
      mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage::RemovePhazonPoolInhabitant);
    } else {
      iter++;
    }
  }
}

void CPhazonPool::SetEmitParticles(bool val) {
  if (x170_elementGen1) {
    x170_elementGen1->SetParticleEmission(val);
  }
}
} // namespace urde::MP1
