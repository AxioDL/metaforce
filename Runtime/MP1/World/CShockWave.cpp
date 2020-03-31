#include "Runtime/MP1/World/CShockWave.hpp"

#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CGameLight.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CShockWave::CShockWave(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       TUniqueId parent, const SShockWaveData& data, float f1, float f2)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), {EMaterialTypes::Projectile},
         CActorParameters::None(), kInvalidUniqueId)
, xe8_id1(parent)
, xec_damageInfo(data.x8_damageInfo)
, x108_elementGenDesc(g_SimplePool->GetObj({SBIG('PART'), data.x4_particleDesc}))
, x110_elementGen(std::make_unique<CElementGen>(x108_elementGenDesc))
, x114_data(data)
, x150_(data.x24_)
, x154_(data.x2c_)
, x15c_(f1)
, x160_(f2) {
  if (data.x34_weaponDesc.IsValid()) {
    x974_electricDesc = g_SimplePool->GetObj({SBIG('ELSC'), data.x34_weaponDesc});
  }
  x110_elementGen->SetParticleEmission(true);
  x110_elementGen->SetOrientation(GetTransform().getRotation());
  x110_elementGen->SetGlobalTranslation(GetTranslation());
  xe6_27_thermalVisorFlags = 2;
}

void CShockWave::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CShockWave::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {
    if (x110_elementGen->SystemHasLight()) {
      x980_id2 = mgr.AllocateUniqueId();
      mgr.AddObject(new CGameLight(x980_id2, GetAreaIdAlways(), GetActive(), "ShockWaveLight_" + x10_name,
                                   GetTransform(), GetUniqueId(), x110_elementGen->GetLight(),
                                   x114_data.x4_particleDesc.Value(), 1, 0.f));
    }
  } else if (msg == EScriptObjectMessage::Deleted) {
    mgr.FreeScriptObject(x980_id2);
    x980_id2 = kInvalidUniqueId;
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
  mgr.SendScriptMsgAlways(x980_id2, uid, msg);
}

void CShockWave::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);
  g_Renderer->AddParticleGen(*x110_elementGen);
}

std::optional<zeus::CAABox> CShockWave::GetTouchBounds() const {
  if (x150_ <= 0.f) {
    return std::nullopt;
  }
  return zeus::CAABox({-x150_, -x150_, 0.f}, {x150_, x150_, 1.f}).getTransformedAABox(GetTransform());
}

void CShockWave::Render(const CStateManager& mgr) const {
  CActor::Render(mgr);
  x110_elementGen->Render();
}

void CShockWave::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    x110_elementGen->Update(dt);
    x158_ += dt;
    x150_ += x154_ * dt;
    x154_ += dt * x114_data.x30_;
    x110_elementGen->SetExternalVar(0, x150_);
    for (int i = 0; i < x110_elementGen->GetNumActiveChildParticles(); i++) {
      auto& particle = static_cast<CElementGen&>(x110_elementGen->GetActiveChildParticle(i));
      if (particle.Get4CharId() == SBIG('PART')) {
        particle.SetExternalVar(0, x150_);
      }
    }
    if (x16c_) {
      x164_ += dt;
      x16c_ = false;
    }
    if (x16d_) {
      x168_ += dt;
      x16d_ = false;
    }
  }
  if (x110_elementGen->IsSystemDeletable() && x15c_ > 0.f && x158_ >= x15c_) {
    mgr.FreeScriptObject(GetUniqueId());
  } else if (x980_id2 != kInvalidUniqueId) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(x980_id2)) {
      if (light->GetActive()) {
        light->SetLight(x110_elementGen->GetLight());
      }
    }
  }
}

void CShockWave::Touch(CActor& actor, CStateManager& mgr) {
  if (x158_ >= x15c_) {
    return;
  }

  bool isParent = xe8_id1 == actor.GetUniqueId();
  if (TCastToConstPtr<CCollisionActor> cactor = mgr.GetObjectById(actor.GetUniqueId())) {
    isParent = xe8_id1 == cactor->GetOwnerId();
  }
  if (!isParent) {
    // TODO
  }
}
} // namespace urde::MP1
